// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 03/02/13
// Description: system information

#ifndef IPCS_COMMON_SYSTEM_SYSTEM_INFORMATION_H
#define IPCS_COMMON_SYSTEM_SYSTEM_INFORMATION_H
#pragma once

#include <unistd.h>

namespace ipcs_common
{

inline int GetCpuNum()
{
    return static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
}

} // namespace ipcs_common

#endif // IPCS_COMMON_SYSTEM_SYSTEM_INFORMATION_H
