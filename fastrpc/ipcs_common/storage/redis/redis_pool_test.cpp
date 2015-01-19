// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/28/13
// Description: pool test

#include "redis_pool.h"
#include <string>
#include <sstream>
#include "gtest.h"

class RedisPoolTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    ipcs_common::RedisConnectionPool m_pool;
};

TEST_F(RedisPoolTest, RedisPoolBasic) {
    std::string host("192.168.10.193");
    int port = 6379;
    int timeout = 2;
    int socket_timeout = 2;
    int table_index = 15;

    m_pool.Init(host, port, timeout, socket_timeout, table_index);

    ipcs_common::CRedisConnection* conn = m_pool.Get();

    std::string key("name");
    std::stringstream ss;
    ss << "set " << key << " pengchong";
    std::string command;
    command = ss.str();
    int command_ret = conn->SafeCommand(command.c_str());
    EXPECT_EQ(command_ret, 0);

    char result[1024] = {'\0'};
    int result_len = 1024;
    command_ret = conn->SafeGetStr(key.c_str(), static_cast<int>(key.size()), result, result_len);
    EXPECT_EQ(command_ret, 0);
    EXPECT_EQ(result_len, 9);
    EXPECT_STREQ(result, "pengchong");

    ss.str("");

    ss << "del " << key;
    command = "";

    command = ss.str();
    command_ret = conn->SafeCommand(command.c_str());

    m_pool.Release(conn);

    m_pool.Dispose();
}
