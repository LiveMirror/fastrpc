// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_HANDLER_H
#define IPCS_COMMON_NET_HTTP_HTTP_HANDLER_H

#include <map>
#include <string>
#include "closure.h"
#include "string_piece.h"
#include "http_connection.h"
#include "netframe.h"
#include "logging.h"

// Forward declaration.
class HttpServer;

// Describes how to complete a http service.
// All http connections of the same http service share the same
// service handler.

// CAUTION: One handler can only be registered to one determined http frame!
class HttpHandler {
public:
    HttpHandler() : m_http_server(NULL) {}
    virtual ~HttpHandler() { m_http_server = NULL; }

    // Note: OnConnected is only called at client side.
    virtual void OnConnected(HttpConnection* connection) {}
    virtual void OnClose(HttpConnection* connection, int error_code) {}
    // Handle a comlete message.
    virtual void HandleMessage(HttpConnection* http_connection) {}

    // The interface the derived class needs to implement.

    // Handle headers. It's called when http header received.
    virtual void HandleHeaders(HttpConnection* http_connection) = 0;
    // Handle a body packet. It's called when a body packet received.
    virtual void HandleBodyPacket(HttpConnection* http_connection,
            const StringPiece& string_piece) = 0;
    // Dectect the body packet size. If unknown, 0 is returned.
    virtual int DetectBodyPacketSize(HttpConnection* http_connection,
            const StringPiece& data) = 0;

    void set_http_server(HttpServer* http_server) {
        DCHECK(http_server != NULL);
        m_http_server = http_server;
    }

protected:
    // Hold a http server pointer, use it to send data safely.
    HttpServer* m_http_server;
};

// The simplest http closure, only accept request and response
typedef Closure<void, const HttpRequest*, HttpResponse*, Closure<void>*> HttpClosure;

// Similarly to HttpClosure, but with a extra HttpConnection parameter
typedef Closure<void, HttpConnection*, const HttpRequest*, HttpResponse*, Closure<void>*>
    HttpClosureWithConnection;

// SimpleHttpServerHandler accept a closure as parameter, user just need to
// process the only request and response, but not the binary traffic data.
class SimpleHttpServerHandler : public HttpHandler {
public:
    // We accept to kind of closure to make it both convenient and powerful
    SimpleHttpServerHandler(HttpClosure* closure, bool own_closure = true);
    SimpleHttpServerHandler(HttpClosureWithConnection* closure, bool own_closure = true);

    virtual ~SimpleHttpServerHandler();

private:
    virtual void HandleHeaders(HttpConnection* http_connection) {}
    virtual void HandleBodyPacket(HttpConnection* http_connection,
            const StringPiece& string_piece);
    virtual int DetectBodyPacketSize(HttpConnection* http_connection,
            const StringPiece& data);
    virtual void HandleMessage(HttpConnection* http_connection);

    void HandleRequestComplete(const HttpRequest* http_request,
                               HttpResponse* http_response,
                               int64_t connection_id);

private:
    // It should be a permanent closure.
    HttpClosure* m_closure;
    HttpClosureWithConnection* m_closure_with_connection;
    bool m_own_closure;
};

#endif // COMMON_NET_HTTP_HTTP_HANDLER_H
