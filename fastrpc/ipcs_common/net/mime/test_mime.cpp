// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "mime.h"
#include "gtest.h"

using namespace net;

TEST(MimeType, FromFileExtension)
{
    MimeType mt = MimeType::FromFileExtension("xml");
    EXPECT_EQ("text/xml", mt.ToString());
}


TEST(MimeType, Set)
{
    MimeType mt;
    EXPECT_TRUE(mt.Set("text/xml"));
    EXPECT_EQ("text/xml", mt.ToString());
    EXPECT_FALSE(mt.Set("invalid"));
}

TEST(MimeType, Match)
{
    MimeType mt;
    mt.Set("text/xml");
    EXPECT_TRUE(mt.Match("text/xml"));
    EXPECT_TRUE(mt.Match("*/xml"));
    EXPECT_TRUE(mt.Match("text/*"));
    EXPECT_FALSE(mt.Match("textxml"));
}
