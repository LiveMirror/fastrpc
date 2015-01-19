// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/28/13
// Description: redis pool

#include "redis_pool.h"

namespace ipcs_common {

RedisConnectionPool::~RedisConnectionPool() {
    if (!m_disposed) {
        Dispose();
    }
}

bool RedisConnectionPool::Init(const std::string& host,
                               int port,
                               int timeout,
                               int redis_socket_timeout,
                               int table_index) {
    m_host = host;
    m_port = port;
    m_timeout = timeout;
    m_redis_socket_timeout = redis_socket_timeout;

    for (size_t i = 0; i < m_max_connections; ++i) {
        CRedisConnection* conn = new CRedisConnection();
        conn->Connect(m_host, m_port, m_timeout, m_redis_socket_timeout, table_index);
        m_connections.PushBack(conn);
    }

    m_disposed = false;
    return true;
}

bool RedisConnectionPool::Dispose() {
    if (m_disposed) return true;

    CRedisConnection* conn = NULL;
    for (size_t i = 0; i < m_max_connections; ++i) {
        m_connections.PopFront(&conn);
        conn->DisConnect();
        delete conn;
        conn = NULL;
    }
    m_disposed = true;
    return true;
}

CRedisConnection* RedisConnectionPool::Get() {
    /// blocking waiting
    CRedisConnection* res = NULL;
    m_connections.PopFront(&res);
    return res;
}

bool RedisConnectionPool::Release(CRedisConnection* conn) {
    /// blocking releasing
    m_connections.PushBack(conn);
    return true;
}

} // namespace ipcs_common


