// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/21/13
// Description: this thread

#include "this_thread.h"
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

namespace ipcs_common
{

bool SupportTLS()
{
    char buf[64] = {0};
    confstr(_CS_GNU_LIBPTHREAD_VERSION, buf, sizeof(buf));
    return strstr(buf, "NPTL") != NULL;
}

pthread_t ThisThread::GetHandle()
{
    return pthread_self();
}

pid_t ThisThread::GetId()
{
    if (SupportTLS()) {
        static __thread pid_t this_tid = 0;
        if (this_tid == 0) {
            this_tid = syscall(SYS_gettid);
        }
        return this_tid;
    }
    return syscall(SYS_gettid);
}

bool ThisThread::IsMainThread()
{
    return ThisThread::GetId() == syscall(SYS_getpid);
}

void ThisThread::Sleep(int64_t time_in_ms)
{
    if (time_in_ms >= 0) {
        struct timespec ts = {time_in_ms / 1000, (time_in_ms % 1000) * 1000000L};
        nanosleep(&ts, &ts);
    }
    else {
        pause();
    }
}

void ThisThread::Yield()
{
    // pthread_yield() is not standard
    sched_yield();
}

void ThisThread::Exit()
{
    pthread_exit(NULL);
}

int ThisThread::GetLastErrorCode()
{
    return errno;
}

}

