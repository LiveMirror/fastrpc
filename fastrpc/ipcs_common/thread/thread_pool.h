// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/27/13
// Description: thread pool

#ifndef IPCS_COMMON_THREAD_THREAD_POOL_H
#define IPCS_COMMON_THREAD_THREAD_POOL_H
#pragma once

#include <stdint.h>
#include <deque>
#include <list>
#include <string>
#include "atomic.h"
#include "closure.h"
#include "uncopyable.h"
#include "condition_variable.h"
#include "mutex.h"
#include "system_information.h"
#include "thread.h"

namespace ipcs_common
{

class ThreadPool : private UnCopyable
{
public:
    class Task
    {
    public:
        void Run()
        {
            if (m_closure) {
                m_closure->Run();
            }
        }

    public:
        Closure<void>* m_closure;
    };

public:
    ThreadPool(std::string name,
               int min_thread_num = 0,
               int max_thread_num = -1,
               int64_t idle_time = 3000,
               size_t stack_size = 0)       : m_name(name),
                                              m_min_thread_num(min_thread_num),
                                              m_max_thread_num(max_thread_num),
                                              m_idle_time(idle_time),
                                              m_stack_size(stack_size),
                                              m_thread_num(0),
                                              m_exit(false),
                                              m_terminating(false)
    {
        if (m_max_thread_num == -1) {
            m_max_thread_num = GetCpuNum() + m_min_thread_num + 2;
        }
    }

    ThreadPool(int min_thread_num = 0,
               int max_thread_num = -1,
               int64_t idle_time = 3000,
               size_t stack_size = 0)       : m_name("unnamed"),
                                              m_min_thread_num(min_thread_num),
                                              m_max_thread_num(max_thread_num),
                                              m_idle_time(idle_time),
                                              m_stack_size(stack_size),
                                              m_thread_num(0),
                                              m_exit(false),
                                              m_terminating(false)
    {
        if (m_max_thread_num == -1) {
            m_max_thread_num = GetCpuNum() + m_min_thread_num + 2;
        }
    }

    ~ThreadPool();

    void SetMinThreadNum(int min_num);
    void SetMaxThreadNum(int max_num);
    void SetStackSize(size_t stack_size);

    void AddTask(Closure<void>* closure);

    bool Terminate(bool wait_for_finished);

    void WorkerLoop(Thread* t);

    int GetWorkingThreadNum() const
    {
        return m_thread_num;
    }

    void WaitForIdle() {}

private:
    bool NeedWorker() const;

    void AddWorkerThread();

private:
    std::string m_name;

    Atomic<int> m_min_thread_num;
    Atomic<int> m_max_thread_num;
    int64_t m_idle_time;
    size_t m_stack_size;

    Atomic<int> m_thread_num;

    std::deque<Thread*> m_free_threads;
    std::deque<Thread*> m_working_threads;

    std::list<Task*> m_free_task_list;
    std::list<Task*> m_pending_task_list;

    Mutex m_mutex;
    ConditionVariable m_task_arrived_cond;
    ConditionVariable m_exit_cond;

    Atomic<bool> m_exit;
    Atomic<bool> m_terminating;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_THREAD_THREAD_POOL_H
