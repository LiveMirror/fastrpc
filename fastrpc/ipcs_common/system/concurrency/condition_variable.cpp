// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/28/13
// Description: condition variable

#include "condition_variable.h"
#include "mutex.h"
#include "posix_time.h"

namespace ipcs_common
{

ConditionVariable::ConditionVariable()
{
    pthread_condattr_t attr;
    CHECK_PTHREAD_ERROR(pthread_condattr_init(&attr));
    CHECK_PTHREAD_ERROR(pthread_cond_init(&m_cond, &attr));
    CHECK_PTHREAD_ERROR(pthread_condattr_destroy(&attr));
}

ConditionVariable::~ConditionVariable()
{
    CHECK_PTHREAD_ERROR(pthread_cond_destroy(&m_cond));
}

void ConditionVariable::Signal()
{
    CHECK_PTHREAD_ERROR(pthread_cond_signal(&m_cond));
}

void ConditionVariable::Broadcast()
{
    CHECK_PTHREAD_ERROR(pthread_cond_broadcast(&m_cond));
}

void ConditionVariable::Wait(MutexBase* mutex)
{
    CHECK_PTHREAD_ERROR(pthread_cond_wait(&m_cond, mutex->GetMutableMutex()));
}

bool ConditionVariable::WaitTime(MutexBase* mutex, int64_t time_in_ms)
{
    if (time_in_ms < 0) {
        Wait(mutex);
        return true;
    }

    struct timespec ts = {0, 0};

    bool trans = RelativeTimeToAbsTimeSpec(time_in_ms, &ts);
    if (!trans) {
        return false;
    }

    return CHECK_PTHREAD_TIMEDOUT_ERROR(pthread_cond_timedwait(&m_cond, mutex->GetMutableMutex(), &ts));
}

}


