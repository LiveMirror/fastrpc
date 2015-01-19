// Copyright (c) 2013, Baidu.com Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/14/13
// Description: atomic type

#ifndef IPCS_COMMON_ATOMIC_ATOMIC_TYPE_H
#define IPCS_COMMON_ATOMIC_ATOMIC_TYPE_H
#pragma once

#include "atomic_adapter.h"

namespace ipcs_common
{

template <class T>
class Atomic
{
public:
    Atomic() : m_value() {}

    // implicit allowed
    Atomic(T v) : m_value(v) {}

    Atomic(const Atomic& v)
    {
        AtomicSet(&m_value, v.Get());
    }

    ~Atomic() {}

    T Get() const
    {
        return AtomicGet(&m_value);
    }

    operator T() const
    {
        return AtomicGet(&m_value);
    }

    Atomic& operator=(Atomic v)
    {
        AtomicSet(&m_value, v.Get());
        return *this;
    }

    T operator++()
    {
        return AtomicIncrement(&m_value);
    }

    T operator++(int)
    {
        return AtomicExchangeAdd(&m_value, static_cast<T>(1));
    }

    T operator--()
    {
        return AtomicDecrement(&m_value);
    }

    T operator--(int)
    {
        return AtomicExchangeSub(&m_value, static_cast<T>(1));
    }

    T operator+=(T v)
    {
        return AtomicAdd(&m_value, v);
    }

    T operator-=(T v)
    {
        return AtomicSub(&m_value, v);
    }

    T operator&=(T v)
    {
        return AtomicAnd(&m_value, v);
    }

    T operator|=(T v)
    {
        return AtomicOr(&m_value, v);
    }

    T operator^=(T v)
    {
        return AtomicXor(&m_value, v);
    }

    bool CompareAndExchange(T value, T exchange, T* old)
    {
        return AtomicCompareAndExchange(&m_value, value, exchange, old);
    }

    bool CompareAndExchange(T value, T exchange)
    {
        return AtomicCompareAndExchange(&m_value, value, exchange);
    }

public:
    T Exchange(const T value) {
        return AtomicExchange(&m_value, value);
    }

private:
    volatile T m_value;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_ATOMIC_ATOMIC_TYPE_H
