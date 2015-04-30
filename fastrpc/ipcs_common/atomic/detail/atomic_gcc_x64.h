// Copyright (c) 2013, feimat.com Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/13/13
// Description: gcc x64 impl

#ifndef IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X64_H
#define IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X64_H
#pragma once

#include "atomic_gcc_x86x64_shared.h"

namespace ipcs_common
{

template <>
class AtomicAsm<8>
{
public:
    union LLD
    {
        unsigned long long ll_val;
        double d_val;
    };

public:
    template <class T>
    static void Exchange(volatile T* target, T value, T* old)
    {
        __asm__ __volatile__(
            "xchgq %0, %1"
            :"=r"(*old)
            :"m"(*target), "0"(value)
            :"memory"
            );
    }

    template <class T>
    static T ExchangeAdd(volatile T* target, T value)
    {
        __asm__ __volatile__(
            "lock; xaddq %0, %1"
            :"=r"(value)
            :"m"(*target), "0"(value)
            :"memory"
            );
        return value;
    }

    template <class T>
    static bool CompareAndExchange(volatile T* target, T value, T exchange, T* old)
    {
        bool result = false;
        __asm__ __volatile__(
            "lock; cmpxchgq %2, %3\n"
            "setz %1"
            :"=a"(*old), "=r"(result)
            :"q"(exchange), "m"(*target), "0"(value)
            :"memory"
            );
        return result;
    }

    static bool CompareAndExchange(volatile double* target,
                                   double value,
                                   double exchange,
                                   double* old)
    {
        bool result = false;
        LLD exchange_ll;
        exchange_ll.d_val = exchange;
        __asm__ __volatile__(
            "lock; cmpxchgq %2, %3\n"
            "setz %1"
            :"=a"(*old), "=r"(result)
            :"q"(exchange_ll.ll_val), "m"(*target), "0"(value)
            :"memory"
            );
        return result;
    }
};

} // namespace ipcs_common

#endif // IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X64_H
