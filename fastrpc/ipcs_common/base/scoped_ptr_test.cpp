// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/01/13
// Description: scoped_ptr test

#include "scoped_ptr.h"
#include "gtest.h"

class TestForScoped
{
public:
    TestForScoped() : m_value(5) {}

    int Return1()
    {
        return 1;
    }

public:
    int m_value;
};

TEST(ScopedPtrTest, ScopedPtr)
{
    using namespace ipcs_common;

    char *c =new char('c');

    scoped_ptr<char> pa(new char('a'));
    scoped_ptr<char> pb(new char('b'));
    scoped_ptr<char> pc(c);

    scoped_ptr<TestForScoped> ptest(new TestForScoped());

    ASSERT_NE(pa, pb);

    // NOT allowed,
    // ASSERT_NE(pa, ptest);

    ASSERT_EQ(*pa, 'a');
    ASSERT_EQ(*pb, 'b');

    pa.swap(pb);

    ASSERT_EQ(*pa, 'b');
    ASSERT_EQ(*pb, 'a');

    ASSERT_EQ(ptest->Return1(), 1);
    ASSERT_EQ(ptest->m_value, 5);

    pa.reset(c);
    ASSERT_EQ(*pa, 'c');
    ASSERT_EQ(pa, pc);
    ASSERT_EQ(pa, c);

    pa.release();
    ASSERT_EQ(pa.get(), static_cast<char*>(0));

    if (ptest) {
        EXPECT_TRUE(true);
    }

    if (!ptest) {
        ASSERT_TRUE(false);
    }
}


TEST(ScopedArrayTest, ScopedArray)
{
    using namespace ipcs_common;

    scoped_array<char> pa(new char[8]);
    scoped_array<char> pb(new char[4]);

    char *arr = new char[9];
    scoped_array<char> pc(arr);

    pa[0] = 'a';
    pa[6] = 'b';
    pa[7] = '\0';

    pb[0] = 'd';
    pb[1] = 'e';
    pb[2] = 'f';
    pb[3] = '\0';

    ASSERT_EQ(pa[0], 'a');
    ASSERT_EQ(pb[2], 'f');

    if (pa) {
        ASSERT_TRUE(true);
    }

    if (!pb) {
        ASSERT_TRUE(false);
    }

    pa.swap(pb);

    ASSERT_EQ(pa[1], 'e');
    ASSERT_EQ(pb[6], 'b');

    pc.release();

    ASSERT_EQ(pc.get(), static_cast<char*>(0));

    delete arr;
}

