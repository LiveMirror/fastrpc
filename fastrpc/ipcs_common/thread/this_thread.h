// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/21/13
// Description: related to thread static methods
//              like pthread_self, sleep, yield, exit.

#ifndef IPCS_COMMON_THREAD_THIS_THREAD_H
#define IPCS_COMMON_THREAD_THIS_THREAD_H
#pragma once

#include <pthread.h>
#include <stdint.h>

namespace ipcs_common
{

class ThisThread
{
public:
    static pthread_t GetHandle();
    static pid_t GetId();
    static bool IsMainThread();
    static void Sleep(int64_t time_in_ms);
    static void Yield();
    static void Exit();
    static int GetLastErrorCode();
};

} // namespace ipcs_common

#endif // IPCS_COMMON_THREAD_THIS_THREAD_H
