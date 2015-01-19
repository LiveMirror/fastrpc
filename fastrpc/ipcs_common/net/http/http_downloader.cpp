// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include <algorithm>
#include <utility>

#include "scoped_ptr.h"
#include "algorithm.h"
#include "string_number.h"
#include "http_downloader.h"
#include "http_message.h"
#include "mime.h"
#include "uri.h"
#include "domain_resolver.h"
#include "logging.h"

namespace ipcs_common {

namespace {

static const char kHttpScheme[] = "http";
static const char kDefaultPath[] = "/";
static const char kDefaultHttpPort[] = "80";
static size_t kDefaultMaxResponseLength = 1024 * 1024 * 2;

typedef HttpDownloader::Options Options;
typedef HttpDownloader::ErrorType ErrorType;

// according to RFC2616, HTTP STATUS 1xx, 204, and 304 doesn't have a HTTP
// body.
bool ResponseStatusHasContent(int http_status)
{
    if (http_status < 200) {
        return false;
    } else if (http_status == 204) {
        return false;
    } else if (http_status == 304) {
        return false;
    }

    return true;
}

void AppendHeaderToRequest(const std::string& path,
                           const HttpHeaders& headers,
                           HttpRequest* request)
{
    size_t count = headers.Count();
    for (size_t k = 0; k < count; ++k) {
        std::pair<std::string, std::string> header;
        headers.GetAt(k, &header);
        request->AddHeader(header.first, header.second);
    }
}

class DownloadTask {
public:
    explicit DownloadTask(HttpDownloader *http_downloader)
        : m_connector(AF_INET, IPPROTO_TCP),
          m_err_code(HttpDownloader::SUCCESS),
          m_max_response_length(0)
    {
        m_connector.SetLinger(true, 1);
        m_http_downloader = http_downloader;
    }

    ~DownloadTask()
    {
    }

    bool ProcessRequest(const std::string& url,
                        const Options& options,
                        HttpRequest* request,
                        HttpResponse *response)
    {
        net::URI *uri = NULL;
        if (!m_uri.Parse(url)) {
            m_err_code = HttpDownloader::ERROR_INVALID_URI_ADDRESS;
            return false;
        }
        uri = &m_uri;

        // Apply HTTP HEADERS into request
        std::string path = uri->Path();
        AppendHeaderToRequest(path, options.Headers(), request);

        m_max_response_length = options.MaxResponseLength() ?
            options.MaxResponseLength() :
            kDefaultMaxResponseLength;

        std::string path_and_query = uri->PathAndQuery();
        std::string host = uri->Host();

        request->SetHeader("User-Agent", m_http_downloader->UserAgent());
        request->SetHeader("Host", host);

        if (!m_http_downloader->Proxy().empty()) {
            if (!m_proxy_uri.Parse(m_http_downloader->Proxy())) {
                m_err_code = HttpDownloader::ERROR_INVALID_PROXY_ADDRESS;
                return false;
            }
            uri = &m_proxy_uri;
            path_and_query = url;
        }

        request->set_uri(path_and_query.empty() ? kDefaultPath : path_and_query);

        std::string port_str;
        if (uri->HasPort()) {
            port_str = uri->Port();
        } else if (uri->Scheme().empty() || uri->Scheme() == kHttpScheme) {
            port_str = kDefaultHttpPort;
        } else {
            m_err_code = HttpDownloader::ERROR_PROTOCAL_NOT_SUPPORTED;
            return false;
        }

        uint32_t port;
        if (!StringToNumber(port_str, &port))
            return false;

        std::vector<SocketAddressInet4> sa;
        if (!HttpDownloader::ResolveAddress(host, port, &sa, &m_err_code)) {
            return false;
        }

        for (std::vector<SocketAddressInet4>::const_iterator it = sa.begin();
            it != sa.end();
            ++it) {
            if (ProcessRequest(*it, *request, response)) {
                return true;
            }
        }

        return false;
    }

