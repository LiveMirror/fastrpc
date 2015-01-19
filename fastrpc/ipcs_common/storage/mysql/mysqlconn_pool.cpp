// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/28/13
// Description: mysql pool

#include "mysqlconn_pool.h"

namespace ipcs_common {

MysqlConnectionPool::~MysqlConnectionPool() {
    if (!m_disposed) {
        Dispose();
    }
}

bool MysqlConnectionPool::Init(const std::string& host,
                               const std::string& user,
                               const std::string& passwd,
                               const std::string& dbname,
                               int port) {
    m_db_host = host;
    m_db_user = user;
    m_db_passwd = passwd;
    m_db = dbname;
    m_port = port;

    for (size_t i = 0; i < m_max_connections; ++i) {
        MysqlConnection* conn = new MysqlConnection(host, user, passwd, dbname, port);
        conn->Connect();
        m_connections.PushBack(conn);
    }

    m_disposed = false;
    return true;
}

bool MysqlConnectionPool::Dispose() {
    if (m_disposed) return true;

    MysqlConnection* conn = NULL;
    for (size_t i = 0; i < m_max_connections; ++i) {
        m_connections.PopFront(&conn);
        conn->Close();
        delete conn;
        conn = NULL;
    }
    m_disposed = true;
    return true;
}

MysqlConnection* MysqlConnectionPool::Get() {
    /// blocking waiting
    MysqlConnection* res = NULL;
    m_connections.PopFront(&res);
    return res;
}

bool MysqlConnectionPool::Release(MysqlConnection* conn) {
    /// blocking releasing
    m_connections.PushBack(conn);
    return true;
}

} // namespace ipcs_common


