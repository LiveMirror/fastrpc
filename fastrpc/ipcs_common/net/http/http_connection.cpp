// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include <stdlib.h>
#include "errno.h"
#include "string.h"
#include "http_client.h"
#include "http_connection.h"
#include "http_server.h"
#include "logging.h"

using namespace ipcs_common;

HttpConnection::HttpConnection(netframe::NetFrame* net_frame) :
    netframe::StreamSocketHandler(*net_frame),
    m_http_handler(NULL),
    m_header_received(false) {
}

void HttpConnection::Close(bool immidiate) {
    GetNetFrame().CloseEndPoint(GetEndPoint(), immidiate);
}

bool HttpConnection::SendPacket(const void* data, size_t size) {
    if (GetNetFrame().SendPacket(GetEndPoint(), data, size) < 0) {
        LOG_EVERY_N(WARNING, 1000) << "Sending failed, remote address: "
            << GetRemoteAddress().ToString();
        return false;
    }
    return true;
}

bool HttpConnection::SendPacket(netframe::Packet* packet) {
    if (GetNetFrame().SendPacket(GetEndPoint(), packet) < 0) {
        LOG_EVERY_N(WARNING, 1000) << "Sending failed, remote address: "
            << GetRemoteAddress().ToString();
        delete packet;
        return false;
    }
    return true;
}

bool HttpConnection::OnSent(netframe::Packet* packet) {
    VLOG(4) << "Sending success, connection_id: " << GetConnectionId();
    return true;
}

bool HttpConnection::OnSendingFailed(netframe::Packet* packet, int error_code)
{
    if (error_code == ENOBUFS) {
        LOG_EVERY_N(WARNING, 1000) << "Sending failed, error: " << strerror(error_code)
            << ", remote address: " << GetRemoteAddress().ToString()
            << ", command queue size: " << GetCommandQueueLength();
    }
    return true;
}

int HttpConnection::DetectPacketSize(const void* data, size_t size) {
    if (!m_header_received) {
        return DetectHeaderSize(data, size);
    } else {
        StringPiece string_piece(reinterpret_cast<const char*>(data), size);
        return m_http_handler->DetectBodyPacketSize(this, string_piece);
    }
}

int HttpConnection::DetectHeaderSize(const void* data, size_t size) {
    static const char kHeaderEnd_1[] = "\r\n\r\n";
    static const size_t kHeaderEndLength_1 = 4;
    // Here we assume there is no malformed http message.
    const char* p = reinterpret_cast<const char*>(
            memmem(data, size, kHeaderEnd_1, kHeaderEndLength_1));
    if (p != NULL) {
        return p - reinterpret_cast<const char*>(data) + kHeaderEndLength_1;
    }
    static const char kHeaderEnd_2[] = "\n\n";
    static const size_t kHeaderEndLength_2 = 2;
    p = reinterpret_cast<const char*>(
            memmem(data, size, kHeaderEnd_2, kHeaderEndLength_2));
    if (p != NULL) {
        return p - reinterpret_cast<const char*>(data) + kHeaderEndLength_2;
    }
    return 0;
}

void HttpConnection::SetTos(int tos)
{
    // don't set on win32
#ifndef _WIN32
    if (tos > 0) {
        StreamSocket socket;
        int socket_fd = GetEndPoint().GetFd();
        socket.Attach(socket_fd);
        socket.SetOption(SOL_IP, IP_TOS, static_cast<uint8_t>(tos));
        socket.SetOption(SOL_SOCKET, SO_PRIORITY, tos);
        socket.Detach();
    }
#endif
}
