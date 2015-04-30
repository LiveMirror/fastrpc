// Copyright (c) 2013, feimat.com Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/13/13
// Description: This is detail impl for atomic,
//              please refer to atomic if you have c++0x enabled.

#ifndef IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X86X64_SHARED_H
#define IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X86X64_SHARED_H
#pragma once

#include <cstddef>

namespace ipcs_common
{
template <size_t TypeSize>
class AtomicAsm
{
};

template <>
class AtomicAsm<1>
{
public:
    template <class T>
    static void Exchange(volatile T* target, T value, T* old)
    {
        __asm__ __volatile__(
            "xchgb %0, %1"
            :"=r"(*old)
            :"m"(*target), "0"(value)
            :"memory"
            );
    }

    template <class T>
    static T ExchangeAdd(volatile T* target, T value)
    {
        __asm__ __volatile__(
            "lock; xaddb %0, %1"
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
            "lock; cmpxchgb %2, %3\n"
            "setz %1"
            :"=a"(*old), "=r"(result)
            :"q"(exchange), "m"(*target), "0"(value)
            :"memory"
            );
        return result;
    }
};

template <>
class AtomicAsm<2>
{
public:
    template <class T>
    static void Exchange(volatile T* target, T value, T* old)
    {
        __asm__ __volatile__(
            "xchgw %0, %1"
            :"=r"(*old)
            :"m"(*target), "0"(value)
            :"memory"
            );
    }

    template <class T>
    static T ExchangeAdd(volatile T* target, T value)
    {
        __asm__ __volatile__(
            "lock; xaddw %0, %1"
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
            "lock; cmpxchgw %2, %3\n"
            "setz %1"
            :"=a"(*old), "=r"(result)
            :"q"(exchange), "m"(*target), "0"(value)
            :"memory"
            );
        return result;
    }
};

template <>
class AtomicAsm<4>
{
public:
    template <class T>
    static void Exchange(volatile T* target, T value, T* old)
    {
        __asm__ __volatile__(
            "xchgl %0, %1"
            :"=r"(*old)
            :"m"(*target), "0"(value)
            :"memory"
            );
    }

    template <class T>
    static T ExchangeAdd(volatile T* target, T value)
    {
        __asm__ __volatile__(
            "lock; xaddl %0, %1"
            :"=r"(value)
            :"m"(*target), "0"(value)
            :"memory"
            );
        return value;
    }

    template <class T>
    static bool CompareAndExchange(volatile T* target, T value, T exchange, T* old)
    {
        bool result;
        __asm__ __volatile__(
            "lock; cmpxchgl %2, %3\n"
            "setz %1"
            :"=a"(*old), "=r"(result)
            :"q"(exchange), "m"(*target), "0"(value)
            :"memory"
            );
        return result;
    }
};

} // namespace ipcs_common

#endif // IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X86X64_SHARED_H
