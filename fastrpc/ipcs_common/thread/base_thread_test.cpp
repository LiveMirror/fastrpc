// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/22/13
// Description: base thread test

#include "base_thread.h"
#include "this_thread.h"
#include "gtest.h"

class TestThread : public ipcs_common::BaseThread
{
protected:
    virtual void Entry()
    {
        for (int i = 0; i < 4096; ++i) {
        }
    }
};

TEST(SimpleTestThread, Basic)
{
    TestThread a;
    EXPECT_FALSE(a.IsAlive());
    EXPECT_FALSE(a.IsJoinable());
    a.Start();
    a.Join();
    EXPECT_FALSE(a.IsAlive());
}
