// Copyright (c) 2013, feimat.com Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/14/13
// Description: atomic adapter

#ifndef IPCS_COMMON_ATOMIC_ATOMIC_ADAPTER_H
#define IPCS_COMMON_ATOMIC_ATOMIC_ADAPTER_H
#pragma once

#include "atomic_impl.h"

namespace ipcs_common
{

template <class T>
inline void AtomicSet(volatile T* target, T value)
{
    AtomicImplementation::Set(target, value);
}

template <class T>
inline T AtomicGet(volatile T* target)
{
    return AtomicImplementation::Get(target);
}

template <class T>
inline void AtomicExchange(volatile T* target, T value, T* old)
{
    AtomicImplementation::Exchange(target, value, old);
}

template <class T>
inline T AtomicExchange(volatile T* target, T value)
{
    T old;
    AtomicExchange(target, value, &old);
    return old;
}

template <class T>
inline T AtomicExchangeAdd(volatile T* target, T value)
{
    return AtomicImplementation::ExchangeAdd(target, value);
}

template <class T>
inline T AtomicExchangeSub(volatile T* target, T value)
{
    return AtomicImplementation::ExchangeSub(target, value);
}

template <class T>
inline T AtomicAdd(volatile T* target, T value)
{
    return AtomicImplementation::Add(target, value);
}

template <class T>
inline T AtomicSub(volatile T* target, T value)
{
    return AtomicImplementation::Sub(target, value);
}

template <class T>
inline T AtomicIncrement(volatile T* target)
{
    return AtomicImplementation::Increment(target);
}

template <class T>
inline T AtomicDecrement(volatile T* target)
{
    return AtomicImplementation::Decrement(target);
}

template <class T>
inline T AtomicExchangeAnd(volatile T* target, T value)
{
    return AtomicImplementation::ExchangeAnd(target, value);
}

template <class T>
inline T AtomicAnd(volatile T* target, T value)
{
    return AtomicImplementation::And(target, value);
}

template <class T>
inline T AtomicExchangeOr(volatile T* target, T value)
{
    return AtomicImplementation::ExchangeOr(target, value);
}

template <class T>
inline T AtomicOr(volatile T* target, T value)
{
    return AtomicImplementation::Or(target, value);
}

template <class T>
inline T AtomicExchangeXor(volatile T* target, T value)
{
    return AtomicImplementation::ExchangeXor(target, value);
}

template <class T>
inline T AtomicXor(volatile T* target, T value)
{
    return AtomicImplementation::Xor(target, value);
}

template <class T>
inline bool AtomicCompareAndExchange(volatile T* target, T value, T exchange, T* old)
{
    return AtomicImplementation::CompareAndExchange(target, value, exchange, old);
}

template <class T>
inline bool AtomicCompareAndExchange(volatile T* target, T value, T exchange)
{
    T old;
    return AtomicImplementation::CompareAndExchange(target, value, exchange, &old);
}

} // namespace ipcs_common

#endif // IPCS_COMMON_ATOMIC_ATOMIC_ADAPTER_H
