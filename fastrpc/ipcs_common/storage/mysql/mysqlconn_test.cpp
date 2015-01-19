// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/28/13
// Description: mysql connection test

#include "mysqlconn.h"
#include "gtest.h"

class MysqlTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    ipcs_common::MysqlConnection m_mysqlconn;
};

TEST_F(MysqlTest, MysqlBasicConnectionTest) {
    std::string host("192.168.0.242");
    std::string user("ftuser");
    std::string passwd("123456");
    std::string db("fantianzhuan");
    int port = 8091;

    int ret = m_mysqlconn.Connect(host, user, passwd, db, port);
    ASSERT_EQ(ret, 0);

    m_mysqlconn.Close();
}

TEST_F(MysqlTest, MysqlBasicExecuteTest) {
    std::string host("192.168.0.242");
    std::string user("ftuser");
    std::string passwd("123456");
    std::string db("fantianzhuan");
    int port = 8091;

    int ret = m_mysqlconn.Connect(host, user, passwd, db, port);
    ASSERT_EQ(ret, 0);

    std::string query("show tables");

    MYSQL_RES* res = m_mysqlconn.Query(query);

    EXPECT_TRUE(res != NULL);

    MYSQL_ROW row = NULL;

    row = m_mysqlconn.GetNextRow(res);

    EXPECT_TRUE(row != NULL);

    while (row) {
        std::cout << row[0] << std::endl;
        row = m_mysqlconn.GetNextRow(res);
    }

    m_mysqlconn.Free(res);

    m_mysqlconn.Close();
    m_mysqlconn.Close();
}
