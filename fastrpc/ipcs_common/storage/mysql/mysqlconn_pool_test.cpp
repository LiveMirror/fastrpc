// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/28/13
// Description: pool test

#include "mysqlconn_pool.h"
#include "gtest.h"

class MysqlPoolTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    ipcs_common::MysqlConnectionPool m_pool;
};

TEST_F(MysqlPoolTest, MysqlPoolBasic) {
    std::string host("192.168.0.242");
    std::string user("ftuser");
    std::string passwd("123456");
    std::string db("fantianzhuan");
    int port = 8091;

    m_pool.Init(host, user, passwd, db, port);

    ipcs_common::MysqlConnection* conn = m_pool.Get();

    std::string query("show tables");

    MYSQL_RES* res = conn->Query(query);

    EXPECT_TRUE(res != NULL);

    conn->Free(res);

    m_pool.Release(conn);

    m_pool.Dispose();
}
