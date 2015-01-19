// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/21/13
// Description: mutex

#ifndef IPCS_COMMON_SYSTEM_CONCURRENCY_MUTEX_H
#define IPCS_COMMON_SYSTEM_CONCURRENCY_MUTEX_H
#pragma once

#include <assert.h>
#include <pthread.h>
#include "uncopyable.h"
#include "check_error.h"
#include "scoped_locker.h"

namespace ipcs_common
{

class MutexBase : private UnCopyable
{
protected:
    explicit MutexBase(int mutex_type)
    {
        pthread_mutexattr_t attr;
        CHECK_PTHREAD_ERROR(pthread_mutexattr_init(&attr));
        CHECK_PTHREAD_ERROR(pthread_mutexattr_settype(&attr, mutex_type));
        CHECK_PTHREAD_ERROR(pthread_mutex_init(&m_mutex, &attr));
        CHECK_PTHREAD_ERROR(pthread_mutexattr_destroy(&attr));
    }

    ~MutexBase()
    {
        CHECK_PTHREAD_ERROR(pthread_mutex_destroy(&m_mutex));
    }

protected:
    int EnableDebug(int mutex_type)
    {
        if (mutex_type == PTHREAD_MUTEX_RECURSIVE) {
            return mutex_type;
        }
        else {
#ifdef NDEBUG
            return mutex_type;
#endif
        }
        return PTHREAD_MUTEX_ERRORCHECK;
    }

public:
    bool IsLocked() const
    {
        // Not support now for gcc 3.4, always returns true
        // return m_mutex.__data.__lock > 0;
        return true;
    }

    void Lock()
    {
        CHECK_PTHREAD_ERROR(pthread_mutex_lock(&m_mutex));
        assert(IsLocked());
    }

    void UnLock()
    {
        assert(IsLocked());
        CHECK_PTHREAD_ERROR(pthread_mutex_unlock(&m_mutex));
    }

    bool TryLock()
    {
        return CHECK_PTHREAD_TRYLOCK_ERROR(pthread_mutex_trylock(&m_mutex));
    }

    pthread_mutex_t* GetMutableMutex()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

class Mutex : public MutexBase
{
public:
    Mutex() : MutexBase(EnableDebug(PTHREAD_MUTEX_ERRORCHECK))
    {}
    ~Mutex() {}
};

} // namespace ipcs_common

typedef ipcs_common::scoped_locker<ipcs_common::Mutex> MutexLocker;
typedef ipcs_common::scoped_trylocker<ipcs_common::Mutex> MutexTryLocker;

#endif // IPCS_COMMON_SYSTEM_CONCURRENCY_MUTEX_H