    ErrorType GetLastError() const
    {
        return m_err_code;
    }

private:
    bool ProcessRequest(const SocketAddressInet4& addr,
                        const HttpRequest& request,
                        HttpResponse* response)
    {
        if (!m_connector.Connect(addr)) {
            m_err_code = HttpDownloader::ERROR_FAIL_TO_CONNECT_SERVER;
            return false;
        }

        return SendRequest(request) && RecvResponse(response);
    }

    bool SendRequest(const HttpRequest& request)
    {
        std::string headers = request.HeadersToString();
        headers.append(request.http_body());
        VLOG(5) << headers << std::endl;

        if (!m_connector.SendAll(headers.c_str(), headers.length())) {
            m_err_code = HttpDownloader::ERROR_FAIL_TO_SEND_REQUEST;
            return false;
        }
        return true;
    }

    bool RecvResponse(HttpResponse* response)
    {
        scoped_array<char> buffer(new char[m_max_response_length]);
        char *buff = buffer.get();

        size_t total_received = 0;
        size_t received_length;
        size_t buffer_length = m_max_response_length - 1;
        char *p = NULL;
        // handle headers first.
        do {
            if (!m_connector.Receive(buff + total_received,
                                     buffer_length,
                                     &received_length)) {
                m_err_code = HttpDownloader::ERROR_FAIL_TO_GET_RESPONSE;
                return false;
            } else if (received_length == 0) {
                m_err_code = HttpDownloader::ERROR_FAIL_TO_GET_RESPONSE;
                VLOG(4) << "The peer reset the network connection.";
                return false;
            }
            total_received += received_length;
            buffer_length -= received_length;

            buff[total_received] = 0;

            p = strstr(buff, "\r\n\r\n");
        } while (p == NULL && buffer_length > 0);

        if (p == NULL) {
            m_err_code = HttpDownloader::ERROR_INVALID_RESPONSE_HEADER;
            return false;
        }
        p += 4;

        StringPiece piece(buff, p - buff);
        HttpMessage::ErrorType message_error;
        if (!m_response.ParseHeaders(piece, &message_error)) {
            m_err_code = HttpDownloader::ERROR_INVALID_RESPONSE_HEADER;
            return false;
        }

        char* current = buff + total_received;
        char* end = current + buffer_length; // end of buffer for body
        if (!ResponseStatusHasContent(m_response.status())) {
            // no content
        } else if (m_response.HasHeader("Transfer-Encoding")
               && m_response.GetHeader("Transfer-Encoding") != "identity") {
            // chunked content
            RecvBodyWithChunks(p, end, current);
        } else if (m_response.HasHeader("Content-Length")) {
            // Content-Length field is given
            RecvBodyWithContentLength(p, end, current);
        } else if (m_response.HasHeader("Content-Type") &&
                   m_response.GetHeader("Content-Type") == "multipart/byteranges") {
            // not supported yet
            m_err_code = HttpDownloader::ERROR_CONTENT_TYPE_NOT_SUPPORTED;
            return false;
        } else {
            // for the case the HTTP server close the connection
            RecvBodyWithConnectionReset(p, end, current);
        }

        std::swap(m_response, *response);

        if (response->status() != HttpResponse::Status_OK)
            m_err_code = HttpDownloader::ERROR_HTTP_STATUS_CODE;

        return true;
    }

    // For response with a HEADER Content-Length
    void RecvBodyWithContentLength(char *begin, char *end, char* current)
    {
        std::string body;
        body.reserve(end - begin + 1);
        body.append(begin, current);

        size_t content_length = m_response.GetContentLength();
        // if received buffer is not enough
        if (content_length > body.length()) {
            size_t buf_len = end - current;
            size_t download = std::min(buf_len, content_length - body.length());

            size_t received = 0;
            if (!m_connector.ReceiveAll(current,
                                        download,
                                        &received)) {
                m_err_code = HttpDownloader::ERROR_FAIL_TO_GET_RESPONSE;
            }
            body.append(current, received);
        }
        m_response.mutable_http_body()->swap(body);
    }

