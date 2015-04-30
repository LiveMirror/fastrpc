// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/21/13
// Description: thread types -- mainly thread attr

#ifndef IPCS_COMMON_THREAD_THREAD_TYPES_H
#define IPCS_COMMON_THREAD_THREAD_TYPES_H
#pragma once

#include <pthread.h>
#include <string>
#include "check_error.h"

namespace ipcs_common
{

class ThreadAttribute
{
public:
    ThreadAttribute() : m_name(""),
                        m_stack_size(0),
                        m_detach_state(PTHREAD_CREATE_JOINABLE)
    {
        CHECK_PTHREAD_ERROR(pthread_attr_init(&m_attr));
    }

    virtual ~ThreadAttribute()
    {
        CHECK_PTHREAD_ERROR(pthread_attr_destroy(&m_attr));
    }

    ThreadAttribute& SetDetachState(int detach_state = PTHREAD_CREATE_JOINABLE);

    ThreadAttribute& SetStackSize(size_t stack_size = 64*1024);

private:
    std::string m_name;
    pthread_attr_t m_attr;
    size_t m_stack_size;
    int m_detach_state;
};

} // namespace ipcs_common


#endif // IPCS_COMMON_THREAD_THREAD_TYPES_H
