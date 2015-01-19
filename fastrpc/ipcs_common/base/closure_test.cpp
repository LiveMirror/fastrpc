// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 01/28/13
// Description: closure test

#include "closure.h"
#include <iostream>
#include "gtest.h"

class ClosureTest
{
public:
    ClosureTest() {}
    virtual ~ClosureTest() {}

public:
    int Test_Bind0_Arg0()
    {
        std::cout << "-----> test bind0 arg0 for class method" << std::endl;
        return 1;
    }

    int Test_Bind1_Arg0(char ch)
    {
        std::cout << "-----> test bind1 arg0 for class method : " << ch << std::endl;
        return 2;
    }

    int Test_Bind0_Arg1(char ch)
    {
        std::cout << "-----> test bind0 arg1 for class method : " << ch << std::endl;
        return 3;
    }

    char Test_Bind3_Arg3(int a, char b, int c, char d, int e, char f)
    {
        std::cout << "-----> test bind3 arg3 for class method : " << a << b << c << d << e << f << std::endl;
        return 'c';
    }

    int Test_Bind7_Arg0(int a, char b, int c, char d, int e, char f, int g)
    {
        std::cout << "-----> test bind7 arg0 for class method : " << a << b << c << d << e << f << g << std::endl;
        return 70;
    }

    int Test_Bind0_Arg7(int a, char b, int c, char d, int e, char f, int g)
    {
        std::cout << "-----> test bind0 arg7 for class method : " << a << b << c << d << e << f << g << std::endl;
        return 7;
    }

    int Test_Bind2_Arg4(int a, char b, int c, char d, int e, char f)
    {
        std::cout << "-----> test bind2 arg4 for class method : " << a << b << c << d << e << f << std::endl;
        return 24;
    }

    void NullMethod()
    {

    }
};

int Func_Test_Bind3_Arg0(int a, int b, char c)
{
    std::cout << "-----> test bind3 arg0 for function: " << a << b << c << std::endl;
    return 8;
}

int Func_Test_Bind0_Arg7(int a, char b, int c, char d, int e, char f, int g)
{
    std::cout << "-----> test bind0 arg7 for function: " << a << b << c << d << e << f << g << std::endl;
    return 9;
}

int Func_Test_Bind0_Arg0()
{
    std::cout << "-----> test bind0 arg0 for function: " << std::endl;
    return 10;
}

int Func_Test_Bind7_Arg0(int a, char b, int c, char d, int e, char f, int g)
{
    std::cout << "-----> test bind7 arg0 for function: " << a << b << c << d << e << f << g << std::endl;
    return 11;
}

TEST(ClosureTestMethod, Method_test)
{
    ClosureTest ct;

    Closure<int> *cb = NewClosure(&ct, &ClosureTest::Test_Bind0_Arg0);

    ASSERT_TRUE(cb->IsSelfDelete());

    int r = cb->Run();

    ASSERT_EQ(r, 1);

    Closure<int> *cb2 = NewPermanentClosure(&ct, &ClosureTest::Test_Bind1_Arg0, 'a');

    ASSERT_FALSE(cb2->IsSelfDelete());

    r = cb2->Run();

    ASSERT_EQ(r, 2);

    delete cb2;

    Closure<int, char> *cb3 = NewClosure(&ct, &ClosureTest::Test_Bind0_Arg1);

    r = cb3->Run('b');

    ASSERT_EQ(r, 3);

    Closure<char, char, int, char> *cb4 = NewClosure(&ct, &ClosureTest::Test_Bind3_Arg3, 5, 'd', 6);

    char ret = cb4->Run('e', 7, 'f');

    ASSERT_EQ(ret, 'c');

    Closure<int> *cb5 = NewClosure(&ct, &ClosureTest::Test_Bind7_Arg0, 8, 'g', 9, 'h', 10, 'i', 11);

    r = cb5->Run();

    ASSERT_EQ(r, 70);

    Closure<int, int, char, int, char, int, char, int> *cb6 = NewClosure(&ct, &ClosureTest::Test_Bind0_Arg7);

    r = cb6->Run(12, 'j', 13, 'k', 14, 'l', 15);

    ASSERT_EQ(r, 7);

    Closure<int> *cb7 = NewClosure(Func_Test_Bind3_Arg0, 16, 17, 'm');

    r = cb7->Run();

    ASSERT_EQ(r, 8);

    Closure<int, int, char, int, char, int, char, int> *cb8 = NewClosure(Func_Test_Bind0_Arg7);

    r = cb8->Run(18, 'n', 19, 'o', 20, 'p', 21);

    ASSERT_EQ(r, 9);

    Closure<int> *cb9 = NewPermanentClosure(Func_Test_Bind0_Arg0);

    r = cb9->Run();

    delete cb9;

    ASSERT_EQ(r, 10);

    Closure<int> *cb10 = NewClosure(Func_Test_Bind7_Arg0, 22, 'q', 23, 'r', 24, 's', 25);

    r = cb10->Run();

    ASSERT_EQ(r, 11);

    Closure<int, int, char, int, char> *cb11 = NewClosure(&ct, &ClosureTest::Test_Bind2_Arg4, 26, 'o');

    r = cb11->Run(27, 'p', 28, 'q');

    ASSERT_EQ(r, 24);

    Closure<int, int, char, int, char> *cb12 = NewPermanentClosure(&ct, &ClosureTest::Test_Bind2_Arg4, 29, 's');

    r = cb12->Run(30, 't', 31, 'v');

    ASSERT_EQ(r, 24);

    delete cb12;
}

TEST(ClosureTestPerf, Performance)
{
    ClosureTest ct;

    for (int i = 0; i < 100000; ++i) {
        Closure<void> *c = NewClosure(&ct, &ClosureTest::NullMethod);

        c->Run();
    }
}

TEST(ClosureTestPerfPermanent, Performance)
{
    ClosureTest ct;

    for (int i = 0; i < 100000; ++i) {
        Closure<void> *c = NewPermanentClosure(&ct, &ClosureTest::NullMethod);

        c->Run();

        delete c;
    }
}
