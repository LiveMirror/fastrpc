// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/20/13
// Description: test check error

#include "check_error.h"
#include "gtest.h"

TEST(CheckErr, BasicCheck)
{
    ASSERT_DEATH(CHECK_PTHREAD_ERROR(12), "memory");
    ASSERT_DEATH(CHECK_ERRNO_ERROR(13), "denied");
    ASSERT_DEATH(CHECK_ERRNO_ERROR(16), "busy");
}
