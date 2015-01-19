// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Created: 06/27/14
// Description: mongodb connection test

#include "CmongoPool.h"
#include "gtest.h"

class MongoTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    ipcs_common::CmongoPool* mongo_pool;
};

TEST_F(MongoTest, MysqlBasicConnectionTest) {
    std::string host("192.168.1.13");
    int port = 8091;
    int pool_size = 10;
    mongo_pool = new CmongoPool(pool_size, host, port);
    DBClientConnection* pcon = mongo_pool->get_con();

    std::string table = "db1.col1";
    //BSONObj insertsql = BSON("id"<<1<<"value"<<"hello");
    std::string str = "{\"id\":1,\"value\":\"hello\"}";
    BSONObj insertsql = fromjson(str);
    pcon->insert(table,insertsql,0);

    Query condition = QUERY("id"<<1);
    int pagesize = 1;
    int offset = 0;
    BSONObj columns = BSON("_id"<<0<<"id"<<1<<"value"<<1);
    auto_ptr<DBClientCursor> cursor = pcon->query(table, condition,
                                                  pagesize, offset,
                                                  &columns, 0, 0);
    std::string sContent;
    while(cursor->more()) {
        BSONObj one_row = cursor->next();
        sContent += one_row.jsonString() + "<br/>";
    }

    mongo_pool->release(pcon);

    ASSERT_EQ(sContent, str);

    delete mongo_pool;
}

