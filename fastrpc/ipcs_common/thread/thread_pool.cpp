// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/27/13
// Description: thread pool impl

#include "thread_pool.h"
#include "this_thread.h"

namespace ipcs_common
{

bool ThreadPool::NeedWorker() const
{
    if (m_thread_num < m_min_thread_num) {
        return true;
    }
    if (m_thread_num >= m_max_thread_num) {
        return false;
    }

    return true;
}

void ThreadPool::AddWorkerThread()
{
    MutexLocker locker(m_mutex);
    Thread* t = NULL;
    if (!m_free_threads.empty()) {
        t = m_free_threads.back();
        m_free_threads.pop_back();
        t->Join();
    } else {
        t = new Thread();
    }
    m_working_threads.push_back(t);

    if (m_stack_size > 0) {
        t->SetStackSize(m_stack_size);
    }
    t->InitThreadFunction(NewClosure(this, &ThreadPool::WorkerLoop, t));
    t->Start();
    m_thread_num++;
}

void ThreadPool::AddTask(Closure<void>* closure)
{
    assert(!m_terminating);

    if (NeedWorker()) {
        AddWorkerThread();
    }

    Task* task = NULL;

    {
        MutexLocker locker(m_mutex);
        if (!m_free_task_list.empty()) {
            task = m_free_task_list.front();
            m_free_task_list.pop_front();
        } else {
            task = new Task();
        }
        task->m_closure = closure;
        m_pending_task_list.push_back(task);
    }

    m_task_arrived_cond.Signal();
}

void ThreadPool::WorkerLoop(Thread* t)
{
    while(!m_exit) {
        Task* task = NULL;
        {
            MutexLocker locker(&m_mutex);
            if (m_pending_task_list.empty()) {
                bool timedout = !m_task_arrived_cond.WaitTime(&m_mutex, m_idle_time);

                if (!m_pending_task_list.empty()) {
                    task = m_pending_task_list.front();
                    m_pending_task_list.pop_front();
                } else {
                    if (timedout && m_thread_num > m_min_thread_num) {
                        // exit this thread
                        break;
                    } else {
                        continue;
                    }
                }
            } else {
                task = m_pending_task_list.front();
                m_pending_task_list.pop_front();
            }
        }

        task->Run();

        {
            MutexLocker lock(m_mutex);
            m_free_task_list.push_back(task);
        }
    }

    MutexLocker locker(m_mutex);
    std::deque<Thread*>::iterator it;
    for (it = m_working_threads.begin(); it != m_working_threads.end(); ++it) {
        if (*it == t) {
            break;
        }
    }
    if (it != m_working_threads.end()) {
        m_working_threads.erase(it);
        m_free_threads.push_back(t);
    }
    m_thread_num--;
    m_exit_cond.Signal();
}

bool ThreadPool::Terminate(bool wait_for_finished)
{
    m_terminating = true;

    if (wait_for_finished) {
        while (!m_pending_task_list.empty()) {
            ThisThread::Sleep(100);
        }
    }

    m_exit = true;

    MutexLocker locker(m_mutex);
    while (m_thread_num > 0) {
        m_task_arrived_cond.Broadcast();
        m_exit_cond.Wait(&m_mutex);
    }

    return true;
}

ThreadPool::~ThreadPool()
{
    Task* task = NULL;
    Thread* thread = NULL;

    this->Terminate(false);

    while (!m_free_task_list.empty()) {
        task = m_free_task_list.front();
        m_free_task_list.pop_front();
        delete task;
        task = NULL;
    }

    while (!m_pending_task_list.empty()) {
        task = m_pending_task_list.front();
        m_pending_task_list.pop_front();
        if (task->m_closure->IsSelfDelete()) {
            delete task->m_closure;
        }
        delete task;
        task = NULL;
    }

    while (!m_free_threads.empty()) {
        thread = m_free_threads.front();
        m_free_threads.pop_front();
        thread->Join();
        delete thread;
        thread = NULL;
    }
}

}

