// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/22/13
// Description: base thread

#ifndef IPCS_COMMON_THREAD_BASE_THREAD_H
#define IPCS_COMMON_THREAD_BASE_THREAD_H
#pragma once

#include <pthread.h>

namespace ipcs_common
{

class BaseThread
{
public:
    BaseThread() : m_handle(pthread_t()),
                   m_id(-1),
                   m_alive(false),
                   m_stack_size(s_default_stack_size),
                   m_stop_requested(false)
    {
        Init();
    }

    virtual ~BaseThread()
    {
        if (IsJoinable()) {
            Join();
        }
    }

    bool Start();
    void Join();
    bool IsJoinable() const;
    bool IsAlive() const;
    void SetStackSize(size_t stack_size);

    pid_t GetId() const;

    void SendStopRequest() {
        m_stop_requested = true;
    }

    bool IsStopRequested() const {
        return m_stop_requested;
    }

    bool StopAndWaitForExit();

protected:
    virtual void Entry() = 0;
    virtual void OnExit();

private:
    static void Init();
    static void DoInit();
    static void* StaticEntry(void* thread_arg);
    static void CleanUp(void* arg);

private:
    static pthread_key_t s_thread_handle_key;
    static pthread_once_t s_init_once;
    static size_t s_default_stack_size;

    pthread_t m_handle; // pthread_t thread handle
    pid_t m_id;         // native thread id
    bool m_alive;
    size_t m_stack_size;

    volatile bool m_stop_requested;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_THREAD_BASE_THREAD_H