    // For content data in chunks
    void RecvBodyWithChunks(char *begin, char *end, char* current)
    {
        std::string body;
        body.reserve(m_max_response_length);

        int buffer_length = end - current;
        while (begin < end) {
            size_t received = 0;

            *current = 0;
            char *p = strstr(begin, "\r\n");
            if (p != NULL) {
                int chunk_size = 0;
                if (sscanf(begin, "%x", &chunk_size) != 1) { // NOLINT(runtime/printf)
                    m_err_code = HttpDownloader::ERROR_FAIL_TO_READ_CHUNKSIZE;
                    return;
                }
                begin = p + 2;
                if (chunk_size == 0) {
                    // finish
                    m_response.mutable_http_body()->swap(body);
                    return;
                }

                chunk_size += 2; // "\r\n" is appended to the end of chunk
                int downloaded = current - begin;
                chunk_size = std::min(chunk_size, buffer_length);
                // if the downloaded content is not enough, download more.
                if (downloaded < chunk_size) {
                    size_t length = chunk_size - downloaded;
                    if (!m_connector.ReceiveAll(current, length, &received)) {
                        m_err_code = HttpDownloader::ERROR_FAIL_TO_GET_RESPONSE;
                        return;
                    }
                    current += received;
                    buffer_length -= received;
                }
                // Exclude the trailing "\r\n"
                body.append(begin, chunk_size - 2);
                begin += chunk_size;
            } else {
                // there is not enough content to get a whole CHUNK header
                // download more data.
                if (!m_connector.Receive(current, buffer_length, &received)) {
                    m_err_code = HttpDownloader::ERROR_FAIL_TO_GET_RESPONSE;
                    return;
                }
                if (received == 0) {
                    // Unexpected peer closed.
                    m_err_code = HttpDownloader::ERROR_FAIL_TO_GET_RESPONSE;
                    return;
                }
                current += received;
                buffer_length -= received;
            }
        }
    }

