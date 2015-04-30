// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/28/13
// Description: pthread cond

#ifndef IPCS_COMMON_SYSTEM_CONCURRENCY_CONDITION_VARIABLE_H
#define IPCS_COMMON_SYSTEM_CONCURRENCY_CONDITION_VARIABLE_H
#pragma once

#include <pthread.h>
#include <stdint.h>
#include "uncopyable.h"

namespace ipcs_common
{
class MutexBase;

class ConditionVariable : private UnCopyable
{
public:
    ConditionVariable();
    ~ConditionVariable();

    void Signal();
    void Broadcast();

    void Wait(MutexBase* mutex);
    void Wait(MutexBase& mutex) {
        Wait(&mutex);
    }

    bool WaitTime(MutexBase* mutex, int64_t time_in_ms);
    bool WaitTime(MutexBase& mutex, int64_t time_in_ms) {
        return WaitTime(&mutex, time_in_ms);
    }

private:
    pthread_cond_t m_cond;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_SYSTEM_CONCURRENCY_CONDITION_VARIABLE_H
