// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: http client lib

#ifndef IPCS_COMMON_NET_HTTP_HTTP_CLIENT_H
#define IPCS_COMMON_NET_HTTP_HTTP_CLIENT_H
#pragma once

#include <map>
#include <string>
#include <vector>
#include "http_base.h"
#include "http_handler.h"

class HttpClientConnection;

class HttpClient : public HttpBase {
    DECLARE_UNCOPYABLE(HttpClient);
public:
    friend class HttpClientConnection;
    explicit HttpClient(netframe::NetFrame* net_frame,
            bool own_net_frame = false,
            int threads_number = 0) :
        HttpBase(net_frame, own_net_frame, threads_number) {
    }
    explicit HttpClient(int threads_number = 0) : HttpBase(threads_number) {}
    virtual ~HttpClient() {}

    // Connect to a server.
    virtual bool ConnectServer(const std::string& address, HttpHandler* handler);
};

class HttpClientConnection : public HttpConnection {
public:
    HttpClientConnection(HttpClient* http_client, HttpHandler* handler);
    virtual ~HttpClientConnection();

protected:
    friend class HttpClient;
    void set_http_client(HttpClient* http_client) {
        m_http_client = http_client;
    }
    virtual void OnConnected();
    virtual void OnClose(int error_code);
    virtual void OnReceived(const netframe::Packet& packet);
private:
    HttpClient* m_http_client;
};

#endif // IPCS_COMMON_NET_HTTP_HTTP_CLIENT_H
