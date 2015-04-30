// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 03/01/13
// Description: posix time

#ifndef IPCS_COMMON_SYSTEM_TIME_POSIX_TIME_H
#define IPCS_COMMON_SYSTEM_TIME_POSIX_TIME_H
#pragma once

#include <sys/time.h>
#include <stdint.h>

namespace ipcs_common
{

bool RelativeTimeToAbsTimeSpec(int64_t time_in_ms, struct timespec* ts);

int64_t GetCurrentTime();

int64_t GetTimeStampInMs();

int64_t GetTimeStampInUs();

} // namespace ipcs_common

#endif // IPCS_COMMON_SYSTEM_TIME_POSIX_TIME_H
