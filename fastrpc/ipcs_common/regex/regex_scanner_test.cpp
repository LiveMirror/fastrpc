// Copyright (c) 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Greg J. Badros
//
// Unittest for scanner, especially GetNextComments and GetComments()
// functionality.

#include "regex_scanner.h"

#include <stdio.h>
#include <string>
#include <vector>

#include "regex.h"
#include "string_piece.h"
#include "gtest.h"
#include "config.h"

#define FLAGS_unittest_stack_size   49152

using std::vector;
// using namespace common;

TEST(RegexScanner, Scanner)
{
    const char input[] = "\n"
        "alpha = 1; // this sets alpha\n"
        "bravo = 2; // bravo is set here\n"
        "gamma = 33; /* and here is gamma */\n";

    const char *re = "(\\w+) = (\\d+);";

    RegexScaner s(input);
    std::string var;
    int number;
    s.SkipCxxComments();
    s.SetSaveComments(true);
    vector<StringPiece> comments;

    s.Consume(re, &var, &number);
    EXPECT_EQ("alpha", var);
    EXPECT_EQ(1, number);
    EXPECT_EQ(3, s.LineNumber());
    s.GetNextComments(&comments);
    EXPECT_EQ(1U, comments.size());
    EXPECT_EQ(" // this sets alpha\n", comments[0].as_string());
    comments.resize(0);

    s.Consume(re, &var, &number);
    EXPECT_EQ("bravo", var);
    EXPECT_EQ(2, number);
    s.GetNextComments(&comments);
    EXPECT_EQ(1U, comments.size());
    EXPECT_EQ(" // bravo is set here\n", comments[0].as_string());
    comments.resize(0);

    s.Consume(re, &var, &number);
    EXPECT_EQ("gamma", var);
    EXPECT_EQ(33, number);
    s.GetNextComments(&comments);
    EXPECT_EQ(1U, comments.size());
    EXPECT_EQ(" /* and here is gamma */\n", comments[0].as_string());
    comments.resize(0);

    s.GetComments(0, sizeof(input), &comments);
    EXPECT_EQ(3U, comments.size());
    EXPECT_EQ(" // this sets alpha\n", comments[0].as_string());
    EXPECT_EQ(" // bravo is set here\n", comments[1].as_string());
    EXPECT_EQ(" /* and here is gamma */\n", comments[2].as_string());
    comments.resize(0);

    s.GetComments(0, (int)(strchr(input, '/') - input), &comments);
    EXPECT_EQ(0U, comments.size());
    comments.resize(0);

    s.GetComments((int)(strchr(input, '/') - input - 1), sizeof(input),
                  &comments);
    EXPECT_EQ(3U, comments.size());
    EXPECT_EQ(" // this sets alpha\n", comments[0].as_string());
    EXPECT_EQ(" // bravo is set here\n", comments[1].as_string());
    EXPECT_EQ(" /* and here is gamma */\n", comments[2].as_string());
    comments.resize(0);

    s.GetComments((int)(strchr(input, '/') - input - 1),
                  (int)(strchr(input + 1, '\n') - input + 1), &comments);
    EXPECT_EQ(1U, comments.size());
    EXPECT_EQ(" // this sets alpha\n", comments[0].as_string());
    comments.resize(0);
}

TEST(RegexScanner, BigComment)
{
    std::string input;
    for (int i = 0; i < 1024; ++i) {
        char buf[1024];  // definitely big enough
        sprintf(buf, "    # Comment %d\n", i);
        input += buf;
    }
    input += "name = value;\n";

    RegexScaner s(input.c_str());
    s.SetSkipExpression("\\s+|#.*\n");

    std::string name;
    std::string value;
    s.Consume("(\\w+) = (\\w+);", &name, &value);
    EXPECT_EQ("name", name);
    EXPECT_EQ("value", value);
}

