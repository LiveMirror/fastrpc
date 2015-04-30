// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 03/30/13
// Description: timer manager

#ifndef IPCS_COMMON_TIMER_TIMER_MANAGER_H
#define IPCS_COMMON_TIMER_TIMER_MANAGER_H
#pragma once

#include <stdint.h>
#include <deque>
#include <queue>
#include <map>
#include "atomic.h"
#include "closure.h"
#include "uncopyable.h"
#include "condition_variable.h"
#include "mutex.h"
#include "base_thread.h"

namespace ipcs_common
{

class TimerManager : private BaseThread, private UnCopyable
{
public:
    // Returns void and has int64_t as timer id
    typedef Closure<void, int64_t> TimerClosure;

    class Timeout
    {
    public:
        int64_t m_timer_id;
        int64_t m_timeout;

        bool operator>(const Timeout& rhs) const {
            return m_timeout > rhs.m_timeout;
        }
    };

    class TimerEntry
    {
    public:
        int64_t m_interval;
        uint16_t m_revision;
        bool m_enabled;
        bool m_repeated;
        TimerClosure* m_callback;
    public:
        TimerEntry() : m_interval(0),
                       m_revision(0),
                       m_enabled(false),
                       m_repeated(false),
                       m_callback(NULL)
        {}
    };

    typedef std::map<int64_t, TimerEntry> TimerMap;

public:
    TimerManager() : m_stop(false), m_running_timer_id(0)
    {
        this->Start();
    }

    virtual ~TimerManager() {}

    // One shot timer
    bool AddOneShotTimer(int64_t interval, TimerClosure* timer_closure);

    // Repeated timer that will execute callback periodically
    bool AddRepeatedTimer(int64_t interval, TimerClosure* timer_closure);

    // Shall call it before desctructing
    void Stop();

private:
    virtual void Entry();

    void SendStop();

    void ScheduleTimeout(int64_t timer_id, const TimerEntry& entry);

    bool GetLatestIntervalNoLock(int64_t* latest) const;

    bool GetLatestTimeNoLock(int64_t* latest_time) const;

    int64_t GetTimerId() const;

    void PushTimeoutNoLock(int64_t timer_id, int64_t definite_time);

    bool AddTimerInternal(int64_t interval,
                          bool repeated,
                          TimerClosure* timer_closure,
                          int64_t timer_id = -1);

private:
    std::priority_queue<Timeout,
                        std::deque<Timeout>,
                        std::greater<Timeout> > m_timeout_queue;

    TimerMap m_timer_map;

    Mutex m_mutex;
    ConditionVariable m_timeout_cond;

    Atomic<bool> m_stop;
    Atomic<int64_t> m_running_timer_id;
};

}

#endif // IPCS_COMMON_TIMER_TIMER_MANAGER_H
