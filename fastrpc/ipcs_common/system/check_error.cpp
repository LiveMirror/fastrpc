// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/20/13
// Description: check error impl

#include "check_error.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace ipcs_common
{

void HandleErrnoError(const char* func, int err)
{
    fprintf(stderr, "%s : fatal error occurred, %s", func, strerror(err));
    abort();
}

void HandlePosixError(const char* func, int ret)
{
    HandleErrnoError(func, ret);
}

bool HandlePthreadTryLockError(const char* func, int ret)
{
    if (ret == EBUSY || ret == EAGAIN) {
        return false;
    }
    else {
        HandleErrnoError(func, ret);
    }
    return false;
}

bool HandlePthreadTimedoutError(const char* func, int ret)
{
    if (ret == ETIMEDOUT) {
        return false;
    }
    else {
        HandleErrnoError(func, ret);
    }
    return false;
}

}


