// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_client.h"
#include "string.h"
#include "string_number.h"
#include "logging.h"

using namespace ipcs_common;

bool HttpClient::ConnectServer(const std::string& server_address,
        HttpHandler* handler) {
    SocketAddressInet address(server_address);
    HttpClientConnection* connection = new HttpClientConnection(this, handler);
    int64_t socket_id = mutable_net_frame()->AsyncConnect(
            address,
            connection,
            kMaxMessageSize);
    if (socket_id < 0) {
        LOG(ERROR) << "Failed to connect to server: " << server_address;
        delete connection;
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// HttpClientConnection staff

HttpClientConnection::HttpClientConnection(HttpClient* http_client,
        HttpHandler* http_handler) :
    HttpConnection(http_client->mutable_net_frame()),
    m_http_client(http_client) {
    CHECK_NOTNULL(http_handler);
    m_http_handler = http_handler;
}

HttpClientConnection::~HttpClientConnection() {
    delete m_http_handler;
}

void HttpClientConnection::OnConnected() {
    m_http_client->mutable_connection_manager()->AddConnection(GetConnectionId());
    m_http_handler->OnConnected(this);
}

void HttpClientConnection::OnClose(int error_code) {
    m_http_handler->OnClose(this, error_code);
    m_http_client->mutable_connection_manager()->RemoveConnection(
            GetConnectionId());

    // Reset the connection status.
    m_header_received = false;
}

void HttpClientConnection::OnReceived(const netframe::Packet& packet) {
    bool message_completed = false;
    if (!m_header_received) {
        m_header_received = true;
        StringPiece header(reinterpret_cast<const char*>(packet.Content()),
                           packet.Length());
        m_http_response.Reset();
        HttpMessage::ErrorType error;
        if (!m_http_response.ParseHeaders(header, &error)) {
            LOG(INFO) << "Failed to parse the http header: " << header << "\n"
                << "Error: " << error;
            Close();
            return;
        }
        m_http_handler->HandleHeaders(this);

        // Check if the message is complete
        std::string value;
        if (m_http_response.status() < 200 ||
                m_http_response.status() == 204 ||
                m_http_response.status() == 304) {
            message_completed = true;
        } else if (m_http_response.GetHeader("Content-Length", &value)) {
            int content_length = 0;
            if (StringToNumber(value, &content_length) && content_length == 0) {
                message_completed = true;
            }
        }
        if (message_completed) {
            m_header_received = false;
            m_http_handler->HandleMessage(this);
        }
    } else {
        StringPiece data(reinterpret_cast<const char*>(packet.Content()),
                         packet.Length());
        m_http_handler->HandleBodyPacket(this, data);

        // Check if the message is complete
        std::string value;
        if (m_http_response.GetHeader("Transfer-Encoding", &value)) {
            // (TODO) handle transfer encoding.
            if (strcasecmp(value.c_str(), "chunked") == 0) {
                if (packet.Length() == 5) { // 0\r\n\r\n
                    message_completed = true;
                }
            }
        } else if (m_http_response.GetHeader("Content-Length", &value)) {
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
