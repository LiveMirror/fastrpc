// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include <string>
#include "closure.h"
#include "algorithm.h"
#include "string_number.h"
#include "http_handler.h"
#include "http_server.h"
#include "uri.h"
#include "logging.h"

SimpleHttpServerHandler::SimpleHttpServerHandler(HttpClosure* closure, bool own_closure) :
    m_closure(closure),
    m_closure_with_connection(NULL),
    m_own_closure(own_closure)
{
    CHECK_NOTNULL(closure);
    CHECK(!closure->IsSelfDelete());
}

SimpleHttpServerHandler::SimpleHttpServerHandler(
    HttpClosureWithConnection* closure, bool own_closure)
:
    m_closure(NULL),
    m_closure_with_connection(closure),
    m_own_closure(own_closure)
{
    CHECK_NOTNULL(closure);
    CHECK(!closure->IsSelfDelete());
}

SimpleHttpServerHandler::~SimpleHttpServerHandler()
{
    if (m_own_closure) {
        delete m_closure;
        delete m_closure_with_connection;
    }
    m_closure = NULL;
    m_closure_with_connection = NULL;
}

void SimpleHttpServerHandler::HandleBodyPacket(HttpConnection* http_connection,
                                               const StringPiece& string_piece)
{
    http_connection->mutable_http_request()->mutable_http_body()->append(
            string_piece.data(), string_piece.length());
}

int SimpleHttpServerHandler::DetectBodyPacketSize(
    HttpConnection* http_connection, const StringPiece &data)
{
    std::string value;
    HttpRequest* http_request = http_connection->mutable_http_request();
    // Header field "Content-Length" exists.
    if (http_request->GetHeader("Content-Length", &value)) {
        int content_length = 0;
        if (StringToNumber(StringTrim(value), &content_length)) {
            return content_length;
        }
        return -1;
    }
    // (TODO) hsiaokangliu: Handle "Transfer-Encoding"
    if (http_request->GetHeader("Transfer-Encoding", &value)) {
        if (strcasecmp(value.c_str(), "chunked") == 0) {
        }
    }
    return 0;
}

void SimpleHttpServerHandler::HandleRequestComplete(
    const HttpRequest* http_request,
    HttpResponse* http_response,
    int64_t connection_id)
{
    // Add some default values for often used fields.
    if (!http_response->HasHeader("Content-Type")) {
        http_response->AddHeader("Content-Type", "text/html");
    }
    if (!http_response->HasHeader("Transfer-Encoding") &&
        !http_response->HasHeader("Content-Length")) {
        http_response->AddHeader("Content-Length",
                IntegerToString(http_response->http_body().size()));
    }

    if (http_request->IsKeepAlive() &&
        http_request->http_version() == HttpMessage::VERSION_1_0) {
        http_response->AddHeader("Connection", "Keep-Alive");
    }

    netframe::NetFrame::EndPoint endpoint(connection_id);
    std::string response = http_response->HeadersToString();
    response.append(http_response->http_body());
    m_http_server->mutable_net_frame()->SendPacket(
            static_cast<netframe::NetFrame::StreamEndPoint&>(endpoint),
            response.data(), response.size());

    // Client request that don't keep alive.
    if (!http_request->IsKeepAlive()) {
        VLOG(3) << "Not keep alive connection, close: " << connection_id;
        m_http_server->mutable_net_frame()->CloseEndPoint(endpoint, false);
    } else if (!http_response->IsKeepAlive()) {
        VLOG(3) << "Response set connection closed: " << connection_id;
        // User set the connection close.
        m_http_server->mutable_net_frame()->CloseEndPoint(endpoint, false);
    }
    delete http_request;
    delete http_response;
}

void SimpleHttpServerHandler::HandleMessage(HttpConnection* http_connection)
{
    HttpRequest* http_request = new HttpRequest();
    http_request->Swap(http_connection->mutable_http_request());
    HttpResponse* http_response = new HttpResponse;
    http_response->set_status(200);
    Closure<void>* done = NewClosure(this,
                                     &SimpleHttpServerHandler::HandleRequestComplete,
                                     (const HttpRequest*)http_request,
                                     http_response,
                                     http_connection->GetConnectionId());
    if (m_closure) {
        m_closure->Run(http_request, http_response, done);
    } else if (m_closure_with_connection) {
        m_closure_with_connection->Run(http_connection, http_request, http_response, done);
    }
}

