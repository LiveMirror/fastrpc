// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_server.h"
#include <signal.h>
#include "closure.h"
#include "algorithm.h"
#include "concat.h"
#include "string_number.h"
#include "http_connection.h"
#include "http_handler.h"
#include "http_time.h"
#include "mime.h"
#include "uri.h"
#include "command_event.h"
#include "socket_handler.h"
#include "this_thread.h"
#include "logging.h"
#include "raw_logging.h"

using namespace ipcs_common;

namespace {

class StaticResourceHttpHandler : public HttpHandler {
public:
    StaticResourceHttpHandler(const std::string& path,
                              const StringPiece& content,
                              const HttpHeaders& headers);
    virtual void HandleHeaders(HttpConnection* http_connection) {}
    virtual void HandleBodyPacket(HttpConnection* http_connection,
                                  const StringPiece& string_piece) {}
    void HandleMessage(HttpConnection* http_connection);

    virtual int DetectBodyPacketSize(HttpConnection* http_connection,
                                     const StringPiece& data) {
        return 0;
    }

private:
    bool SetStaticResource(const std::string& path,
                           const StringPiece& content,
                           const HttpHeaders& headers);

    // Store static resource path
    std::string m_path;
    // Store the content of static resource
    StringPiece m_content;
    // Http headers added by user
    HttpHeaders m_http_headers;
};

StaticResourceHttpHandler::StaticResourceHttpHandler(const std::string& path,
                                                     const StringPiece& content,
                                                     const HttpHeaders& headers)
{
    if (!SetStaticResource(path, content, headers)) {
        throw std::runtime_error("Register source failed!");
    }
}

bool StaticResourceHttpHandler::SetStaticResource(const std::string& path,
                                                  const StringPiece& content,
                                                  const HttpHeaders& headers)
{
    // Override the static resource if set the same path
    m_path = path;
    m_content = content;
    m_http_headers = headers;

    // Check if set Content-Type
    if (!m_http_headers.Has("Content-Type")) {
        std::string::size_type pos = path.rfind(".");
        net::MimeType mt = net::MimeType::FromFileExtension(path.substr(pos + 1, path.size()));
        if (mt.Empty()) {
            LOG(ERROR) << "error find content type!";
            return false;
        }
        m_http_headers.Add("Content-Type", mt.ToString());
    }

    // Check if set Cache-Control
    if (!m_http_headers.Has("Cache-Control")) {
        m_http_headers.Add("Cache-Control", "max-age=60");
    }

    // Check if set Expires
    if (!m_http_headers.Has("Expires")) {
        std::string expires_gmt_string;
        if (FormatHttpTime(time(NULL)+60, &expires_gmt_string)) {
            m_http_headers.Add("Expires", expires_gmt_string);
        }
    }

    return true;
}

void StaticResourceHttpHandler::HandleMessage(HttpConnection* http_connection)
{
    HttpRequest* http_request = http_connection->mutable_http_request();
    HttpResponse http_response;

    http_response.set_status(HttpResponse::Status_OK);
    http_response.AddHeaders(m_http_headers);
    http_response.AddHeader("Content-Length", NumberToString(m_content.size()));
    http_response.mutable_http_body()->append(
        m_content.data(), m_content.size());

    // Send the package
    std::string response = http_response.HeadersToString();
    response.append(http_response.http_body());
    http_connection->SendPacket(response.data(), response.size());

    // Client request that don't keep alive.
    if (!http_request->IsKeepAlive()) {
        VLOG(3) << "Not keep alive connection, close: "
            << http_connection->GetConnectionId();
        http_connection->Close(false);
    } else if (!http_response.IsKeepAlive()) {
        VLOG(3) << "Response set connection closed: "
            << http_connection->GetConnectionId();
        http_connection->Close(false);
    }
}

} // namespace

class HttpServerListener : public netframe::ListenSocketHandler {
public:
    explicit HttpServerListener(HttpServer* http_server)
        : netframe::ListenSocketHandler(*(http_server->mutable_net_frame())),
          m_http_server(http_server) {
    }
    virtual ~HttpServerListener() {}

private:
    // Implements ListenSocketHandler interface.
    virtual void OnClose(int error_code) {
        m_http_server->mutable_listener_manager()->RemoveConnection(
                GetEndPoint().GetId());
    }

    virtual netframe::StreamSocketHandler* OnAccepted(netframe::SocketId id) {
        // netframe would take the ownership of the allocated http connection.
        HttpServerConnection* connection =
            new HttpServerConnection(m_http_server);
        m_http_server->mutable_connection_manager()->AddConnection(id.GetId());
        return connection;
    }

    HttpServer* m_http_server;
};

void HttpServer::RegisterHandler(const std::string& path, HttpHandler* handler) {
    handler->set_http_server(this);
    m_handler_manager.RegisterHandler(path, handler);
}

