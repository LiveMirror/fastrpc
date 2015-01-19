// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: static assert

#include "static_assert.h"
#include "gtest.h"

TEST(StaticAssert, Test)
{
    STATIC_ASSERT(1 == 1);
    STATIC_ASSERT(1 == 1, "1 should be equal to 1");
}

TEST(StaticAssert, NoCompileTest)
{
#if 0 // uncomment to test
    STATIC_ASSERT(false);
    STATIC_ASSERT(1 == 2);
    STATIC_ASSERT(1 == 2, "1 == 2");
#endif
}
