// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/21/13
// Description: thread types -- mainly thread attr

#include "thread_types.h"

namespace ipcs_common
{

ThreadAttribute& ThreadAttribute::SetDetachState(int detach_state)
{
    int detach_stat = detach_state;
    if (detach_state != PTHREAD_CREATE_JOINABLE &&
            detach_state != PTHREAD_CREATE_DETACHED) {
        detach_stat = PTHREAD_CREATE_JOINABLE;
    }
    CHECK_PTHREAD_ERROR(pthread_attr_setdetachstate(&m_attr, detach_stat));
    return *this;
}

ThreadAttribute& ThreadAttribute::SetStackSize(size_t stack_size)
{
    CHECK_PTHREAD_ERROR(pthread_attr_setstacksize(&m_attr, stack_size));
    return *this;
}

}


