// Copyright (c) 2013, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/20/13
// Description: check posix error wrapper

#ifndef IPCS_COMMON_SYSTEM_CHECK_ERROR_H
#define IPCS_COMMON_SYSTEM_CHECK_ERROR_H
#pragma once

namespace  ipcs_common
{
#ifdef __unix__

void HandleErrnoError(const char* func, int err);
inline void CheckErrnoError(const char* func, int err)
{
    if (err) {
        HandleErrnoError(func, err);
    }
}

void HandlePosixError(const char* func, int ret);
inline void CheckPosixError(const char* func, int ret)
{
    if (ret < 0) {
        HandlePosixError(func, ret);
    }
}

bool HandlePthreadTryLockError(const char* func, int ret);
inline bool CheckPthreadTryLockError(const char* func, int ret)
{
    if (ret) {
        return HandlePthreadTryLockError(func, ret);
    }
    return true;
}

bool HandlePthreadTimedoutError(const char* func, int ret);
inline bool CheckPthreadTimedoutError(const char* func, int ret)
{
    if (ret) {
        return HandlePthreadTimedoutError(func, ret);
    }
    return true;
}

#endif
} // namespace ipcs_common

#define CHECK_ERRNO_ERROR(expr) \
    ipcs_common::CheckErrnoError(__PRETTY_FUNCTION__, (expr))

#define CHECK_PTHREAD_ERROR(expr) \
    ipcs_common::CheckErrnoError(__PRETTY_FUNCTION__, (expr))

#define CHECK_PTHREAD_TRYLOCK_ERROR(expr) \
    ipcs_common::CheckPthreadTryLockError(__PRETTY_FUNCTION__, (expr))

#define CHECK_PTHREAD_TIMEDOUT_ERROR(expr) \
    ipcs_common::CheckPthreadTimedoutError(__PRETTY_FUNCTION__, (expr))

#endif // IPCS_COMMON_SYSTEM_CHECK_ERROR_H
