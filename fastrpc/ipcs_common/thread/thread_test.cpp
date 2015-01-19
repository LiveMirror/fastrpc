// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/25/13
// Description: thread test

#include "thread.h"
#include <iostream>
#include "gtest.h"
#include "this_thread.h"

class TestTask
{
public:
    void Work()
    {
        ipcs_common::ThisThread::Sleep(100);
        EXPECT_FALSE(ipcs_common::ThisThread::IsMainThread());
    }

    void Run(unsigned long long param)
    {
        std::cout<< "param " << param << std::endl;
    }
};

void StartTask(void* context, unsigned long long param)
{
    static_cast<TestTask*>(context)->Run(param);
}

TEST(ThreadTest, Basic)
{
    TestTask tt;

    using namespace ipcs_common;
    Thread t(NewClosure(&tt, &TestTask::Work));
    EXPECT_FALSE(t.IsAlive());
    t.Start();
    ipcs_common::ThisThread::Sleep(50);
    EXPECT_TRUE(t.IsAlive());
    t.Join();
    EXPECT_FALSE(t.IsAlive());

    EXPECT_TRUE(ipcs_common::ThisThread::IsMainThread());

    Thread t2(StartTask, static_cast<void*>(&tt), 1LL);
    t2.Start();

    Thread t3(Bind(&TestTask::Work, &tt));
    t3.Start();
}

