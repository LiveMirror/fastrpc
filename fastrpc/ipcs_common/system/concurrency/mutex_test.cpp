// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/22/13
// Description: mutex test

#include "mutex.h"
#include "gtest.h"

TEST(MutexTest, Basic)
{
    ipcs_common::Mutex lock;
    {
        MutexLocker locker(lock);
    }
}

