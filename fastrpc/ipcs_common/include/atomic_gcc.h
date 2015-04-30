// Copyright (c) 2013, Baidu.com Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/14/13
// Description: decorator for detail

#ifndef IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_H
#define IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_H
#pragma once

#if __i386__
#include "atomic_gcc_x86.h"
#elif __x86_64__
#include "atomic_gcc_x64.h"
#else
#error unsupported arch
#endif

#endif // IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_H
