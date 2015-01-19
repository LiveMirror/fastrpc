// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include <iostream>
#include "closure.h"
#include "algorithm.h"
#include "string_number.h"
#include "http_client.h"
#include "http_server.h"
#include "netframe.h"
#include "atomic.h"
#include "base_thread.h"
#include "this_thread.h"
#include "thread_pool.h"
#include "gflags.h"
#include "logging.h"
#include "gtest.h"

using namespace ipcs_common;

DEFINE_int32(request_number, 10, "max request number");
DEFINE_int32(request_tos, 96, "default tos value");

Atomic<int32_t> g_request_count = 0;
Atomic<int32_t> g_bad_response_count = 0;
Atomic<bool> g_server_started = false;
ThreadPool g_thread_pool;
const char* static_resource = "this is static resource!";
const char* const html =
    "<html><header><title>demo</title></header><body><center>fuck</center></body></html>";
const std::string g_static_resource_path = "/test/prefix/test.html";
const std::string g_get_handler_path = "/test_get";
const std::string g_post_handler_path = "/test_post";
const std::string g_prefix_handler_path = "/prefix/";
const std::string g_prefix_closure_path = "/prefix_closure";
const std::string g_prefix_closure_chunked_path = "/prefix_chunked";
const std::string g_prefix_closure_with_connection_path = "/prefix_closure_with_connection";
const std::string g_bad_resource_path = "/test/prefix/test2.html";
const std::string g_bad_handler_path = "/def";

SocketAddressStorage g_server_address;

void ProcessPrefixRequest(const HttpRequest* http_request,
        HttpResponse* http_response,
        Closure<void>* done)
{
    http_response->mutable_http_body()->append("this is prefix!\n");
    done->Run();
}

void DoProcessChunkRequest(const HttpRequest* http_request,
        HttpResponse* http_response,
        Closure<void>* done)
{
    std::cout << http_request->HeadersToString();
    ThisThread::Sleep(2000);
    if (!http_request->http_body().empty()) {
        std::cout << http_request->http_body();
    }
    http_response->SetHeader("Transfer-Encoding", "chunked");
    http_response->mutable_http_body()->append("6\r\nhello,\r\n5\r\nworld\r\n0\r\n\r\n");
    done->Run();
}

void DoProcessRequest(const HttpRequest* http_request,
        HttpResponse* http_response,
        Closure<void>* done)
{
    std::cout << http_request->HeadersToString();
    ThisThread::Sleep(2000);
    if (!http_request->http_body().empty()) {
        std::cout << http_request->http_body();
    }
    http_response->mutable_http_body()->append(html);
    done->Run();
}

void DoProcessRequestWithConnection(HttpConnection* http_connection,
        const HttpRequest* http_request,
        HttpResponse* http_response,
        Closure<void>* done)
{
    VLOG(4) << "ProcessRequest with connection: " <<
        http_connection->GetRemoteAddress().ToString();
    DoProcessRequest(http_request, http_response, done);
}

void AsyncProcessRequest(const HttpRequest* http_request,
        HttpResponse* http_response,
        Closure<void>* done)
{
    Closure<void>* closure = NewClosure(DoProcessRequest,
            http_request, http_response, done);
    g_thread_pool.AddTask(closure);
}

