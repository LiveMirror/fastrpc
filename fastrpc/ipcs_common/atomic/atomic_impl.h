// Copyright (c) 2013, Baidu.com Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/14/13
// Description: atomic implementation

#ifndef IPCS_COMMON_ATOMIC_ATOMIC_IMPL_H
#define IPCS_COMMON_ATOMIC_ATOMIC_IMPL_H
#pragma once

#include "atomic_gcc.h"

namespace ipcs_common
{

template <class P>
class AtomicCompareAndExchangeDetail
{
public:
    template <class T>
    static void Exchange(volatile T* target, T value, T* old)
    {
        *old = *target;
        for (;!P::CompareAndExchange(target, *old, value, old);) {}
    }

    template <class T>
    static T ExchangeAdd(volatile T* target, T value)
    {
        T old = *target;
        for (;!P::CompareAndExchange(target, old, static_cast<T>(old + value), &old);) {}
        return old;
    }

    template <class T>
    static void Set(volatile T* target, T value)
    {
        T old = *target;
        Exchange(target, value, &old);
    }

    template <class T>
    static T ExchangeSub(volatile T* target, T value)
    {
        return ExchangeAdd(target, static_cast<T>(-value));
    }

    template <class T>
    static T Add(volatile T* target, T value)
    {
        return ExchangeAdd(target, value) + value;
    }

    template <class T>
    static T Sub(volatile T* target, T value)
    {
        return ExchangeSub(target, value) - value;
    }

    template <class T>
    static T Increment(volatile T* target)
    {
        return Add(target, static_cast<T>(1));
    }

    template <class T>
    static T Decrement(volatile T* target)
    {
        return Sub(target, static_cast<T>(1));
    }

    template <class T>
    static T ExchangeAnd(volatile T* target, T value)
    {
        T old = *target;
        for (;!P::CompareAndExchange(target, old, static_cast<T>(old & value), &old);) {}
        return old;
    }

    template <class T>
    static T And(volatile T* target, T value)
    {
        return ExchangeAnd(target, value) & value;
    }

    template <class T>
    static T ExchangeOr(volatile T* target, T value)
    {
        T old = *target;
        for (;!P::CompareAndExchange(target, old, static_cast<T>(old | value), &old);) {}
        return old;
    }

    template <class T>
    static T Or(volatile T* target, T value)
    {
        return ExchangeOr(target, value) | value;
    }

    template <class T>
    static T ExchangeXor(volatile T* target,  T value)
    {
        T old = *target;
        for (;!P::CompareAndExchange(target, old, static_cast<T>(old ^ value), &old);) {}
        return old;
    }

    template <class T>
    static T Xor(volatile T* target, T value)
    {
        return ExchangeXor(target, value) ^ value;
    }
};

class AtomicImplementation
{
public:
    template <class T>
    static T Get(volatile T* target)
    {
        return *target;
    }

    template <class T>
    static void Set(volatile T* target, T value)
    {
        AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(T)> >::Set(target, value);
    }

    template <class T>
    static void Exchange(volatile T* target, T value, T* old)
    {
        AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(T)> >::Exchange(target, value, old);
    }

    template <class T>
    static T ExchangeAdd(volatile T* target, T value)
    {
        return AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(T)> >::ExchangeAdd(target, value);
    }

    template <class T>
    static T ExchangeSub(volatile T* target, T value)
    {
        return AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(T)> >::ExchangeSub(target, value);
    }

    static unsigned char ExchangeSub(volatile unsigned char* target,
                                     unsigned char value)
    {
        return static_cast<unsigned char>(
                   ExchangeSub(reinterpret_cast<volatile signed char*>(target),
                               static_cast<signed char>(value)));
    }

    static unsigned short ExchangeSub(volatile unsigned short* target,
                                      unsigned short value)
    {
        return static_cast<unsigned short>(
                   ExchangeSub(reinterpret_cast<volatile signed short*>(target),
                               static_cast<signed short>(value)));
    }

    static unsigned int ExchangeSub(volatile unsigned int* target,
                                    unsigned int value)
    {
        return static_cast<unsigned int>(
                   ExchangeSub(reinterpret_cast<volatile signed int*>(target),
                               static_cast<signed int>(value)));
    }

    static unsigned long ExchangeSub(volatile unsigned long* target,
                                     unsigned long value)
    {
        return static_cast<unsigned long>(
                   ExchangeSub(reinterpret_cast<volatile signed long*>(target),
                               static_cast<signed long>(value)));
    }

    static unsigned long long ExchangeSub(volatile unsigned long long* target,
                                          unsigned long long value)
    {
        return static_cast<unsigned long long>(
                   ExchangeSub(reinterpret_cast<volatile signed long long*>(target),
                               static_cast<signed long long>(value)));
    }

    static double ExchangeAdd(volatile double* target,
                              double value)
    {
        return AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(double)> >::ExchangeAdd(target, value);
    }

    static float ExchangeAdd(volatile float* target,
                             float value)
    {
        return AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(float)> >::ExchangeAdd(target, value);
    }

    template <class T>
    static T Add(volatile T* target, T value)
    {
        return ExchangeAdd(target, value) + value;
    }

    template <class T>
    static T Sub(volatile T* target, T value)
    {
        return ExchangeSub(target, value) - value;
    }

    template <class T>
    static T Increment(volatile T* target)
    {
        return Add(target, static_cast<T>(1));
    }

    template <class T>
    static T Decrement(volatile T* target)
    {
        return Sub(target, static_cast<T>(1));
    }

    template <class T>
    static T ExchangeAnd(volatile T* target, T value)
    {
        return AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(T)> >::ExchangeAnd(target, value);
    }

    template <class T>
    static T And(volatile T* target, T value)
    {
        return ExchangeAnd(target, value) & value;
    }

    template <class T>
    static T ExchangeOr(volatile T* target, T value)
    {
        return AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(T)> >::ExchangeOr(target, value);
    }

    template <class T>
    static T Or(volatile T* target, T value)
    {
        return ExchangeOr(target, value) | value;
    }

    template <class T>
    static T ExchangeXor(volatile T* target, T value)
    {
        return AtomicCompareAndExchangeDetail<AtomicAsm<sizeof(T)> >::ExchangeXor(target, value);
    }

    template <class T>
    static T Xor(volatile T* target, T value)
    {
        return ExchangeXor(target, value) ^ value;
    }

    template <class T>
    static bool CompareAndExchange(volatile T* target, T value, T exchange, T* old)
    {
        return AtomicAsm<sizeof(T)>::CompareAndExchange(target, value, exchange, old);
    }
};

} // namespace ipcs_common

#endif // IPCS_COMMON_ATOMIC_ATOMIC_IMPL_H
