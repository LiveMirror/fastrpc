// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/25/13
// Description: thread impl

#include <errno.h>
#include "check_error.h"
#include "thread.h"

namespace ipcs_common
{

void Thread::ThreadImpl::Entry()
{
    if (m_function) {
        m_function();
    } else if (m_routine) {
        (*m_routine)(m_context, m_param);
    } else if (m_closure) {
        Closure<void>* closure = m_closure;
        if (m_closure->IsSelfDelete()) {
            m_closure = NULL;
        }
        closure->Run();
    }
}

void Thread::ThreadImpl::OnExit()
{
    BaseThread::OnExit();
}

void Thread::ThreadImpl::InitThreadFunction(ThreadStart routine,
                                            void* context,
                                            unsigned long long param)
{
    CheckNotJoinable();
    m_routine = routine;
    m_context = context;
    m_param = param;
    m_closure = NULL;
    m_function = NULL;
}

void Thread::ThreadImpl::InitThreadFunction(Closure<void>* closure)
{
    CheckNotJoinable();
    m_routine = NULL;
    m_context = NULL;
    m_param = 0;
    m_closure = closure;
    m_function = NULL;
}

void Thread::ThreadImpl::InitThreadFunction(const Function<void ()>& function) {
    CheckNotJoinable();
    m_routine = NULL;
    m_context = NULL;
    m_param = 0;
    m_closure = NULL;
    m_function = function;
}

void Thread::ThreadImpl::CheckNotJoinable() const
{
    if (IsJoinable()) {
        CHECK_PTHREAD_ERROR(EINVAL);
    }
}

bool Thread::ThreadImpl::IsInitialized() const
{
    return m_routine != NULL || m_closure != NULL || m_function != NULL;
}

void Thread::InitThreadFunction(ThreadStart routine,
                                void* context,
                                unsigned long long param)
{
    m_impl->InitThreadFunction(routine, context, param);
}

void Thread::InitThreadFunction(Closure<void>* closure)
{
    m_impl->InitThreadFunction(closure);
}

void Thread::InitThreadFunction(const Function<void ()>& function) {
    m_impl->InitThreadFunction(function);
}

bool Thread::Start()
{
    if (!m_impl->IsInitialized()) {
        CHECK_PTHREAD_ERROR(EINVAL);
        return false;
    }
    return m_impl->Start();
}

void Thread::Join()
{
    m_impl->Join();
}

bool Thread::IsJoinable() const
{
    return m_impl->IsJoinable();
}

bool Thread::IsAlive() const
{
    return m_impl->IsAlive();
}

void Thread::SetStackSize(size_t stack_size)
{
    m_impl->SetStackSize(stack_size);
}

pthread_t Thread::GetId() const
{
    return m_impl->GetId();
}

}

