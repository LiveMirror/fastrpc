// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 03/30/13
// Description: timer manager impl

#include "timer_manager.h"
#include "posix_time.h"
#include "this_thread.h"

namespace ipcs_common
{

void TimerManager::Stop()
{
    SendStop();
    Join();

    while (!m_timeout_queue.empty()) {
        m_timeout_queue.pop();
    }

    TimerMap::iterator it;
    TimerEntry entry;
    for (it = m_timer_map.begin(); it != m_timer_map.end();) {
        entry = it->second;
        if (entry.m_callback->IsSelfDelete()) {
            delete entry.m_callback;
            entry.m_callback = NULL;
        }
        m_timer_map.erase(it++);
    }

}

void TimerManager::SendStop()
{
    m_stop = true;
    m_timeout_cond.Broadcast();
}

void TimerManager::Entry()
{
    TimerEntry entry;

    for (;;) {
        {
            if (m_stop) {
                return;
            }

            MutexLocker lock(m_mutex);
            int64_t latest_interval = 0;
            if (GetLatestIntervalNoLock(&latest_interval)) {
                if (latest_interval >= 0) {
                    bool timed = m_timeout_cond.WaitTime(&m_mutex, latest_interval);
                    if (timed) {
                        continue;
                    }
                }
                // if latest_interval < 0 , to pop it out as soon as possible
            } else {
                m_timeout_cond.Wait(&m_mutex);
                continue;
            }

            // Get the entry and run it.
            if (m_timeout_queue.empty()) continue;
            Timeout to = m_timeout_queue.top();
            m_timeout_queue.pop();

            TimerMap::iterator it = m_timer_map.find(to.m_timer_id);
            if (it != m_timer_map.end()) {
                entry = it->second;
                m_running_timer_id = to.m_timer_id;
                m_timer_map.erase(it);
            } else {
                continue;
            }

        }
        ScheduleTimeout(m_running_timer_id, entry);
    }
}

void TimerManager::ScheduleTimeout(int64_t timer_id, const TimerEntry& entry)
{
    // run the timeout callback, no lock should be used here
    // it cost time here!
    entry.m_callback->Run(timer_id);

    // add to queue if repeated, lock used internal
    if (entry.m_repeated) {
        assert(AddTimerInternal(entry.m_interval,
                                true,
                                entry.m_callback,
                                timer_id));
    }
    m_running_timer_id = 0;
}

bool TimerManager::GetLatestIntervalNoLock(int64_t* latest) const
{
    int64_t latest_timeout = 0;
    if (GetLatestTimeNoLock(&latest_timeout)) {
        *latest = latest_timeout - GetCurrentTime();
        return true;
    }
    return false;
}

bool TimerManager::GetLatestTimeNoLock(int64_t* latest_time) const
{
    if (!m_timeout_queue.empty()) {
        const Timeout& to = m_timeout_queue.top();
        *latest_time = to.m_timeout;
        return true;
    }
    return false;
}

void TimerManager::PushTimeoutNoLock(int64_t timer_id, int64_t definite_time)
{
    Timeout to = {timer_id, definite_time};
    m_timeout_queue.push(to);
}

int64_t TimerManager::GetTimerId() const
{
    static int64_t timer_id = 0;
    return AtomicIncrement(&timer_id);
}

bool TimerManager::AddTimerInternal(int64_t interval,
                                    bool repeated,
                                    TimerClosure* timer_closure,
                                    int64_t timer_id)
{
    assert(interval >= 0);
    if (repeated) {
        assert(!timer_closure->IsSelfDelete());
    } else {
        assert(timer_closure->IsSelfDelete());
    }
    assert(timer_closure != NULL);

    TimerEntry entry;
    entry.m_interval = interval;
    entry.m_repeated = repeated;
    entry.m_callback = timer_closure;
    entry.m_enabled = true;

    int64_t id = timer_id == -1 ? GetTimerId() : timer_id;

    MutexLocker locker(m_mutex);

    bool inserted = m_timer_map.insert(std::make_pair(id, entry)).second;
    if (!inserted) {
        return false;
    }

    int64_t current_time = GetCurrentTime();
    int64_t definite_timeout = entry.m_interval + current_time;

    int64_t latest_time = 0;

    if (ThisThread::GetId() == this->GetId()) {
        // the same thread, no need to signal
        PushTimeoutNoLock(id, definite_timeout);
    } else {
        if (GetLatestTimeNoLock(&latest_time)) {
            PushTimeoutNoLock(id, definite_timeout);
            if (definite_timeout < latest_time) {
                m_timeout_cond.Signal();
            }
        } else {
            PushTimeoutNoLock(id, definite_timeout);
            m_timeout_cond.Signal();
        }
    }

    return true;
}

bool TimerManager::AddOneShotTimer(int64_t interval, TimerClosure* timer_closure)
{
    return AddTimerInternal(interval, false, timer_closure);
}

bool TimerManager::AddRepeatedTimer(int64_t interval, TimerClosure* timer_closure)
{
    return AddTimerInternal(interval, true, timer_closure);
}

}
