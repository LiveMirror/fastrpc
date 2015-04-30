// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 03/01/13
// Description: condition variable test

#include "condition_variable.cpp"
#include "closure.h"
#include "mutex.h"
#include "this_thread.h"
#include "thread.h"
#include "gtest.h"

class Routine
{
public:
    void Wait()
    {
        MutexLocker locker(m_mutex);
        m_cond.Wait(&m_mutex);
    }

    void Signal()
    {
        MutexLocker locker(m_mutex);
        m_cond.Signal();
    }

    void TimedWait()
    {
        MutexLocker locker(m_mutex);
        bool r = m_cond.WaitTime(&m_mutex, 100);
        EXPECT_FALSE(r);
    }

private:
    ipcs_common::Mutex m_mutex;
    ipcs_common::ConditionVariable m_cond;
};

TEST(CV, Basic)
{
    using namespace ipcs_common;
    Routine r;
    Thread t_wait(NewClosure(&r, &Routine::Wait));
    Thread t_signal(NewClosure(&r, &Routine::Signal));
    Thread t_timedwait(NewClosure(&r, &Routine::TimedWait));

    t_wait.Start();
    ThisThread::Sleep(100);
    t_signal.Start();
    t_timedwait.Start();

    t_wait.Join();
    t_signal.Join();
    t_timedwait.Join();
}