void HttpServer::RegisterSimpleHandler(const std::string& path,
                                       HttpClosure* closure) {
    SimpleHttpServerHandler* handler = new SimpleHttpServerHandler(closure);
    RegisterHandler(path, handler);
}

void HttpServer::RegisterSimpleHandler(const std::string& path,
                                       HttpClosureWithConnection* closure) {
    SimpleHttpServerHandler* handler = new SimpleHttpServerHandler(closure);
    RegisterHandler(path, handler);
}

void HttpServer::RegisterPrefixHandler(const std::string& path,
                                       HttpHandler* handler) {
    handler->set_http_server(this);
    return m_handler_manager.RegisterPrefixHandler(path, handler);
}

void HttpServer::RegisterPrefixSimpleHandler(const std::string& path,
                                             HttpClosure* closure) {
    SimpleHttpServerHandler* handler = new SimpleHttpServerHandler(closure);
    return RegisterPrefixHandler(path, handler);
}

void HttpServer::RegisterPrefixSimpleHandler(const std::string& path,
                                             HttpClosureWithConnection* closure) {
    SimpleHttpServerHandler* handler = new SimpleHttpServerHandler(closure);
    return RegisterPrefixHandler(path, handler);
}

void HttpServer::RegisterStaticResource(const std::string& path,
                                        const StringPiece& content,
                                        const HttpHeaders& headers) {
    StaticResourceHttpHandler* handler =
        new StaticResourceHttpHandler(path, content, headers);
    RegisterHandler(path, handler);
}

HttpHandler* HttpServer::FindHandler(const std::string& path) {
    return m_handler_manager.FindHandler(path);
}

bool HttpServer::Start(const std::string& server_address,
                       const netframe::NetFrame::EndPointOptions& options,
                       SocketAddress* real_bind_address) {
    std::string real_server_address = server_address;
    if (HttpServer::IsPrivatePortAddress(server_address)) {
        IPAddress private_address;
        if (IPAddress::GetFirstPrivateAddress(&private_address)) {
            real_server_address = private_address.ToString() + server_address;
            LOG(INFO) << "get private address: " << real_server_address << " for: "
                << server_address;
        } else {
            LOG(ERROR) << "Can't get private address for: " << server_address;
            return false;
        }
    }
    SocketAddressInet server_address_inet(real_server_address);
    HttpServerListener* listener =
        new HttpServerListener(this);
    int64_t listen_socket_id = mutable_net_frame()->AsyncListen(
            server_address_inet, listener, kMaxMessageSize, options);
    if (listen_socket_id < 0) {
        LOG(ERROR) << "Can't listen on address: " << server_address;
        delete listener;
        return false;
    }

    if (server_address_inet.GetPort() == 0) {
        ListenerSocket socket;
        socket.Attach(netframe::SocketId(listen_socket_id).GetFd());
        CHECK(socket.GetLocalAddress(&server_address_inet));
        socket.Detach();
    }
    LOG(INFO) << "Listen on " << server_address_inet.ToString();

    if (real_bind_address) {
        *real_bind_address = server_address_inet;
    }

    m_listener_manager.AddConnection(listen_socket_id);
    return true;
}

bool HttpServer::Start(const std::string& server_address,
                       SocketAddress* real_bind_address) {
    netframe::NetFrame::EndPointOptions options =
        netframe::NetFrame::EndPointOptions();
    return Start(server_address, options, real_bind_address);
}

void HttpServer::Stop() {
    // listen should be closed firstly
    m_listener_manager.CloseAllConnections();
    HttpBase::Stop();
}

static volatile bool s_quit = false;

static void SignalIntHandler(int sig_no)
{
    RAW_LOG(INFO, "Signal received: %s", strsignal(sig_no));
    s_quit = true;
}

int HttpServer::Run()
{
    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);

    while (!s_quit) {
        ThisThread::Sleep(1000);
    }
    return 0;
}

HttpServer::~HttpServer() {
    Stop();
}

