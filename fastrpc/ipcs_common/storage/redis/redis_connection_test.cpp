// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 06/14/13
// Description: redis_test

#include "redis_connection.h"
#include <sstream>
#include "gtest.h"
#include "redis_pb_test_info.pb.h"

class RedisConnectionTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    ipcs_common::CRedisConnection m_redis_conn;;
};

TEST_F(RedisConnectionTest, RedisBasicTest) {
    int ret = m_redis_conn.Connect("192.168.10.193", 6379, 2, 2, 1);
    ASSERT_EQ(ret, 0);

    std::string key("name");
    std::stringstream ss;
    ss << "set " << key << " pengchong";
    std::string command;
    command = ss.str();
    int command_ret = m_redis_conn.SafeCommand(command.c_str());
    EXPECT_EQ(command_ret, 0);

    char result[1024] = {'\0'};
    int result_len = 1024;
    command_ret = m_redis_conn.SafeGetStr(key.c_str(), static_cast<int>(key.size()), result, result_len);
    EXPECT_EQ(command_ret, 0);
    EXPECT_EQ(result_len, 9);
    EXPECT_STREQ(result, "pengchong");

    ss.str("");

    ss << "del " << key;
    command = "";

    command = ss.str();
    command_ret = m_redis_conn.SafeCommand(command.c_str());

    EXPECT_EQ(command_ret, 0);

    ss.str("");
    ss << "zadd testset 1 a 1 b";
    command_ret = m_redis_conn.SafeCommand(ss.str().c_str());
    EXPECT_EQ(command_ret, 0);

    ss.str("");
    ss << "zrange testset 0 -1";
    std::vector<std::string> set;

    command_ret = m_redis_conn.SafeGetSetCommand(ss.str().c_str(), &set);

    EXPECT_EQ((size_t)2, set.size());

    std::vector<std::string>::iterator iter = set.begin();
    bool eq = *iter == std::string("a");
    EXPECT_TRUE(eq);
    iter++;
    eq = *iter == std::string("b");
    EXPECT_TRUE(eq);

    ipcs_common::RedisTestObject test_object;
    test_object.set_sun(101);
    test_object.set_moon(102);
    test_object.set_mercury("yes");
    command_ret = m_redis_conn.SetProtoMessage("test_object_key", test_object);
    EXPECT_EQ(0, command_ret);

    char* bytes = new char[test_object.ByteSize()];
    int msg_len = test_object.ByteSize();

    command_ret = m_redis_conn.ReadProtoMessage("test_object_key", bytes, msg_len);
    ipcs_common::RedisTestObject checker;
    checker.ParseFromArray(bytes, msg_len);
    EXPECT_EQ(0, command_ret);
    EXPECT_EQ(101, checker.sun());
    EXPECT_EQ(102, checker.moon());
    EXPECT_TRUE("yes" == checker.mercury());
    delete[] bytes;

    ipcs_common::RedisTestObject checker2;
    command_ret = m_redis_conn.ReadProtoMessage("test_object_key", &checker2, test_object.ByteSize());
    EXPECT_EQ(0, command_ret);
    EXPECT_EQ(101, checker2.sun());
    EXPECT_EQ(102, checker2.moon());
    EXPECT_TRUE("yes" == checker2.mercury());

    ipcs_common::RedisTestObject checker3;
    command_ret = m_redis_conn.ReadProtoMessage("test_object_key", &checker3);
    EXPECT_TRUE(command_ret < 0);

    std::string hash_name = "team_hash";
    std::string team_id_1 = "1";
    std::string team_id_1_val = "1 is best";
    std::string team_id_2 = "2";
    std::string team_id_2_val = "2 is better";

    command_ret = m_redis_conn.SafeHashSet(hash_name.c_str(), hash_name.size(),
                                           team_id_1.c_str(), team_id_1.size(),
                                           team_id_1_val.c_str(), team_id_1_val.size());
    EXPECT_EQ(command_ret, 0);

    command_ret = m_redis_conn.SafeHashSet(hash_name.c_str(), hash_name.size(),
                                           team_id_2.c_str(), team_id_2.size(),
                                           team_id_2_val.c_str(), team_id_2_val.size());

    std::vector<std::string> hkeys;

    command_ret = m_redis_conn.SafeHashKeys(hash_name.c_str(), hash_name.size(),
                                            &hkeys);
    EXPECT_TRUE(hkeys[0] == "1" || hkeys[0] == "2");
    EXPECT_TRUE(hkeys[1] == "2" || hkeys[1] == "1");

    command_ret = m_redis_conn.SafeHashDelKey(hash_name.c_str(), hash_name.size(),
                                              team_id_1.c_str(), team_id_1.size());
    EXPECT_EQ(command_ret, 0);

    command_ret = m_redis_conn.SafeHashKeys(hash_name.c_str(), hash_name.size(),
                                            &hkeys);

    EXPECT_TRUE(hkeys[0] == "2");

    m_redis_conn.DisConnect();
};
