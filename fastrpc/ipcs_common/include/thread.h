// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/25/13
// Description: thread impl, users could init
//              thread with closure and normal routine

#ifndef IPCS_COMMON_THREAD_THREAD_H
#define IPCS_COMMON_THREAD_THREAD_H
#pragma once

#include "closure.h"
#include "uncopyable.h"
#include "base_thread.h"
#include "function.h"

namespace ipcs_common
{

class Thread : private UnCopyable
{
public:
    typedef void (*ThreadStart)(void* context, unsigned long long param);

public:
    class ThreadImpl : public BaseThread
    {
    public:
        ThreadImpl() : m_routine(NULL),
                       m_context(NULL),
                       m_param(0),
                       m_closure(NULL),
                       m_function(NULL)
        {}

        ThreadImpl(ThreadStart routine,
                   void* context,
                   unsigned long long param) : m_routine(routine),
                                               m_context(context),
                                               m_param(param),
                                               m_closure(NULL),
                                               m_function(NULL)
        {}

        explicit ThreadImpl(Closure<void>* closure) : m_routine(NULL),
                                                      m_context(NULL),
                                                      m_param(0),
                                                      m_closure(closure),
                                                      m_function(NULL)
        {}

        explicit ThreadImpl(const Function<void ()>& function) : m_routine(NULL),
                                                                 m_context(NULL),
                                                                 m_param(0),
                                                                 m_closure(NULL),
                                                                 m_function(function)
        {
        }

        virtual ~ThreadImpl()
        {
            if (m_routine) {
                m_routine = NULL;
                m_context = NULL;
                m_param = 0;
            }

            if (m_closure) {
                m_closure = NULL;
            }

            if (m_function) {
                m_function = NULL;
            }
        }

        void InitThreadFunction(ThreadStart routine,
                                void* context,
                                unsigned long long param);
        void InitThreadFunction(Closure<void>* closure);
        void InitThreadFunction(const Function<void ()>& function);

        void CheckNotJoinable() const;

        bool IsInitialized() const;

    protected:
        // should be overridden
        virtual void Entry();
        virtual void OnExit();

    private:
        ThreadStart m_routine;
        void* m_context;
        unsigned long long m_param;

        Closure<void>* m_closure;
        Function<void ()> m_function;
    };

public:
    Thread() : m_impl(new ThreadImpl()) {}

    Thread(ThreadStart routine,
           void* context,
           unsigned long long param) : m_impl(new ThreadImpl(routine, context, param))
    {}

    explicit Thread(Closure<void>* closure) : m_impl(new ThreadImpl(closure)) {}
    explicit Thread(const Function<void ()>& function) : m_impl(new ThreadImpl(function)) {}

    virtual ~Thread()
    {
        if (IsJoinable()) {
            Join();
        }
        delete m_impl;
        m_impl = NULL;
    }

    void InitThreadFunction(ThreadStart routine,
                            void* context,
                            unsigned long long param);
    void InitThreadFunction(Closure<void>* closure);
    void InitThreadFunction(const Function<void ()>& function);

    bool Start();
    void Join();
    bool IsJoinable() const;
    bool IsAlive() const;
    void SetStackSize(size_t stack_size);

    pthread_t GetId() const;

protected:
    // The signature of Entry must be different from that of BaseThread.
    virtual void Entry() const {}

private:
    ThreadImpl* m_impl;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_THREAD_THREAD_H
