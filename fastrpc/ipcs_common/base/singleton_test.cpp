// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: singleton class template

#include "singleton.h"
#include "gtest.h"

// make a singleton class
class TestClass : public SingletonBase<TestClass>
{
    friend class SingletonBase<TestClass>;
private:
    TestClass() {}
public:
    int Test() const
    {
        return 1;
    }
};

TEST(Singleton, SingletonClass)
{
    TestClass& test = TestClass::Instance();
    EXPECT_EQ(1, test.Test());
}

class TestClass2 {};
TEST(Singleton, ClassSingleton)
{
    TestClass2& test = Singleton<TestClass2>::Instance();
    (void) test;
}

class TestClass3 : public SingletonBase<TestClass3>
{
    friend class SingletonBase<TestClass>;
};

TEST(Singleton, IsAlive)
{
    EXPECT_FALSE(TestClass3::IsAlive());
    TestClass3::Instance();
    EXPECT_TRUE(TestClass3::IsAlive());
}
