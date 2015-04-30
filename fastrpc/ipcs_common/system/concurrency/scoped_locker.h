// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/21/13
// Description: scoped_locker

#ifndef IPCS_COMMON_SYSTEM_CONCURRENCY_SCOPED_LOCKER_H
#define IPCS_COMMON_SYSTEM_CONCURRENCY_SCOPED_LOCKER_H
#pragma once

#include "uncopyable.h"

namespace ipcs_common
{

// scoped_locker looks like scoped_ptr. It acquires
// lock when it is constructed and releases it when it is
// destructed.
// LockType should implement Lock, UnLock and TryLock methods.
template <class LockType>
class scoped_locker : private UnCopyable
{
public:
    explicit scoped_locker(LockType& lock) : m_locker(&lock)
    {
        m_locker->Lock();
    }

    explicit scoped_locker(LockType* lock) : m_locker(lock)
    {
        m_locker->Lock();
    }

    ~scoped_locker()
    {
        m_locker->UnLock();
    }

private:
    LockType* m_locker;
};

template <class LockType>
class scoped_trylocker : private UnCopyable
{
public:
    explicit scoped_trylocker(LockType& lock) : m_locker(&lock)
    {
        m_locked = m_locker->TryLock();
    }

    explicit scoped_trylocker(LockType* lock) : m_locker(lock)
    {
        m_locked = m_locker->TryLock();
    }

    ~scoped_trylocker()
    {
        if (m_locked) {
            m_locker->UnLock();
        }
    }

    bool IsLocked() const
    {
        return m_locked;
    }

private:
    LockType* m_locker;
    bool m_locked;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_SYSTEM_CONCURRENCY_SCOPED_LOCKER_H
