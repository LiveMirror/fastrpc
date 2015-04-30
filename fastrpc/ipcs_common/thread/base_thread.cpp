// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/22/13
// Description: base thread

#include "base_thread.h"
#include <assert.h>
#include <errno.h>
#include "atomic.h"
#include "check_error.h"
#include "this_thread.h"

namespace ipcs_common
{

pthread_key_t BaseThread::s_thread_handle_key;
pthread_once_t BaseThread::s_init_once = PTHREAD_ONCE_INIT;
size_t BaseThread::s_default_stack_size = 64*1024;

void BaseThread::Init()
{
    CHECK_PTHREAD_ERROR(pthread_once(&s_init_once, BaseThread::DoInit));
}

void BaseThread::DoInit()
{
    CHECK_PTHREAD_ERROR(pthread_key_create(&s_thread_handle_key, NULL));
}

bool BaseThread::Start()
{
    // It is impossible that it is joinable
    if (IsJoinable()) {
        CHECK_PTHREAD_ERROR(EINVAL);
    }
    m_id = 0;

    pthread_attr_t attr;
    CHECK_PTHREAD_ERROR(pthread_attr_init(&attr));
    if (m_stack_size > 0) {
        CHECK_PTHREAD_ERROR(pthread_attr_setstacksize(&attr, m_stack_size));
    }

    int err = pthread_create(&m_handle, &attr, StaticEntry, this);
    if (err != 0) {
        if (err != EAGAIN) {
            CHECK_PTHREAD_ERROR(err);
        }
        m_id = -1;
    }

    CHECK_PTHREAD_ERROR(pthread_attr_destroy(&attr));
    return err == 0;
}

void* BaseThread::StaticEntry(void* thread_arg)
{
    BaseThread* this_thread = static_cast<BaseThread*>(thread_arg);
    CHECK_PTHREAD_ERROR(pthread_setspecific(s_thread_handle_key, thread_arg));
    this_thread->m_alive = true;
    this_thread->m_id = ThisThread::GetId();
    pthread_cleanup_push(CleanUp, thread_arg);
    this_thread->Entry();
    pthread_cleanup_pop(1);
    return 0;
}

void BaseThread::OnExit()
{
    m_alive = false;
}

void BaseThread::CleanUp(void* arg)
{
    BaseThread* this_thread = static_cast<BaseThread*>(arg);
    this_thread->OnExit();
}

bool BaseThread::IsJoinable() const
{
    return !pthread_equal(m_handle, pthread_t());
}

void BaseThread::Join()
{
    assert(IsJoinable());
    CHECK_PTHREAD_ERROR(pthread_join(m_handle, 0));
    m_handle = pthread_t();
    m_id = -1;
}

bool BaseThread::IsAlive() const
{
    return m_alive;
}

pid_t BaseThread::GetId() const
{
    if (m_id != 0) {
        return m_id;
    }
    while (AtomicGet(&m_id) == 0) {
        ThisThread::Sleep(1);
    }
    return m_id;
}

void BaseThread::SetStackSize(size_t stack_size)
{
    m_stack_size = stack_size;
}

bool BaseThread::StopAndWaitForExit()
{
    if (IsJoinable()) {
        m_stop_requested = true;
        Join();
        return true;
    }
    return false;
}

}