// static
bool HttpServer::IsPrivatePortAddress(const std::string& server_address)
{
    if (server_address[0] == ':') {
        std::string port_str = server_address.substr(1);
        int port_value = 0;
        if (StringToNumber(port_str, &port_value)) {
            if (port_value > 0 && port_value <= 65535) {
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// HttpServerConnection staff

HttpServerConnection::HttpServerConnection(HttpServer* http_server)
    : HttpConnection(http_server->mutable_net_frame()),
      m_http_server(http_server) {
}

std::string HttpServerConnection::GetClientDescription() const
{
    std::string client_address = GetRemoteAddress().ToString();
    if (!m_proxy_addresses.empty()) {
        client_address.append("(");
        for (size_t i = 0; i < m_proxy_addresses.size(); i++) {
            client_address.append(", ");
        }
        client_address.append(")");
    }
    return client_address;
}

void HttpServerConnection::OnConnected()
{
    m_stats_entry.SetDescription(GetClientDescription());
    m_http_server->mutable_traffic_stats()->RegisterStatsEntry(&m_stats_entry);
}

void HttpServerConnection::OnClose(int error_code) {
    if (m_http_handler) {
        m_http_handler->OnClose(this, error_code);
    }
    m_http_server->mutable_traffic_stats()->UnregisterStatsEntry(&m_stats_entry);
    m_http_server->mutable_connection_manager()->RemoveConnection(
            GetConnectionId());

    // Reset the connection status.
    m_header_received = false;
}

bool HttpServerConnection::OnSent(netframe::Packet* packet) {
    m_stats_entry.AddTxBytes(packet->Length());
    return true;
}

void HttpServerConnection::OnReceived(const netframe::Packet& packet) {
    m_stats_entry.AddRxBytes(packet.Length());
    bool message_completed = false;
    if (!m_header_received) {
        StringPiece header(reinterpret_cast<const char*>(packet.Content()),
                           packet.Length());
        m_http_request.Reset();
        HttpMessage::ErrorType error = HttpMessage::SUCCESS;
        if (!m_http_request.ParseHeaders(header, &error)) {
            // (TODO) handler parser error.
            LOG(WARNING) << "Failed to parse the http header: " << header << "\n"
                         << "Error: " << error << ", from "
                         << packet.GetRemoteAddress().ToString();
            static const char response[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
            SendPacket(response, sizeof(response) - 1);
            Close();
            return;
        }

        m_proxy_addresses.clear();
        std::string x_forwarded_for;
        if (m_http_request.GetHeader("X-Forwarded-For", &x_forwarded_for)) {
            SplitStringByAnyOf(x_forwarded_for, ", ", &m_proxy_addresses);
            if (!m_proxy_addresses.empty()) {
                m_proxy_addresses.push_back(GetRemoteAddress().ToString());
                SocketAddressInet ip_address;
                if (ip_address.Parse(m_proxy_addresses[0] + ":0")) {
                    SetRemoteAddress(ip_address);
                }

                m_proxy_addresses.erase(m_proxy_addresses.begin());
            }
        }

        net::URI uri;
        if (!uri.Parse(m_http_request.uri())) {
            LOG(WARNING) << "Invalid request uri: " << m_http_request.uri()
                         << " from " << packet.GetRemoteAddress().ToString();
            std::string bad_req_page = "HTTP/1.1 400 Bad Request\r\n";
            const char bad_page_body[] ="<html><head><title>400 Bad Request</title></head>"
                    "<body><font size=10><b>Invalid Request URI!</b></font></body></html>";
            StringAppend(&bad_req_page, "Content-Length: ");
            StringAppend(&bad_req_page, sizeof(bad_page_body) - 1);
            StringAppend(&bad_req_page, "\r\n\r\n");
            StringAppend(&bad_req_page, bad_page_body);
            SendPacket(bad_req_page.c_str(), bad_req_page.size());
            Close();
            return;
        }

        m_http_handler = m_http_server->FindHandler(uri.Path());
        if (m_http_handler == NULL) {
            // (TODO) no handler registered.
            LOG(WARNING) << "No handler registered on the path: " << uri.Path()
                         << " from " << packet.GetRemoteAddress().ToString();
            std::string not_found = "HTTP/1.1 404 Not Found\r\n";
            static const char response_body[] = "<html><head><title>404 not found</title></head>"
                    "<body><font size=10><b>Page Not Found!</b></font></body></html>";
            StringAppend(&not_found, "Content-Length: ");
            StringAppend(&not_found, sizeof(response_body) - 1);
            StringAppend(&not_found, "\r\n\r\n");
            StringAppend(&not_found, response_body);
            SendPacket(not_found.c_str(), not_found.size());
            Close();
            return;
        }
        m_http_handler->HandleHeaders(this);

        // Check if the message is complete
        std::string value;
        if (m_http_request.method() == HttpRequest::METHOD_GET ||
            m_http_request.method() == HttpRequest::METHOD_HEAD) {
            message_completed = true;
        } else if (m_http_request.GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(value, &content_length) && content_length == 0) {
                message_completed = true;
            }
        }

        // If message_completed, this request has been received totally
        // m_header_received should be set false again for the next request
        m_header_received = !message_completed;

        if (message_completed) {
            m_http_handler->HandleMessage(this);
        }
    } else {
        StringPiece data(reinterpret_cast<const char*>(packet.Content()),
                         packet.Length());
        m_http_handler->HandleBodyPacket(this, data);

        // Check if the message is complete
        std::string value;
        if (m_http_request.GetHeader("Transfer-Encoding", &value)) {
            // (TODO) handle transfer encoding.
            if (strcasecmp(value.c_str(), "chunked") == 0) {
                if (packet.Length() == 5) { // 0\r\n\r\n
                    message_completed = true;
                }
            }
        } else if (m_http_request.GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(value, &content_length) &&
                    content_length == static_cast<int>(packet.Length())) {
                message_completed = true;
            }
        }

        if (message_completed) {
            m_header_received = false;
            m_http_handler->HandleMessage(this);
        }
    }
}