class HttpServerThread : public BaseThread
{
public:
    void Entry()
    {
        netframe::NetFrame net_frame;
        // Two http servers share a netframe.
        HttpServer http_server_1(&net_frame, false);
        HttpServer http_server_2(&net_frame, false);

        // A handler can't be shared by different servers.
        HttpClosure* closure1 = NewPermanentClosure(DoProcessRequest);
        SimpleHttpServerHandler* handler1 = new SimpleHttpServerHandler(
                closure1);
        http_server_1.RegisterHandler(g_get_handler_path, handler1);
        http_server_1.RegisterHandler(g_post_handler_path, handler1);

        // Register Prefix Handler
        http_server_1.RegisterPrefixHandler(g_prefix_handler_path, handler1);

        // Register static resource
        http_server_1.RegisterStaticResource(g_static_resource_path, static_resource);
        HttpClosure* closure2 = NewPermanentClosure(AsyncProcessRequest);
        SimpleHttpServerHandler* handler2 = new SimpleHttpServerHandler(closure2);
        http_server_2.RegisterHandler(g_get_handler_path, handler2);
        http_server_2.RegisterHandler(g_post_handler_path, handler2);

        // Register Prefix Simple Handler
        HttpClosure* closure3 = NewPermanentClosure(DoProcessRequest);
        http_server_1.RegisterPrefixSimpleHandler(g_prefix_closure_path, closure3);

        // Register Prefix Simple Handler with HttpConnection
        HttpClosureWithConnection* closure4 = NewPermanentClosure(DoProcessRequestWithConnection);
        http_server_1.RegisterPrefixSimpleHandler(g_prefix_closure_with_connection_path, closure4);

        // Register Prefix Simple Handler process chunk request
        HttpClosure* closure5 = NewPermanentClosure(DoProcessChunkRequest);
        http_server_1.RegisterPrefixSimpleHandler(g_prefix_closure_chunked_path, closure5);

        // Start the two servers.
        bool server1_started = http_server_1.Start("127.0.0.1:0", &g_server_address);
        bool server2_started = http_server_2.Start(":60000");
        if (server1_started && server2_started) {
            g_server_started = true;
        }

        while (!IsStopRequested()) {
            ThisThread::Sleep(1000);
        }
        g_thread_pool.WaitForIdle();
        http_server_1.Stop();
        http_server_2.Stop();
    }
};

enum RequestState
{
    REQUEST_STATIC_RESOURCE,
    REQUEST_HANDLER,
    REQUEST_PREFIX_HANDLER,
    REQUEST_PREFIX_CLOSURE,
    REQUEST_PREFIX_CLOSURE_WITH_CONNECTION,
    REQUEST_PREFIX_CLOSURE_CHUNKED,
    REQUEST_POST,
};

enum RequestType
{
    REQUEST_NORMAL = 0,
    REQUEST_BAD_STATIC_RESOURCE,
    REQUEST_BAD_HANDLER,
    REQUEST_BAD_HEADER,
    REQUEST_BAD_URI,
    REQUEST_NOT_KEEPALIVE,
};

class TestHandler : public HttpHandler
{
public:
    explicit TestHandler(RequestType request_type) :
        m_request_type(request_type),
        m_request_state()
    {
    }

    void DoStaticResourceRequest(HttpConnection* http_connection, const std::string& uri_path)
    {
        HttpRequest http_request;
        http_request.set_method(HttpRequest::METHOD_GET);
        http_request.set_uri(uri_path);
        http_request.SetHeader("X-Forwarded-For", "127.0.0.1");

        std::string request = http_request.ToString();
        LOG(INFO) << "=========== send request ================";
        if (!http_connection->SendPacket(request.c_str(), request.size())) {
            LOG(INFO) << "Failed to send packet to server.";
        }
    }

    void DoPost(HttpConnection* http_connection)
    {
        HttpRequest http_request;
        http_request.set_method(HttpRequest::METHOD_POST);

        http_request.set_uri(g_post_handler_path);
        std::string body = "test post handler";
        http_request.set_body(body);
        http_request.SetHeader("Content-Length", IntegerToString(body.size()));
        std::string request = http_request.ToString();
        if (!http_connection->SendPacket(request.c_str(), request.size())) {
            LOG(INFO) << "Failed to send packet to server";
        }
    }

    void DoHandlerRequest(HttpConnection* http_connection, const std::string& handler_path)
    {
        HttpRequest http_request;
        http_request.set_method(HttpRequest::METHOD_GET);
        http_request.set_uri(handler_path);
        std::string request = http_request.ToString();
        if (!http_connection->SendPacket(request.c_str(), request.size())) {
            LOG(INFO) << "Failed to send packet to server";
        }
    }

    void DoBadHttpHeaderRequest(HttpConnection* http_connection)
    {
        std::string bad_header = "UNKNOWN /test HTTP/1.1\r\nHost: www.qq.com\r\n\r\n";
        netframe::Packet* packet = new netframe::Packet();
        packet->SetContent(bad_header.data(), bad_header.size());
        if (!http_connection->SendPacket(packet)) {
            LOG(INFO) << "Failed to send packet to server";
        }
    }

