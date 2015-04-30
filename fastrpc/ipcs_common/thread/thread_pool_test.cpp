// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 03/02/13
// Description: thread pool test

#include "thread_pool.h"
#include "this_thread.h"
#include "gtest.h"

class WorkingRoutine
{
public:
    void Work_Sleep_1000()
    {
        ipcs_common::ThisThread::Sleep(1000);
    }

    void Work_Sleep_100()
    {
        ipcs_common::ThisThread::Sleep(100);
    }
};

TEST(TestThreadPoolExceedNum, Basic)
{
    using namespace ipcs_common;

    WorkingRoutine wr;

    ThreadPool tp("tp", 2, 3, 400);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    int thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(1, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(2, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_100));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(3, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(3, thread_num);

    ThisThread::Sleep(600);
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(3, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(3, thread_num);

    tp.Terminate(true);

}

TEST(TestStackSize, Basic)
{
    using namespace ipcs_common;

    WorkingRoutine wr;

    ThreadPool tp("tp", 2, 3, 400, 32*1024);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    int thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(1, thread_num);
    tp.Terminate(true);
}

TEST(TestThreadWaitForTask, Basic)
{
    using namespace ipcs_common;

    WorkingRoutine wr;

    ThreadPool tp("tp", 2, -1, 400);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    int thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(1, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(2, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_100));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(3, thread_num);

    ThisThread::Sleep(200);
    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_100));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(4, thread_num);

    tp.Terminate(true);
}

TEST(ThreadPoolTest, Basic)
{
    using namespace ipcs_common;

    WorkingRoutine wr;

    ThreadPool tp("tp", 2, -1, 400);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));

    int thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(3, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_100));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(4, thread_num);

    ThisThread::Sleep(600);
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(3, thread_num);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(4, thread_num);

    tp.Terminate(true);
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(0, thread_num);
}

TEST(TestTerminateWithOutWait, Basic)
{
    using namespace ipcs_common;

    EXPECT_TRUE(ThisThread::IsMainThread());

    WorkingRoutine wr;

    ThreadPool tp("tp", 2, -1, 400);

    tp.AddTask(NewClosure(&wr, &WorkingRoutine::Work_Sleep_1000));
    int thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(1, thread_num);
    tp.Terminate(false);
    thread_num = tp.GetWorkingThreadNum();
    EXPECT_EQ(0, thread_num);
}
