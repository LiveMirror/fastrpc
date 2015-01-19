// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: http base class

#ifndef IPCS_COMMON_NET_HTTP_HTTP_BASE_H
#define IPCS_COMMON_NET_HTTP_HTTP_BASE_H
#pragma once

#include "uncopyable.h"
#include "http_connection_manager.h"
#include "netframe.h"

// for historical reason, cocktail access our internal interface
// directly. But these members should be private.
// Add their classes as friend to make it compile, but should be removed in the
// future.

namespace cocktail {
namespace frontend {
class FrontendHandler;
}
}

class HttpServerConnection;
class SimpleHttpServerHandler;
class HttpServerListener;

class HttpBase {
    DECLARE_UNCOPYABLE(HttpBase);
    friend class HttpServerConnection;
    friend class SimpleHttpServerHandler;
    friend class HttpServerListener;
    friend class ::cocktail::frontend::FrontendHandler;

public:
    // 32M, add 16k reserved for headers
    static const int kMaxMessageSize = 32 * 1024 * 1024 + 16 * 1024;

protected:
    explicit HttpBase(netframe::NetFrame* net_frame,
                      bool own_net_frame = false,
                      int threads_number = 0);
    explicit HttpBase(int threads_number = 0);
    virtual ~HttpBase();

    virtual void Stop();

    netframe::NetFrame* mutable_net_frame() const {
        return m_net_frame;
    }

    ConnectionManager* mutable_connection_manager() {
        return &m_connection_manager;
    }

private:
    bool m_own_net_frame;
    netframe::NetFrame* m_net_frame;
    ConnectionManager m_connection_manager;
};

#endif // IPCS_COMMON_NET_HTTP_HTTP_BASE_H