    // Old HTTP servers will close connection after send response package
    void RecvBodyWithConnectionReset(char *begin, char *end, char* current)
    {
        size_t received;
        if (m_connector.ReceiveAll(current, end - current, &received) ||
           Socket::GetLastError() == ECONNRESET) {
            current += received;
            std::string body(begin, current - begin);
            m_response.mutable_http_body()->swap(body);
        }
    }

private:
    HttpDownloader *m_http_downloader;
    net::URI m_uri;
    net::URI m_proxy_uri;
    StreamSocket m_connector;
    HttpResponse m_response;
    ErrorType m_err_code;
    size_t m_max_response_length;
};

} // anonymous namespace

const HttpDownloader::ErrorMessage HttpDownloader::kErrorMessage[] =
{
    { SUCCESS,                          "Success"                           },
    { ERROR_INVALID_URI_ADDRESS,        "Invalid URI address"               },
    { ERROR_INVALID_PROXY_ADDRESS,      "Invalid proxy address"             },
    { ERROR_INVALID_RESPONSE_HEADER,    "Invalid response header"           },
    { ERROR_FAIL_TO_RESOLVE_ADDRESS,    "Failed to resolve address"         },
    { ERROR_FAIL_TO_SEND_REQUEST,       "Failed to send request"            },
    { ERROR_FAIL_TO_GET_RESPONSE,       "Failed to get response"            },
    { ERROR_FAIL_TO_CONNECT_SERVER,     "Failed to connect to server"       },
    { ERROR_FAIL_TO_READ_CHUNKSIZE,     "Failed to read chunk size"         },
    { ERROR_PROTOCAL_NOT_SUPPORTED,     "Protocal is not supported"         },
    { ERROR_CONTENT_TYPE_NOT_SUPPORTED, "Content type is not supported yet" },
};

HttpDownloader::Options&
HttpDownloader::Options::SetEncoding(const std::string& encoding)
{
    m_encoding = encoding;
    return *this;
}

const std::string&
HttpDownloader::Options::Encoding() const
{
    return m_encoding;
}

HttpHeaders&
HttpDownloader::Options::Headers()
{
    return m_headers;
}

const HttpHeaders&
HttpDownloader::Options::Headers() const
{
    return m_headers;
}

HttpDownloader::Options&
HttpDownloader::Options::SetMaxResponseLength(size_t length)
{
    m_max_response_length = length;
    return *this;
}

size_t HttpDownloader::Options::MaxResponseLength() const
{
    return m_max_response_length;
}

HttpDownloader::HttpDownloader()
{
    m_user_agent = "SosoDownloader/1.0(compatible; MSIE 7.0; Windows NT 5.1)";
}

HttpDownloader::~HttpDownloader()
{
}

bool HttpDownloader::ResolveAddress(const std::string& host,
                                    uint16_t port,
                                    std::vector<SocketAddressInet4> *sa,
                                    ErrorType *error)
{
    std::vector<IPAddress> ipaddr;
    int error_code;
    if (!DomainResolver::ResolveIpAddress(
            host,
            &ipaddr,
            &error_code)) {
        *error = HttpDownloader::ERROR_FAIL_TO_RESOLVE_ADDRESS;
        return false;
    }

    std::vector<SocketAddressInet4> sock_addr;
    for (std::vector<IPAddress>::const_iterator it = ipaddr.begin();
        it != ipaddr.end();
        ++it) {
        sock_addr.push_back(SocketAddressInet4(*it, port));
    }
    sa->swap(sock_addr);

    return true;
}

const char* HttpDownloader::GetErrorMessage(ErrorType err_code)
{
    size_t size = sizeof(kErrorMessage) / sizeof(kErrorMessage[0]);
    for (size_t k = 0; k < size; ++k) {
        if (err_code == kErrorMessage[k].err_code) {
            return kErrorMessage[k].err_msg;
        }
    }

    return NULL;
}

HttpDownloader& HttpDownloader::SetProxy(const std::string& proxy)
{
    m_proxy = proxy;
    return *this;
}

const std::string& HttpDownloader::Proxy() const
{
    return m_proxy;
}

HttpDownloader& HttpDownloader::SetUserAgent(const std::string& user_agent)
{
    m_user_agent = user_agent;
    return *this;
}

const std::string& HttpDownloader::UserAgent() const
{
    return m_user_agent;
}

bool HttpDownloader::Request(HttpRequest::MethodType method,
                             const std::string& url,
                             const std::string& data,
                             const Options& options,
                             HttpResponse *response,
                             ErrorType *error)
{
    ErrorType error_placeholder;
    if (error == NULL) {
        error = &error_placeholder;
    }

    HttpRequest request;
    request.set_method(method);
    request.set_body(data);
    request.SetHeader("Content-Length", IntegerToString(data.size()));

    DownloadTask task(this);
    bool ret = task.ProcessRequest(url, options, &request, response);

    *error = task.GetLastError();
    return ret && *error == SUCCESS;
}

bool HttpDownloader::Get(const std::string& url,
                         const Options& options,
                         HttpResponse *response,
                         ErrorType *error)
{
    return Request(HttpRequest::METHOD_GET, url, "", options, response, error);
}

bool HttpDownloader::Get(const std::string& url,
                         HttpResponse* response,
                         ErrorType *error)
{
    return Get(url, Options(), response, error);
}

bool HttpDownloader::Post(const std::string& url,
                          const std::string& data,
                          const Options& options,
                          HttpResponse *response,
                          ErrorType *error)
{
    return Request(HttpRequest::METHOD_POST, url, data, options, response, error);
}

bool HttpDownloader::Post(const std::string& url,
                          const std::string& data,
                          HttpResponse* response,
                          ErrorType *error)
{
    return Post(url, data, Options(), response, error);
}

bool HttpDownloader::Put(const std::string& url,
                         const std::string& data,
                         const Options& options,
                         HttpResponse* response,
                         ErrorType* error)
{
    return Request(HttpRequest::METHOD_PUT, url, data, options, response, error);
}

bool HttpDownloader::Put(const std::string& url,
                         const std::string& data,
                         HttpResponse* response,
                         ErrorType* error)
{
    return Put(url, data, Options(), response, error);
}

bool HttpDownloader::Delete(const std::string& url,
                            const Options& options,
                            HttpResponse* response,
                            ErrorType* error)
{
    return Request(HttpRequest::METHOD_DELETE, url, "", options, response, error);
}

bool HttpDownloader::Delete(const std::string& url,
                            HttpResponse* response,
                            ErrorType* error)
{
    return Delete(url, Options(), response, error);
}

} // namespace common

