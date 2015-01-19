// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: some useful annotation

#ifndef IPCS_COMMON_BASE_ANNOTATION_H
#define IPCS_COMMON_BASE_ANNOTATION_H
#pragma once

#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif
/// warn_unused_result of function
/// example:
/// WARN_UNUSED_RESULT int write();
/// if the return value of write is not used, compile will report error
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))

/// @fn IgnoreUnused
/// @brief ignore value explicitly
/// example:
/// IgnoreUnused(write(STDERR_FILENO, "hello", 5));

#ifdef __cplusplus
namespace common {
template <typename T> void IgnoreUnused(const T& value) {}
}

// make C and C++ share the same interface
using common::IgnoreUnused;
#else
static __inline void DoIgnoreUnused(int dummy, ...) {}
#define IgnoreUnused(e) DoIgnoreUnused(0, (e))
#endif // __cplusplus

#endif // IPCS_COMMON_BASE_ANNOTATION_H