    void DoBadURIRequest(HttpConnection* http_connection)
    {
        std::string bad_uri_header = "GET :bad_uri HTTP/1.1\r\nHost: www.qq.com\r\n\r\n";
        if (!http_connection->SendPacket(bad_uri_header.data(), bad_uri_header.size())) {
            LOG(INFO) << "Failed to send packet to server";
        }
    }

    virtual void OnClose(HttpConnection* http_connection, int error_code)
    {
        if (m_request_type != REQUEST_NORMAL) {
            if (m_request_type == REQUEST_BAD_HEADER ||
                m_request_type == REQUEST_BAD_URI) {
                EXPECT_EQ(HttpResponse::Status_BadRequest,
                          http_connection->mutable_http_response()->status())
                    << m_request_type;
            } else if (m_request_type == REQUEST_BAD_STATIC_RESOURCE ||
                       m_request_type == REQUEST_BAD_HANDLER) {
                EXPECT_EQ(HttpResponse::Status_NotFound,
                          http_connection->mutable_http_response()->status())
                    << m_request_type;
            }
            ++g_bad_response_count;
        }
    }

    virtual void HandleHeaders(HttpConnection* http_connection)
    {
        LOG(INFO) << "=========== handler headers =============";
    }

    virtual void OnConnected(HttpConnection* http_connection)
    {
#ifndef _WIN32
        http_connection->SetTos(FLAGS_request_tos);
#endif
        if (m_request_type == REQUEST_NORMAL) {
            DoStaticResourceRequest(http_connection, g_static_resource_path);
            m_request_state = REQUEST_HANDLER;
        } else if (m_request_type == REQUEST_BAD_STATIC_RESOURCE) {
            DoStaticResourceRequest(http_connection, g_bad_resource_path);
        } else if (m_request_type == REQUEST_BAD_HANDLER) {
            DoHandlerRequest(http_connection, g_bad_handler_path);
        } else if (m_request_type == REQUEST_BAD_HEADER) {
            DoBadHttpHeaderRequest(http_connection);
        } else if (m_request_type == REQUEST_BAD_URI) {
            DoBadURIRequest(http_connection);
        }
    }

    virtual void HandleBodyPacket(HttpConnection* http_connection, const StringPiece& string_piece)
    {
        std::string value;
        if (http_connection->mutable_http_response()->GetHeader("Transfer-Encoding", &value)) {
            if (strcasecmp(value.data(), "chunked") == 0) {
                // chunked
                const char* begin = string_piece.data();
                const char* p = strstr(begin, "\r\n");
                if (p != NULL) {
                    int chunk_size = 0;
                    if (sscanf(begin, "%x", &chunk_size) != 1) {
                        return;
                    }
                    begin = p + 2;
                    if (chunk_size == 0) {
                        return;
                    }
                    http_connection->mutable_http_response()->mutable_http_body()->append(
                        begin, chunk_size);
                }
            }
        } else {
            http_connection->mutable_http_response()->mutable_http_body()->append(
                string_piece.data(), string_piece.length());
        }
    }

    virtual void HandleMessage(HttpConnection* http_connection)
    {
        LOG(INFO) << "=========== handler message =============";
        HttpResponse* http_response = http_connection->mutable_http_response();
        std::cout << std::endl;
        std::cout << http_response->HeadersToString();
        std::cout << http_response->http_body();
        std::cout << std::endl;
        if (m_request_type == REQUEST_NORMAL) {
#ifndef _WIN32
            StreamSocket socket;
            int socket_fd = http_connection->GetEndPoint().GetFd();
            socket.Attach(socket_fd);
            int tos;
            EXPECT_TRUE(socket.GetOption(SOL_IP, IP_TOS, &tos));
            EXPECT_EQ(FLAGS_request_tos, tos);
            socket.Detach();
#endif
            ++g_request_count;
            if (g_request_count >= FLAGS_request_number) {
                return;
            }
            switch (m_request_state) {
            case REQUEST_STATIC_RESOURCE:
                DoStaticResourceRequest(http_connection, g_static_resource_path);
                m_request_state = REQUEST_HANDLER;
                break;
            case REQUEST_HANDLER:
                DoHandlerRequest(http_connection, g_get_handler_path);
                m_request_state = REQUEST_PREFIX_HANDLER;
                break;
            case REQUEST_PREFIX_HANDLER:
                DoHandlerRequest(http_connection, g_prefix_handler_path + "/test");
                m_request_state = REQUEST_PREFIX_CLOSURE;
                break;
            case REQUEST_PREFIX_CLOSURE:
                DoHandlerRequest(http_connection, g_prefix_closure_path + "/test");
                m_request_state = REQUEST_PREFIX_CLOSURE_WITH_CONNECTION;
                break;
            case REQUEST_PREFIX_CLOSURE_WITH_CONNECTION:
                DoHandlerRequest(http_connection, g_prefix_closure_with_connection_path + "/test");
                m_request_state = REQUEST_PREFIX_CLOSURE_CHUNKED;
                // m_request_state = REQUEST_POST;
                break;
            case REQUEST_PREFIX_CLOSURE_CHUNKED:
                DoHandlerRequest(http_connection, g_prefix_closure_chunked_path + "/test");
                m_request_state = REQUEST_POST;
                break;
            case REQUEST_POST:
                DoPost(http_connection);
                m_request_state = REQUEST_STATIC_RESOURCE;
                break;
            default:
                break;
            }
        }
    }

