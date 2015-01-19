// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/27/13
// Description: mysql connection

#include "mysqlconn.h"
#include <sstream>
#include "logging.h"

namespace ipcs_common {

int MysqlConnection::Connect() {
    return this->Connect(m_db_host.c_str(),
                         m_db_user.c_str(),
                         m_db_passwd.c_str(),
                         m_dbname.c_str(),
                         m_port);
}

int MysqlConnection::Connect(const std::string& host,
                             const std::string& user,
                             const std::string& passwd,
                             const std::string& dbname,
                             int port) {
    return this->Connect(host.c_str(),
                         user.c_str(),
                         passwd.c_str(),
                         dbname.c_str(),
                         port);
}

int MysqlConnection::Connect(const char* host,
                             const char* user,
                             const char* passwd,
                             const char* dbname,
                             int port) {
    if (NULL == host || NULL == user || NULL == passwd || NULL == dbname) {
        LOG(ERROR) << "invalid mysql connection string";
        return -1;
    }

    m_db_host = std::string(host);
    m_db_user = std::string(user);
    m_db_passwd = std::string(passwd);
    m_dbname = std::string(dbname);
    m_port = port;

    mysql_init(&m_mysql);

    if (!mysql_real_connect(&m_mysql, host, user, passwd, dbname, port, NULL, 0)) {
        LOG(ERROR) << "connect to db server failed : " << mysql_error(&m_mysql);
        return -2;
    }

    if (Execute("set names utf8") < 0 || Execute("SET AUTOCOMMIT = 1") < 0) {
        LOG(ERROR) << "set utf8 failed or set autocommit failed : " << mysql_error(&m_mysql);
        return -3;
    }

    m_closed = false;
    return 0;
}

unsigned long MysqlConnection::EscapeString(char *to,
                                            const char *from,
                                            unsigned long from_length) {
    return mysql_real_escape_string(&m_mysql, to, from, from_length);
}

MYSQL_RES* MysqlConnection::Query(const std::string& sql) {
    return this->Query(sql.c_str());
}

MYSQL_RES* MysqlConnection::Query(const char* sql)
{
    if (NULL == sql) {
        LOG(ERROR) << "empty sql statement";
        return NULL;
    }

    int ret = mysql_real_query(&m_mysql, sql, strlen(sql));
    if (ret) {
        if(2013 == mysql_errno(&m_mysql) || 2006 == mysql_errno(&m_mysql)) {
            LOG(ERROR) << mysql_error(&m_mysql);
            if (!mysql_real_connect(&m_mysql,
                                    m_db_host.c_str(),
                                    m_db_user.c_str(),
                                    m_db_passwd.c_str(),
                                    m_dbname.c_str(),
                                    m_port, NULL, 0)) {
                LOG(ERROR) << "connect db server failed : " << mysql_error(&m_mysql);
                return NULL;
            }

            if (Execute("set names utf8") < 0 || Execute("SET AUTOCOMMIT = 1") < 0) {
                LOG(ERROR) << "set utf8 failed : " << mysql_error(&m_mysql);
                return NULL;
            }

            ret = mysql_real_query(&m_mysql, sql, strlen(sql));
            if (ret) {
                LOG(ERROR) << "Query failed, error no : " << ret << ", " << mysql_error(&m_mysql);
                return NULL;
            }
        } else {
            LOG(ERROR) << sql << " failed: " << ret << ", " << mysql_error(&m_mysql);
            return NULL;
        }
    }

    return mysql_store_result(&m_mysql);
}

MYSQL_ROW MysqlConnection::GetNextRow(MYSQL_RES *res) {
    return mysql_fetch_row(res);
}

unsigned long* MysqlConnection::GetNextRowLength(MYSQL_RES *res) {
    return mysql_fetch_lengths(res);
}

unsigned long long MysqlConnection::GetRowNum(MYSQL_RES* res) {
    return mysql_num_rows(res);
}

void MysqlConnection::AutoCommit() {
}

int MysqlConnection::Execute(const std::string& sql) {
    return this->Execute(sql.c_str());
}

int MysqlConnection::Execute(const char* sql)
{
    if(sql == NULL) {
        LOG(ERROR) << "empty sql while trying to execute query";
        return -1;
    }

    int ret = mysql_real_query(&m_mysql, sql, strlen(sql));
    if (ret) {
        if (2013 == mysql_errno(&m_mysql) || 2006 == mysql_errno(&m_mysql)) {
            LOG(ERROR) << mysql_error(&m_mysql);
            if (!mysql_real_connect(&m_mysql,
                                    m_db_host.c_str(),
                                    m_db_user.c_str(),
                                    m_db_passwd.c_str(),
                                    m_dbname.c_str(),
                                    m_port, NULL, 0)) {
                LOG(ERROR) << "connect db server failed : " <<  mysql_error(&m_mysql);
                return -2;
            }

            if (Execute("set names utf8") < 0 || Execute("SET AUTOCOMMIT = 1") < 0) {
                LOG(ERROR) << "set utf8 failed : " << mysql_error(&m_mysql);
                return -3;
            }

            ret = mysql_real_query(&m_mysql, sql, strlen(sql));
            if(ret) {
                LOG(ERROR) << "query failed : " <<  mysql_error(&m_mysql);
                return -4;
            }
        } else {
            LOG(ERROR) << "execute failed : " << mysql_error(&m_mysql);
            return -4;
        }
    }
    return 0;
}

int MysqlConnection::ExecuteAndReturnAffectNum(const std::string& sql) {
    return this->ExecuteAndReturnAffectNum(sql.c_str());
}

int MysqlConnection::ExecuteAndReturnAffectNum(const char* sql)
{
    if(sql == NULL) {
        LOG(ERROR) << "empty sql while trying to execute query";
        return -1;
    }

    int ret = mysql_real_query(&m_mysql, sql, strlen(sql));
    if (ret) {
        if (2013 == mysql_errno(&m_mysql) || 2006 == mysql_errno(&m_mysql)) {
            LOG(ERROR) << mysql_error(&m_mysql);
            if (!mysql_real_connect(&m_mysql,
                                    m_db_host.c_str(),
                                    m_db_user.c_str(),
                                    m_db_passwd.c_str(),
                                    m_dbname.c_str(),
                                    m_port, NULL, 0)) {
                LOG(ERROR) << "connect db server failed : " <<  mysql_error(&m_mysql);
                return -2;
            }

            if (Execute("set names utf8") < 0 || Execute("SET AUTOCOMMIT = 1") < 0) {
                LOG(ERROR) << "set utf8 failed : " << mysql_error(&m_mysql);
                return -3;
            }

            ret = mysql_real_query(&m_mysql, sql, strlen(sql));
            if(ret) {
                LOG(ERROR) << "query failed : " <<  mysql_error(&m_mysql);
                return -4;
            }
        } else {
            LOG(ERROR) << "execute failed : " << mysql_error(&m_mysql);
            return -4;
        }
    }
    return mysql_affected_rows(&m_mysql);
}

int MysqlConnection::SetTimeOut(int time_out_in_sec) {
    std::stringstream ss;
    ss << "set interactive_timeout=" << time_out_in_sec;
    std::string to_str = ss.str();
    return this->Execute(to_str);
}

void MysqlConnection::Free(MYSQL_RES *res) {
    if (res == NULL) {
        return;
    }
    mysql_free_result(res);
}

void MysqlConnection::Close() {
    if (!m_closed) {
        mysql_close(&m_mysql);
        m_closed = true;
    }
}

} // namespace ipcs_common
