// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: platform features

#ifndef IPCS_COMMON_BASE_PLATFORM_FEATURES_H
#define IPCS_COMMON_BASE_PLATFORM_FEATURES_H

#ifdef __GNUC__
/// support thread safe static variable initialization
#define HAS_THREAD_SAFE_STATICS 1

/// static const integral members need definition out of class
#define STATIC_CONST_MEMBER_NEED_DEFINATION 1

#endif

#if defined _MSC_VER
# define THREAD_LOCAL __declapsec(thread)
# define WEAK_SYMBOL __declapsec(selectany)
# define ALWAYS_INLINE __forceinline
#elif defined __GNUC__
# define THREAD_LOCAL __thread
# define WEAK_SYMBOL __attribute__((weak))
# define ALWAYS_INLINE inline __attribute__((always_inline))
#else
# error Unknown compiler
#endif

/// known alignment insensitive platforms
#if defined(__i386__) || \
    defined(__x86_64__) || \
    defined(_M_IX86) || \
    defined(_M_X64)
#define ALIGNMENT_INSENSITIVE_PLATFORM 1
#endif

/// define __attribute__ of gcc to null under non gcc
#ifndef __GNUC__
#define __attribute__(x)
#endif

// 用以标注一个参数不能为空
// void swap(int *p, int* q) __attribute__((nonnull(1, 2)));

// 变参格式的说明
// int log_printf(const char* format, ...) __attribute__((format(1, 2)));

#endif // IPCS_COMMON_BASE_PLATFORM_FEATURES_H
