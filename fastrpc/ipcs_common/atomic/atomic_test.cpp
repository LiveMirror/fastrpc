// Copyright (c) 2013, Baidu.com Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/14/13
// Description: atomic_test

#include "atomic.h"
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <iostream>
#include "gtest.h"

template <class T>
class TypeTester
{
public:
    static void Run()
    {
        ipcs_common::Atomic<T> n;

        n = 0;
        EXPECT_EQ(n, static_cast<T>(0));

        n++;
        EXPECT_EQ(n, static_cast<T>(1));

        n--;
        EXPECT_EQ(n, static_cast<T>(0));

        ++n;
        EXPECT_EQ(n, static_cast<T>(1));

        --n;
        EXPECT_EQ(n, static_cast<T>(0));

        n+=64;
        EXPECT_EQ(n, static_cast<T>(64));

        n-=32;
        EXPECT_EQ(n, static_cast<T>(32));
    }
};

template <>
class TypeTester<bool>
{
public:
    static void Run()
    {
        ipcs_common::Atomic<bool> n = false;

        EXPECT_FALSE(n);

        n = true;
        EXPECT_TRUE(n);

        bool v = n;
        EXPECT_TRUE(v);

        bool old;
        n.CompareAndExchange(true, false, &old);
        EXPECT_FALSE(n);
        EXPECT_TRUE(old);

        v = n;
        EXPECT_FALSE(v);

        n.CompareAndExchange(true, false);
        EXPECT_FALSE(n);
        v = n;
        EXPECT_FALSE(v);
    }
};

template <>
class TypeTester<double>
{
public:
    static void Run()
    {
        ipcs_common::Atomic<double> n;

        n = 1.0;
        EXPECT_DOUBLE_EQ(1, n);

        n++;
        EXPECT_DOUBLE_EQ(2, n);

        n--;
        EXPECT_DOUBLE_EQ(1, n);

        ++n;
        EXPECT_DOUBLE_EQ(2, n);

        --n;
        EXPECT_DOUBLE_EQ(1, n);

        double old;
        n.CompareAndExchange(1, 3, &old);
        EXPECT_DOUBLE_EQ(1, old);
        EXPECT_DOUBLE_EQ(3, n);

        n.CompareAndExchange(2, 5, &old);
        EXPECT_DOUBLE_EQ(3, old);
        EXPECT_DOUBLE_EQ(3, n);
    }
};

template <>
class TypeTester<float>
{
public:
    static void Run()
    {
        ipcs_common::Atomic<float> n;

        n = 1.0f;
        EXPECT_DOUBLE_EQ(1, n);

        n++;
        EXPECT_DOUBLE_EQ(2, n);

        n--;
        EXPECT_DOUBLE_EQ(1, n);

        ++n;
        EXPECT_DOUBLE_EQ(2, n);

        --n;
        EXPECT_DOUBLE_EQ(1, n);

        float old;
        n.CompareAndExchange(1, 3, &old);
        EXPECT_DOUBLE_EQ(1, old);
        EXPECT_DOUBLE_EQ(3, n);

        n.CompareAndExchange(2, 5, &old);
        EXPECT_DOUBLE_EQ(3, old);
        EXPECT_DOUBLE_EQ(3, n);
    }
};

inline long long GetTimeStamp()
{
    timeval tv;
    gettimeofday(&tv, NULL);

    return static_cast<long long>(tv.tv_sec*1000LL + tv.tv_usec/1000);
}

TEST(Atomics, BasicTypes)
{
    TypeTester<int>::Run();
    TypeTester<unsigned int>::Run();

    TypeTester<char>::Run();
    TypeTester<unsigned char>::Run();

    TypeTester<int64_t>::Run();
    TypeTester<uint64_t>::Run();

    TypeTester<long long>::Run();
    TypeTester<unsigned long long>::Run();

    TypeTester<bool>::Run();
    TypeTester<double>::Run();
    TypeTester<float>::Run();
}

TEST(Atomics, Adapter)
{
    int var = 0;
    int old = 0;

    using namespace ipcs_common;

    AtomicSet(&var, 1);
    EXPECT_EQ(1, AtomicGet(&var));

    AtomicExchange(&var, 2, &old);
    EXPECT_EQ(2, var);

    AtomicExchange(&var, 1, &old);
    EXPECT_EQ(2, old);
    EXPECT_EQ(1, var);

    EXPECT_EQ(1, AtomicExchangeAdd(&var, 2));
    EXPECT_EQ(3, var);

    EXPECT_EQ(3, AtomicExchangeSub(&var, 3));
    EXPECT_EQ(0, var);

    AtomicIncrement(&var);
    EXPECT_EQ(1, var);

    AtomicDecrement(&var);
    EXPECT_EQ(0, var);

    EXPECT_EQ(1, AtomicAdd(&var, 1));
    EXPECT_EQ(1, var);

    EXPECT_EQ(0, AtomicSub(&var, 1));
    EXPECT_EQ(0, var);

    AtomicAdd(&var, 2);
    EXPECT_EQ(2, var);

    EXPECT_EQ(2, AtomicExchangeAnd(&var, 1));
    EXPECT_EQ(0, var);

    AtomicAdd(&var, 2);
    EXPECT_EQ(2, AtomicAnd(&var, 3));
    EXPECT_EQ(2, var);

    EXPECT_EQ(2, AtomicExchangeOr(&var, 1));
    EXPECT_EQ(3, var);

    EXPECT_EQ(7, AtomicOr(&var, 4));

    AtomicSet(&var, 12);

    EXPECT_EQ(12, AtomicExchangeXor(&var, 3));
    EXPECT_EQ(15, var);

    EXPECT_EQ(3, AtomicXor(&var, 12));
    EXPECT_EQ(3, var);

    EXPECT_TRUE(AtomicCompareAndExchange(&var, 3, 12, &old));
    EXPECT_EQ(3, old);
    EXPECT_EQ(12, var);
}

TEST(Atomics, Performance)
{
    long long start = 0LL;
    long long end = 0LL;
    unsigned int lock_cnt = 1000000;
    unsigned int var = 0;

    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);

    start = GetTimeStamp();
    for (unsigned int i = 0; i < lock_cnt; ++i) {
        pthread_mutex_lock(&lock);
        ++var;
        pthread_mutex_unlock(&lock);
    }
    end = GetTimeStamp();

    std::cout << "[INFO] pthread mutex cost : " << end - start << std::endl;

    pthread_mutex_destroy(&lock);

    var = 0;
    start = GetTimeStamp();
    for (unsigned int i = 0; i < lock_cnt; ++i) {
        ipcs_common::AtomicIncrement(&var);
    }
    end = GetTimeStamp();

    std::cout << "[INFO] ipcs_common::Atomic cost : " << end - start << std::endl;
}
