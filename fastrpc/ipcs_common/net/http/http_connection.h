// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_CONNECTION_H
#define IPCS_COMMON_NET_HTTP_HTTP_CONNECTION_H

#include <set>
#include <string>
#include <vector>
#include "string_piece.h"
#include "http_message.h"
#include "http_stats.h"
#include "netframe.h"
#include "socket_handler.h"

class HttpServer;
class HttpClient;
class HttpHandler;

// Represents a http connection.
class HttpConnection : public netframe::StreamSocketHandler {
public:
    explicit HttpConnection(netframe::NetFrame* net_frame);
    virtual ~HttpConnection() {}

    int64_t GetConnectionId() {
        return GetEndPoint().GetId();
    }

    const HttpRequest& http_request() const {
        return m_http_request;
    }
    HttpRequest* mutable_http_request() {
        return &m_http_request;
    }
    HttpResponse* mutable_http_response() {
        return &m_http_response;
    }
    std::set<int>* mutable_peer_supported_compress_types() {
        return &m_peer_supported_compress_types;
    }
    const std::set<int>& peer_supported_compress_types() const {
        return m_peer_supported_compress_types;
    }

    // Close the connection. 'immidiate' is defaultly set to false. It means
    // connection will be closed when all pendding packets are sent.
    void Close(bool immidiate = false);
    bool SendPacket(const void* data, size_t size);
    bool SendPacket(netframe::Packet* packet);

    void SetTos(int tos);

protected:
    virtual bool OnSent(netframe::Packet* packet);
    virtual bool OnSendingFailed(netframe::Packet* packet, int error_code);
    virtual int  DetectPacketSize(const void* data, size_t size);
    virtual int  DetectHeaderSize(const void* data, size_t size);

    virtual void OnConnected() = 0;
    virtual void OnClose(int error_code) = 0;
    virtual void OnReceived(const netframe::Packet& packet) = 0;

    HttpHandler* m_http_handler;
    HttpRequest  m_http_request;
    HttpResponse m_http_response;
    bool         m_header_received;
    std::set<int> m_peer_supported_compress_types;
};

#endif // IPCS_COMMON_NET_HTTP_HTTP_CONNECTION_H
