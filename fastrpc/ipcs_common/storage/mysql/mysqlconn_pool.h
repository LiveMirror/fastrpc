// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/28/13
// Description: connection pool

#ifndef IPCS_COMMON_STORAGE_MYSQL_MYSQLCONN_POOL_H
#define IPCS_COMMON_STORAGE_MYSQL_MYSQLCONN_POOL_H
#pragma once

#include "uncopyable.h"
#include "blocking_queue.h"
#include "mysqlconn.h"

namespace ipcs_common {

class MysqlConnectionPool : private UnCopyable {
public:
    MysqlConnectionPool(size_t max_conns = 4) : m_max_connections(max_conns) {
    }

    virtual ~MysqlConnectionPool();

    bool Init(const std::string& host,
              const std::string& user,
              const std::string& passwd,
              const std::string& dbname,
              int port);

    bool Dispose();

    MysqlConnection* Get();

    bool Release(MysqlConnection* conn);

    size_t GetMaxConnections() const {
        return m_max_connections;
    }

private:
    BlockingQueue<MysqlConnection*> m_connections;
    size_t m_max_connections;

    std::string m_db_host;
    std::string m_db_user;
    std::string m_db_passwd;
    std::string m_db;
    int m_port;

    Atomic<bool> m_disposed;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_STORAGE_MYSQL_MYSQLCONN_POOL_H
