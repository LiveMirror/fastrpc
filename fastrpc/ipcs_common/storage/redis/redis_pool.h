// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/28/13
// Description: connection pool

#ifndef IPCS_COMMON_STORAGE_REDIS_REDIS_POOL_H
#define IPCS_COMMON_STORAGE_REDIS_REDIS_POOL_H
#pragma once

#include "atomic.h"
#include "uncopyable.h"
#include "blocking_queue.h"
#include "redis_connection.h"

namespace ipcs_common {

class RedisConnectionPool : private UnCopyable {
public:
    RedisConnectionPool(size_t max_conns = 20) : m_max_connections(max_conns) {
    }

    virtual ~RedisConnectionPool();

    bool Init(const std::string& host,
              int port,
              int timeout,
              int redis_socket_timeout,
              int table_index=0);

    bool Dispose();

    CRedisConnection* Get();

    bool Release(CRedisConnection* conn);

    size_t GetMaxConnections() const {
        return m_max_connections;
    }

private:
    BlockingQueue<CRedisConnection*> m_connections;
    size_t m_max_connections;

    std::string m_host;
    int m_port;
    int m_timeout;
    int m_redis_socket_timeout;

    Atomic<bool> m_disposed;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_STORAGE_REDIS_REDIS_POOL_H
