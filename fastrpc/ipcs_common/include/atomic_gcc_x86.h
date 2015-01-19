// Copyright (c) 2013, Baidu.com Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 02/14/13
// Description: atomic x86 impl

#ifndef IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X86_H
#define IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X86_H
#pragma once

// If PIC is defined then ebx is used as the `thunk' reg
// However cmpxchg8b requires ebx
// In this case we backup the value into esi before cmpxchg8b
// and then restore ebx from esi after cmpxchg8b
#undef ATOMIC_BREG
#undef XCHG_ATOMIC_BREG
#if defined(__PIC__) && !defined(__CYGWIN__)
#define ATOMIC_BREG "S"
#define ATOMIC_BACKUP_BREG "xchgl %%ebx,%%esi\n\t"
#define ATOMIC_RESTORE_BREG ATOMIC_BACKUP_BREG
#else
#define ATOMIC_BREG "b"
#define ATOMIC_BACKUP_BREG ""
#define ATOMIC_RESTORE_BREG ""
#endif

#include "atomic_gcc_x86x64_shared.h"

namespace ipcs_common
{

template <>
class AtomicAsm<8> :
{
public:
    union LDD
    {
        unsigned long long ll_val;
        double d_val;
    };

public:
    template <class T>
    static bool CompareAndExchange(volatile T* target, T compare, T exchange, T* old)
    {
        bool result = false;
        __asm__ __volatile__(
            ATOMIC_BACKUP_BREG
            "lock; cmpxchg8b (%%edi)\n\t"
            ATOMIC_RESTORE_BREG
            "setz %1\n"
            : "=A"(*old), "=q"(result)
            : ATOMIC_BREG((unsigned)exchange),
            "c"((unsigned)((unsigned long long)exchange >> 32)),
            "D"(target),
            "0"(compare)
            : "memory");
        return result;
    }

    static bool CompareAndExchange(
        volatile double* target,
        double compare,
        double exchange,
        double* old)
    {
        bool result = false;
        LDD exchange_ll;
        exchange_ll.d_val = exchange;
        __asm__ __volatile__(
            ATOMIC_BACKUP_BREG
            "lock; cmpxchg8b (%%edi)\n\t"
            ATOMIC_RESTORE_BREG
            "setz %1\n"
            : "=A"(*old), "=q"(result)
            : ATOMIC_BREG((unsigned)exchange_ll.ll_val),
            "c"((unsigned)(exchange_ll.ll_val >> 32)),
            "D"(target),
            "0"(compare)
            : "memory");
        return result;
    }
};

} // namespace ipcs_common

#undef ATOMIC_BREG
#undef ATOMIC_BACKUP_BREG
#undef ATOMIC_RESTORE_BREG

#endif // IPCS_COMMON_ATOMIC_DETAIL_ATOMIC_GCC_X86_H
