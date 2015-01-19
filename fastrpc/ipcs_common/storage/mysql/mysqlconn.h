// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/27/13
// Description: mysql connection

#ifndef IPCS_COMMON_STORAGE_MYSQL_MYSQLCONN2_H
#define IPCS_COMMON_STORAGE_MYSQL_MYSQLCONN2_H
#pragma once

#include <string>
#include "atomic.h"
#include "mysql.h"

namespace ipcs_common {

class MysqlConnection {
public:
    MysqlConnection() : m_db_host(""),
                        m_db_user(""),
                        m_db_passwd(""),
                        m_dbname(""),
                        m_port(0),
                        m_closed(true)
    {}

    MysqlConnection(const std::string& host,
                    const std::string& user,
                    const std::string& passwd,
                    const std::string& dbname,
                    int port) : m_db_host(host),
                                m_db_user(user),
                                m_db_passwd(passwd),
                                m_dbname(dbname),
                                m_port(port),
                                m_closed(true) {
    }

    virtual ~MysqlConnection() {
        if (!m_closed) {
            Close();
            m_closed = true;
        }
    }

    int Connect();

    int Connect(const std::string& host,
                const std::string& user,
                const std::string& passwd,
                const std::string& dbname,
                int port);

    int Connect(const char* host,
                const char* user,
                const char* passwd,
                const char* dbname,
                int port);

    unsigned long EscapeString(char *to,
                               const char *from,
                               unsigned long from_length);

    MYSQL_RES* Query(const char *sql);

    MYSQL_RES* Query(const std::string& sql);

    MYSQL_ROW GetNextRow(MYSQL_RES *res);

    unsigned long* GetNextRowLength(MYSQL_RES *res);

    unsigned long long GetRowNum(MYSQL_RES* res);

    int Execute(const char* sql);

    int Execute(const std::string& sql);

    int ExecuteAndReturnAffectNum(const char* sql);

    int ExecuteAndReturnAffectNum(const std::string& sql);

    int SetTimeOut(int time_out_in_sec);

    void Free(MYSQL_RES *res);

    void Close();

    void AutoCommit();

    void UnCommit();

private:
    MYSQL m_mysql;
    std::string m_db_host;
    std::string m_db_user;
    std::string m_db_passwd;
    std::string m_dbname;
    int m_port;
    ipcs_common::Atomic<bool> m_closed;
    static const int kMaxSqlLength = 500;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_STORAGE_MYSQL_MYSQLCONN2_H