    virtual int DetectBodyPacketSize(HttpConnection* http_connection, const StringPiece& data)
    {
        std::string value;
        HttpResponse* http_response = http_connection->mutable_http_response();
        // Header field "Content-Length" exists.
        if (http_response->GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(StringTrim(value), &content_length)) {
                LOG(INFO) << "content_length: " << content_length;
                return content_length;
            }
            return -1;
        }
        else if (http_response->GetHeader("Transfer-Encoding", &value)) {
            if (strcasecmp(value.c_str(), "chunked") == 0) {
                const char* begin = data.data();
                const char* p = strstr(begin, "\r\n");
                if (p != NULL) {
                    int chunk_size = 0;
                    if (sscanf(begin, "%x", &chunk_size) != 1) { // NOLINT(runtime/printf)
                        return -1;
                    }
                    begin = p + 2;
                    if (chunk_size == 0) {
                        return begin + 2 - data.data();
                    } else {
                        // "\r\n" is appended to the end of chunk
                        return chunk_size + 2 + (begin - data.data());
                    }
                } else {
                    // not receive a complete chunk
                    return 0;
                }
            }
        }
        return 0;
    }

private:
    RequestType m_request_type;
    RequestState m_request_state;
};

TEST(Http, ServerClientConnection)
{
    HttpServerThread server;
    g_server_started = false;
    int retry_count = 0;
    server.Start();
    // Wait until server starts
    while (!g_server_started) {
        if (retry_count == 3) {
            break;
        }
        ++retry_count;
        ThisThread::Sleep(1000);
    }

    if (!g_server_started) {
        EXPECT_TRUE(false) << "Server not started";
        server.StopAndWaitForExit();
        return;
    }

    HttpClient http_client;
    g_request_count = 0;
    if (!http_client.ConnectServer(g_server_address.ToString(), new TestHandler(REQUEST_NORMAL))) {
        EXPECT_TRUE(false) << "Failed to connect to server";
        server.StopAndWaitForExit();
        return;
    }
    while (g_request_count < FLAGS_request_number) {
        ThisThread::Sleep(1000);
    }

    server.StopAndWaitForExit();
}

TEST(Http, BadRequest)
{
    HttpServerThread server;
    g_server_started = false;
    int retry_count = 0;
    server.Start();
    while (!g_server_started) {
        if (retry_count == 3) {
            break;
        }
        ++retry_count;
        ThisThread::Sleep(1000);
    }

    if (!g_server_started) {
        EXPECT_TRUE(false) << "Server not started.";
        server.StopAndWaitForExit();
        return;
    }

    HttpClient http_client;
    g_bad_response_count = 0;
    for (int i = static_cast<int>(REQUEST_BAD_STATIC_RESOURCE);
         i <= static_cast<int>(REQUEST_BAD_URI);
         ++i) {
        if (!http_client.ConnectServer(g_server_address.ToString(),
                                       new TestHandler(static_cast<RequestType>(i)))) {
            EXPECT_TRUE(false) << "Failed to connect to server";
            server.StopAndWaitForExit();
            return;
        }
    }
    while (g_bad_response_count < 4) {
        ThisThread::Sleep(1000);
    }
    server.StopAndWaitForExit();
}
