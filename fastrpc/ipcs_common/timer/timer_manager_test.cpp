// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 03/31/13
// Description: timer manager test

#include "timer_manager.h"
#include "closure.h"
#include "this_thread.h"
#include "gtest.h"

using namespace ipcs_common;

static int gCount = 0;

class TestHelper
{
public:
    void Inc(int* gAddr, int64_t timer_id)
    {
        *gAddr += 1;
    }

    void Inc2(int *gAddr, int64_t timer_id)
    {
        *gAddr += 2;
    }
};

class TimerManagerTest : public testing::Test
{
public:
    TimerManagerTest()
    {
    }

    virtual ~TimerManagerTest()
    {

    }

protected:
    TimerManager m_timer_manager;
};

TEST_F(TimerManagerTest, BasicTest)
{
    TestHelper th;
    Closure<void, int64_t>* cb = NewClosure(&th, &TestHelper::Inc, &gCount);
    Closure<void, int64_t>* cb2 = NewClosure(&th, &TestHelper::Inc2, &gCount);
    Closure<void, int64_t>* cb3 = NewClosure(&th, &TestHelper::Inc, &gCount);
    Closure<void, int64_t>* cb4 = NewPermanentClosure(&th, &TestHelper::Inc, &gCount);

    ThisThread::Sleep(100);

    m_timer_manager.AddOneShotTimer(300, cb);
    ThisThread::Sleep(50);
    m_timer_manager.AddOneShotTimer(100, cb2);

    m_timer_manager.AddOneShotTimer(1000000, cb3);

    m_timer_manager.AddRepeatedTimer(100, cb4);

    ThisThread::Sleep(800);

    EXPECT_GE(gCount, 3);

    m_timer_manager.Stop();

    delete cb4;
}

TEST_F(TimerManagerTest, RepeatTest)
{
    TestHelper th;
    Closure<void, int64_t>* cb4 = NewPermanentClosure(&th, &TestHelper::Inc, &gCount);
    m_timer_manager.AddRepeatedTimer(100, cb4);

    ThisThread::Sleep(500);

    EXPECT_GE(gCount, 4);

    m_timer_manager.Stop();

    delete cb4;
}

TEST_F(TimerManagerTest, NegativeTest)
{
    TestHelper th;
    Closure<void, int64_t>* cb = NewClosure(&th, &TestHelper::Inc, &gCount);
    Closure<void, int64_t>* cb2 = NewPermanentClosure(&th, &TestHelper::Inc2, &gCount);

    m_timer_manager.AddOneShotTimer(100, cb);
    m_timer_manager.AddRepeatedTimer(100, cb2);

    ThisThread::Sleep(300);

    EXPECT_GE(gCount, 3);

    m_timer_manager.Stop();

    delete cb2;
}
