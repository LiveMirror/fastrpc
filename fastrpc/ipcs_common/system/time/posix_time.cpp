// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 03/01/13
// Description: posix time

#include "posix_time.h"

namespace ipcs_common
{

bool RelativeTimeToAbsTimeSpec(int64_t time_in_ms, struct timespec* ts)
{
    if (time_in_ms < 0) {
        return false;
    }

    struct timeval tv;
    gettimeofday(&tv, 0);

    int64_t u_full = 1000000;
    int64_t usec = tv.tv_usec + time_in_ms * 1000;

    ts->tv_sec = tv.tv_sec + usec / u_full;
    ts->tv_nsec = (usec % u_full) * 1000;
    return true;
}

int64_t GetCurrentTime()
{
    return GetTimeStampInMs();
}

int64_t GetTimeStampInMs()
{
    return GetTimeStampInUs() / 1000;
}

int64_t GetTimeStampInUs()
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    return tv.tv_sec*1000000 + tv.tv_usec;
}

}
