// -*- coding: utf-8 -*-
//
// Copyright (c) 2005 - 2010, Google Inc.
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
// Author: Sanjay Ghemawat
//
// TODO: Test extractions for PartialMatch/Consume

#include "regex.h"
#include <stdio.h>
#include <cassert>
#include <vector>
#include "gtest.h"
#include "config.h"

// using namespace common;

static void Timing1(int num_iters) {
    // Same pattern lots of times
    Regex pattern("ruby:\\d+");
    StringPiece p("ruby:1234");
    for (int j = num_iters; j > 0; j--) {
        EXPECT_TRUE(pattern.FullMatch(p));
    }
}

static void Timing2(int num_iters) {
    // Same pattern lots of times
    Regex pattern("ruby:(\\d+)");
    int i;
    for (int j = num_iters; j > 0; j--) {
        EXPECT_TRUE(pattern.FullMatch("ruby:1234", &i));
        EXPECT_EQ(1234, i);
    }
}

static void Timing3(int num_iters) {
    std::string text_string;
    for (int j = num_iters; j > 0; j--) {
        text_string += "this is another line\n";
    }

    Regex line_matcher(".*\n");
    std::string line;
    StringPiece text(text_string);
    int counter = 0;
    while (line_matcher.Consume(&text)) {
        counter++;
    }
    printf("Matched %d lines\n", counter);
}

TEST(Regex, Timing)
{
    Timing1(1000);
    Timing2(1000);
    Timing3(1000);
}

#if 0  // uncomment this if you have a way of defining VirtualProcessSize()

static void LeakTest() {
    // Check for memory leaks
    unsigned long long initial_size = 0;
    for (int i = 0; i < 100000; i++) {
        if (i == 50000) {
            initial_size = VirtualProcessSize();
            printf("Size after 50000: %llu\n", initial_size);
        }
        char buf[100];  // definitely big enough
        sprintf(buf, "pat%09d", i);
        Regex newre(buf);
    }
    uint64 final_size = VirtualProcessSize();
    printf("Size after 100000: %llu\n", final_size);
    const double growth = double(final_size - initial_size) / final_size;
    printf("Growth: %0.2f%%", growth * 100);
    EXPECT_TRUE(growth < 0.02);       // Allow < 2% growth
}

#endif

TEST(Regex, RadixTests) {
    printf("Testing hex\n");

#define EXPECT_TRUE_HEX(type, value) \
    do { \
        type v; \
        EXPECT_TRUE(Regex("([0-9a-fA-F]+)[uUlL]*").FullMatch(#value, Hex(&v))); \
        EXPECT_EQ(0x ## value, v); \
        EXPECT_TRUE(Regex("([0-9a-fA-FxX]+)[uUlL]*").FullMatch("0x" #value, CRadix(&v))); \
        EXPECT_EQ(0x ## value, v); \
    } while(0)

    EXPECT_TRUE_HEX(short,              2bad);
    EXPECT_TRUE_HEX(unsigned short,     2badU);
    EXPECT_TRUE_HEX(int,                dead);
    EXPECT_TRUE_HEX(unsigned int,       deadU);
    EXPECT_TRUE_HEX(long,               7eadbeefL);
    EXPECT_TRUE_HEX(unsigned long,      deadbeefUL);
#ifdef HAVE_LONG_LONG
    EXPECT_TRUE_HEX(long long,          12345678deadbeefLL);
#endif
#ifdef HAVE_UNSIGNED_LONG_LONG
    EXPECT_TRUE_HEX(unsigned long long, cafebabedeadbeefULL);
#endif

#undef EXPECT_TRUE_HEX

    printf("Testing octal\n");

#define EXPECT_TRUE_OCTAL(type, value) \
    do { \
        type v; \
        EXPECT_TRUE(Regex("([0-7]+)[uUlL]*").FullMatch(#value, Octal(&v))); \
        EXPECT_EQ(0 ## value, v); \
        EXPECT_TRUE(Regex("([0-9a-fA-FxX]+)[uUlL]*").FullMatch("0" #value, CRadix(&v))); \
        EXPECT_EQ(0 ## value, v); \
    } while(0)

    EXPECT_TRUE_OCTAL(short,              77777);
    EXPECT_TRUE_OCTAL(unsigned short,     177777U);
    EXPECT_TRUE_OCTAL(int,                17777777777);
    EXPECT_TRUE_OCTAL(unsigned int,       37777777777U);
    EXPECT_TRUE_OCTAL(long,               17777777777L);
    EXPECT_TRUE_OCTAL(unsigned long,      37777777777UL);
#ifdef HAVE_LONG_LONG
    EXPECT_TRUE_OCTAL(long long,          777777777777777777777LL);
#endif
#ifdef HAVE_UNSIGNED_LONG_LONG
    EXPECT_TRUE_OCTAL(unsigned long long, 1777777777777777777777ULL);
#endif

#undef EXPECT_TRUE_OCTAL

    printf("Testing decimal\n");

#define EXPECT_TRUE_DECIMAL(type, value) \
    do { \
        type v; \
        EXPECT_TRUE(Regex("(-?[0-9]+)[uUlL]*").FullMatch(#value, &v)); \
        EXPECT_EQ(value, v); \
        EXPECT_TRUE(Regex("(-?[0-9a-fA-FxX]+)[uUlL]*").FullMatch(#value, CRadix(&v))); \
        EXPECT_EQ(value, v); \
    } while(0)

    EXPECT_TRUE_DECIMAL(short,              -1);
    EXPECT_TRUE_DECIMAL(unsigned short,     9999);
    EXPECT_TRUE_DECIMAL(int,                -1000);
    EXPECT_TRUE_DECIMAL(unsigned int,       12345U);
    EXPECT_TRUE_DECIMAL(long,               -10000000L);
    EXPECT_TRUE_DECIMAL(unsigned long,      3083324652U);
#ifdef HAVE_LONG_LONG
    EXPECT_TRUE_DECIMAL(long long,          -100000000000000LL);
#endif
#ifdef HAVE_UNSIGNED_LONG_LONG
    EXPECT_TRUE_DECIMAL(unsigned long long, 1234567890987654321ULL);
#endif

#undef EXPECT_TRUE_DECIMAL

}

TEST(Regex, Replace) {
    printf("Testing Replace\n");

    struct ReplaceTest {
        const char *regexp;
        const char *rewrite;
        const char *original;
        const char *single;
        const char *global;
        int global_count;         // the expected return value from ReplaceAll
    };
    static const ReplaceTest tests[] = {
        { "(qu|[b-df-hj-np-tv-z]*)([a-z]+)",
            "\\2\\1ay",
            "the quick brown fox jumps over the lazy dogs.",
            "ethay quick brown fox jumps over the lazy dogs.",
            "ethay ickquay ownbray oxfay umpsjay overay ethay azylay ogsday.",
            9 },
        { "\\w+",
            "\\0-NOSPAM",
            "paul.haahr@google.com",
            "paul-NOSPAM.haahr@google.com",
            "paul-NOSPAM.haahr-NOSPAM@google-NOSPAM.com-NOSPAM",
            4 },
        { "^",
            "(START)",
            "foo",
            "(START)foo",
            "(START)foo",
            1 },
        { "^",
            "(START)",
            "",
            "(START)",
            "(START)",
            1 },
        { "$",
            "(END)",
            "",
            "(END)",
            "(END)",
            1 },
        { "b",
            "bb",
            "ababababab",
            "abbabababab",
            "abbabbabbabbabb",
            5 },
        { "b",
            "bb",
            "bbbbbb",
            "bbbbbbb",
            "bbbbbbbbbbbb",
            6 },
        { "b+",
            "bb",
            "bbbbbb",
            "bb",
            "bb",
            1 },
        { "b*",
            "bb",
            "bbbbbb",
            "bb",
            "bbbb",
            2 },
        { "b*",
            "bb",
            "aaaaa",
            "bbaaaaa",
            "bbabbabbabbabbabb",
            6 },
        { "b*",
            "bb",
            "aa\naa\n",
            "bbaa\naa\n",
            "bbabbabb\nbbabbabb\nbb",
            7 },
        { "b*",
            "bb",
            "aa\raa\r",
            "bbaa\raa\r",
            "bbabbabb\rbbabbabb\rbb",
            7 },
        { "b*",
            "bb",
            "aa\r\naa\r\n",
            "bbaa\r\naa\r\n",
            "bbabbabb\r\nbbabbabb\r\nbb",
            7 },
        // Check empty-string matching (it's tricky!)
        { "aa|b*",
            "@",
            "aa",
            "@",
            "@@",
            2 },
        { "b*|aa",
            "@",
            "aa",
            "@aa",
            "@@@",
            3 },
#ifdef SUPPORT_UTF8
        { "b*",
            "bb",
            "\xE3\x83\x9B\xE3\x83\xBC\xE3\x83\xA0\xE3\x81\xB8",   // utf8
            "bb\xE3\x83\x9B\xE3\x83\xBC\xE3\x83\xA0\xE3\x81\xB8",
            "bb\xE3\x83\x9B""bb""\xE3\x83\xBC""bb""\xE3\x83\xA0""bb""\xE3\x81\xB8""bb",
            5 },
        { "b*",
            "bb",
            "\xE3\x83\x9B\r\n\xE3\x83\xBC\r\xE3\x83\xA0\n\xE3\x81\xB8\r\n",   // utf8
            "bb\xE3\x83\x9B\r\n\xE3\x83\xBC\r\xE3\x83\xA0\n\xE3\x81\xB8\r\n",
            ("bb\xE3\x83\x9B""bb\r\nbb""\xE3\x83\xBC""bb\rbb""\xE3\x83\xA0"
             "bb\nbb""\xE3\x81\xB8""bb\r\nbb"),
            9 },
#endif
        { "", NULL, NULL, NULL, NULL, 0 }
    };

#ifdef SUPPORT_UTF8
    const bool support_utf8 = true;
#else
    const bool support_utf8 = false;
#endif

    for (const ReplaceTest *t = tests; t->original != NULL; ++t) {
        Regex regex(t->regexp, Regex::Options(PCRE_NEWLINE_CRLF).SetUtf8(support_utf8));
        assert(regex.Error().empty());
        std::string one(t->original);
        EXPECT_TRUE(regex.Replace(t->rewrite, &one));
        EXPECT_EQ(t->single, one);
        std::string all(t->original);
        const int replace_count = regex.GlobalReplace(t->rewrite, &all);
        EXPECT_EQ(t->global, all);
        EXPECT_EQ(t->global_count, replace_count);
    }

    // One final test: test \r\n replacement when we'regex not in CRLF mode
    {
        Regex regex("b*", Regex::Options(PCRE_NEWLINE_CR).SetUtf8(support_utf8));
        assert(regex.Error().empty());
        std::string all("aa\r\naa\r\n");
        EXPECT_EQ(9, regex.GlobalReplace("bb", &all));
        EXPECT_EQ(std::string("bbabbabb\rbb\nbbabbabb\rbb\nbb"), all);
    }
    {
        Regex regex("b*", Regex::Options(PCRE_NEWLINE_LF).SetUtf8(support_utf8));
        assert(regex.Error().empty());
        std::string all("aa\r\naa\r\n");
        EXPECT_EQ(9, regex.GlobalReplace("bb", &all));
        EXPECT_EQ(std::string("bbabbabb\rbb\nbbabbabb\rbb\nbb"), all);
    }
    // TODO: test what happens when no PCRE_NEWLINE_* flag is set.
    //       Alas, the answer depends on how pcre was compiled.
}

TEST(Regex, Extract) {
    printf("Testing Extract\n");

    std::string s;

    EXPECT_TRUE(Regex("(.*)@([^.]*)").Extract("\\2!\\1", "boris@kremvax.ru", &s));
    EXPECT_EQ("kremvax!boris", s);

    // check the Regex interface as well
    EXPECT_TRUE(Regex(".*").Extract("'\\0'", "foo", &s));
    EXPECT_EQ("'foo'", s);
    EXPECT_TRUE(!Regex("bar").Extract("'\\0'", "baz", &s));
    EXPECT_EQ("'foo'", s);
}

TEST(Regex, Consume) {
    printf("Testing Consume\n");

    std::string word;

    std::string s("   aaa b!@#$@#$cccc");
    StringPiece input(s);

    Regex r("\\s*(\\w+)");    // matches a word, possibly proceeded by whitespace
    EXPECT_TRUE(r.Consume(&input, &word));
    EXPECT_EQ("aaa", word);
    EXPECT_TRUE(r.Consume(&input, &word));
    EXPECT_EQ("b", word);
    EXPECT_TRUE(!r.Consume(&input, &word));
}

TEST(Regex, FindAndConsume) {
    printf("Testing FindAndConsume\n");

    std::string word;

    std::string s("   aaa b!@#$@#$cccc");
    StringPiece input(s);

    Regex r("(\\w+)");      // matches a word
    EXPECT_TRUE(r.FindAndConsume(&input, &word));
    EXPECT_EQ("aaa", word);
    EXPECT_TRUE(r.FindAndConsume(&input, &word));
    EXPECT_EQ("b", word);
    EXPECT_TRUE(r.FindAndConsume(&input, &word));
    EXPECT_EQ("cccc", word);
    EXPECT_TRUE(!r.FindAndConsume(&input, &word));
}

TEST(Regex, MatchNumberPeculiarity) {
    printf("Testing match-number peculiaraity\n");

    std::string word1;
    std::string word2;
    std::string word3;

    Regex r("(foo)|(bar)|(baz)");
    EXPECT_TRUE(r.PartialMatch("foo", &word1, &word2, &word3));
    EXPECT_EQ("foo", word1);
    EXPECT_EQ("", word2);
    EXPECT_EQ("", word3);
    EXPECT_TRUE(r.PartialMatch("bar", &word1, &word2, &word3));
    EXPECT_EQ("", word1);
    EXPECT_EQ("bar", word2);
    EXPECT_EQ("", word3);
    EXPECT_TRUE(r.PartialMatch("baz", &word1, &word2, &word3));
    EXPECT_EQ("", word1);
    EXPECT_EQ("", word2);
    EXPECT_EQ("baz", word3);
    EXPECT_TRUE(!r.PartialMatch("f", &word1, &word2, &word3));

    std::string a;
    EXPECT_TRUE(Regex("(foo)|hello").FullMatch("hello", &a));
    EXPECT_EQ("", a);
}

TEST(Regex, Recursion) {
    printf("Testing recursion\n");

    // Get one string that passes (sometimes), one that never does.
    std::string text_good("abcdefghijk");
    std::string text_bad("acdefghijkl");

    // According to pcretest, matching text_good against (\w+)*b
    // requires MatchLimit of at least 8192, and match_recursion_limit
    // of at least 37.

    Regex::Options options_ml;
    options_ml.SetMatchLimit(8192);
    Regex re("(\\w+)*b", options_ml);
    EXPECT_TRUE(re.PartialMatch(text_good) == true);
    EXPECT_TRUE(re.PartialMatch(text_bad) == false);
    EXPECT_TRUE(re.FullMatch(text_good) == false);
    EXPECT_TRUE(re.FullMatch(text_bad) == false);

    options_ml.SetMatchLimit(1024);
    Regex re2("(\\w+)*b", options_ml);
    EXPECT_TRUE(re2.PartialMatch(text_good) == false);   // because of MatchLimit
    EXPECT_TRUE(re2.PartialMatch(text_bad) == false);
    EXPECT_TRUE(re2.FullMatch(text_good) == false);
    EXPECT_TRUE(re2.FullMatch(text_bad) == false);

    Regex::Options options_mlr;
    options_mlr.SetMatchLimitRecursion(50);
    Regex re3("(\\w+)*b", options_mlr);
    EXPECT_TRUE(re3.PartialMatch(text_good) == true);
    EXPECT_TRUE(re3.PartialMatch(text_bad) == false);
    EXPECT_TRUE(re3.FullMatch(text_good) == false);
    EXPECT_TRUE(re3.FullMatch(text_bad) == false);

    options_mlr.SetMatchLimitRecursion(10);
    Regex re4("(\\w+)*b", options_mlr);
    EXPECT_TRUE(re4.PartialMatch(text_good) == false);
    EXPECT_TRUE(re4.PartialMatch(text_bad) == false);
    EXPECT_TRUE(re4.FullMatch(text_good) == false);
    EXPECT_TRUE(re4.FullMatch(text_bad) == false);
}

// A meta-quoted string, interpreted as a pattern, should always match
// the original unquoted string.
static void TestQuoteMeta(const std::string& unquoted, const Regex::Options& options = Regex::Options()) {
    std::string quoted = Regex::QuoteMeta(unquoted);
    Regex Regex(quoted, options);
    EXPECT_TRUE(Regex.FullMatch(unquoted));
}

// A string containing meaningful regexp characters, which is then meta-
// quoted, should not generally match a string the unquoted string does.
static void NegativeTestQuoteMeta(
    const std::string& unquoted,
    const std::string& should_not_match,
    const Regex::Options& options = Regex::Options()
    )
{
    std::string quoted = Regex::QuoteMeta(unquoted);
    Regex Regex(quoted, options);
    EXPECT_TRUE(!Regex.FullMatch(should_not_match));
}

// Tests that quoted meta characters match their original strings,
// and that a few things that shouldn't match indeed do not.
TEST(QuoteMeta, QuotaMetaSimple) {
    TestQuoteMeta("foo");
    TestQuoteMeta("foo.bar");
    TestQuoteMeta("foo\\.bar");
    TestQuoteMeta("[1-9]");
    TestQuoteMeta("1.5-2.0?");
    TestQuoteMeta("\\d");
    TestQuoteMeta("Who doesn't like ice cream?");
    TestQuoteMeta("((a|b)c?d*e+[f-h]i)");
    TestQuoteMeta("((?!)xxx).*yyy");
    TestQuoteMeta("([");
    TestQuoteMeta(std::string("foo\0bar", 7));
}

TEST(QuoteMeta, QuoteMetaSimpleNegative) {
    NegativeTestQuoteMeta("foo", "bar");
    NegativeTestQuoteMeta("...", "bar");
    NegativeTestQuoteMeta("\\.", ".");
    NegativeTestQuoteMeta("\\.", "..");
    NegativeTestQuoteMeta("(a)", "a");
    NegativeTestQuoteMeta("(a|b)", "a");
    NegativeTestQuoteMeta("(a|b)", "(a)");
    NegativeTestQuoteMeta("(a|b)", "a|b");
    NegativeTestQuoteMeta("[0-9]", "0");
    NegativeTestQuoteMeta("[0-9]", "0-9");
    NegativeTestQuoteMeta("[0-9]", "[9]");
    NegativeTestQuoteMeta("((?!)xxx)", "xxx");
}

TEST(QuoteMeta, QuoteMetaLatin1) {
    TestQuoteMeta("3\xb2 = 9");
}

TEST(QuoteMeta, QuoteMetaUtf8) {
#ifdef SUPPORT_UTF8
    TestQuoteMeta("Pl\xc3\xa1\x63ido Domingo", Regex::UTF8());
    TestQuoteMeta("xyz", Regex::UTF8());            // No fancy utf8
    TestQuoteMeta("\xc2\xb0", Regex::UTF8());       // 2-byte utf8 (degree symbol)
    TestQuoteMeta("27\xc2\xb0 degrees", Regex::UTF8());  // As a middle character
    TestQuoteMeta("\xe2\x80\xb3", Regex::UTF8());   // 3-byte utf8 (double prime)
    TestQuoteMeta("\xf0\x9d\x85\x9f", Regex::UTF8()); // 4-byte utf8 (music note)
    TestQuoteMeta("27\xc2\xb0"); // Interpreted as Latin-1, but should still work
    NegativeTestQuoteMeta("27\xc2\xb0",               // 2-byte utf (degree symbol)
                          "27\\\xc2\\\xb0",
                          Regex::UTF8());
#endif
}

// Regex::Options tests contributed by
// Giuseppe Maxia, CTO, Stardata s.r.l.
// July 2005
//
static void GetOneOptionResult(
    const char *option_name,
    const char *regex,
    const char *str,
    Regex::Options options,
    bool full,
    std::string expected) {

    printf("Testing Option <%s>\n", option_name);
    printf("/%s/ finds \"%s\" within \"%s\" \n",
           regex,
           expected.c_str(),
           str);
    std::string captured("");
    if (full)
        Regex(regex,options).FullMatch(str, &captured);
    else
        Regex(regex,options).PartialMatch(str, &captured);
    EXPECT_EQ(expected, captured);
}

static void TestOneOption(
    const char *option_name,
    const char *regex,
    const char *str,
    Regex::Options options,
    bool full,
    bool assertive = true) {

    printf("Testing Option <%s>\n", option_name);
    printf("'%s' %s /%s/ \n",
           str,
           (assertive? "matches" : "doesn't match"),
           regex);
    if (assertive) {
        if (full)
            EXPECT_TRUE(Regex(regex, options).FullMatch(str));
        else
            EXPECT_TRUE(Regex(regex, options).PartialMatch(str));
    } else {
        if (full)
            EXPECT_TRUE(!Regex(regex, options).FullMatch(str));
        else
            EXPECT_TRUE(!Regex(regex, options).PartialMatch(str));
    }
}

TEST(Options, _CASELESS) {
    Regex::Options options;
    Regex::Options options2;

    options.SetCaseless(true);
    TestOneOption("CASELESS (class)",  "HELLO",    "hello", options, false);
    TestOneOption("CASELESS (class2)", "HELLO",    "hello", options2.SetCaseless(true), false);
    TestOneOption("CASELESS (class)",  "^[A-Z]+$", "Hello", options, false);

    TestOneOption("CASELESS (function)", "HELLO",    "hello", Regex::CASELESS(), false);
    TestOneOption("CASELESS (function)", "^[A-Z]+$", "Hello", Regex::CASELESS(), false);
    options.SetCaseless(false);
    TestOneOption("no CASELESS", "HELLO",    "hello", options, false, false);
}

TEST(Options, _MULTILINE) {
    Regex::Options options;
    Regex::Options options2;
    const char *str = "HELLO\n" "cruel\n" "world\n";

    options.SetMultiLine(true);
    TestOneOption("MULTILINE (class)",    "^cruel$", str, options, false);
    TestOneOption("MULTILINE (class2)",   "^cruel$", str, options2.SetMultiLine(true), false);
    TestOneOption("MULTILINE (function)", "^cruel$", str, Regex::MULTI_LINE(), false);
    options.SetMultiLine(false);
    TestOneOption("no MULTILINE", "^cruel$", str, options, false, false);
}

TEST(Options, _DOTALL) {
    Regex::Options options;
    Regex::Options options2;
    const char *str = "HELLO\n" "cruel\n" "world";

    options.SetDotAll(true);
    TestOneOption("DOTALL (class)",    "HELLO.*world", str, options, true);
    TestOneOption("DOTALL (class2)",   "HELLO.*world", str, options2.SetDotAll(true), true);
    TestOneOption("DOTALL (function)",    "HELLO.*world", str, Regex::DOT_ALL(), true);
    options.SetDotAll(false);
    TestOneOption("no DOTALL", "HELLO.*world", str, options, true, false);
}

TEST(Options, _DOLLAR_ENDONLY) {
    Regex::Options options;
    Regex::Options options2;
    const char *str = "HELLO world\n";

    TestOneOption("no DOLLAR_ENDONLY", "world$", str, options, false);
    options.SetDollarEndOnly(true);
    TestOneOption("DOLLAR_ENDONLY 1",    "world$", str, options, false, false);
    TestOneOption("DOLLAR_ENDONLY 2",    "world$", str, options2.SetDollarEndOnly(true), false, false);
}

TEST(Options, _EXTRA) {
    Regex::Options options;
    const char *str = "HELLO";

    options.SetExtra(true);
    TestOneOption("EXTRA 1", "\\HELL\\O", str, options, true, false );
    TestOneOption("EXTRA 2", "\\HELL\\O", str, Regex::Options().SetExtra(true), true, false );
    options.SetExtra(false);
    TestOneOption("no EXTRA", "\\HELL\\O", str, options, true );
}

TEST(Options, _EXTENDED) {
    Regex::Options options;
    Regex::Options options2;
    const char *str = "HELLO world";

    options.SetExtended(true);
    TestOneOption("EXTENDED (class)",    "HELLO world", str, options, false, false);
    TestOneOption("EXTENDED (class2)",   "HELLO world", str, options2.SetExtended(true), false, false);
    TestOneOption("EXTENDED (class)",
                  "^ HE L{2} O "
                  "\\s+        "
                  "\\w+ $      ",
                  str,
                  options,
                  false);

    TestOneOption("EXTENDED (function)",    "HELLO world", str, Regex::EXTENDED(), false, false);
    TestOneOption("EXTENDED (function)",
                  "^ HE L{2} O "
                  "\\s+        "
                  "\\w+ $      ",
                  str,
                  Regex::EXTENDED(),
                  false);

    options.SetExtended(false);
    TestOneOption("no EXTENDED", "HELLO world", str, options, false);
}

TEST(Options, _NO_AUTO_CAPTURE) {
    Regex::Options options;
    const char *str = "HELLO world";
    std::string captured;

    printf("Testing Option <no NO_AUTO_CAPTURE>\n");
    printf("parentheses capture text\n");
    Regex Regex("(world|universe)$", options);
    EXPECT_TRUE(Regex.Extract("\\1", str , &captured));
    EXPECT_EQ("world", captured);
    options.SetNoAutoCapture(true);
    printf("testing Option <NO_AUTO_CAPTURE>\n");
    printf("parentheses do not capture text\n");
    Regex.Extract("\\1",str, &captured );
    EXPECT_EQ("world", captured);
}

TEST(Options, _UNGREEDY) {
    Regex::Options options;
    const char *str = "HELLO, 'this' is the 'world'";

    options.SetUngreedy(true);
    GetOneOptionResult("UNGREEDY 1", "('.*')", str, options, false, "'this'" );
    GetOneOptionResult("UNGREEDY 2", "('.*')", str, Regex::Options().SetUngreedy(true), false, "'this'" );
    GetOneOptionResult("UNGREEDY", "('.*?')", str, options, false, "'this' is the 'world'" );

    options.SetUngreedy(false);
    GetOneOptionResult("no UNGREEDY", "('.*')", str, options, false, "'this' is the 'world'" );
    GetOneOptionResult("no UNGREEDY", "('.*?')", str, options, false, "'this'" );
}

TEST(Options, _all_options) {
    const char *str = "HELLO\n" "cruel\n" "world";
    Regex::Options options;
    options.SetAllOptions(PCRE_CASELESS | PCRE_DOTALL);

    TestOneOption("AllOptions (CASELESS|DOTALL)", "^hello.*WORLD", str , options, false);
    options.SetAllOptions(0);
    TestOneOption("AllOptions (0)", "^hello.*WORLD", str , options, false, false);
    options.SetAllOptions(PCRE_MULTILINE | PCRE_EXTENDED);

    TestOneOption("AllOptions (MULTILINE|EXTENDED)", " ^ c r u e l $ ", str, options, false);
    TestOneOption("AllOptions (MULTILINE|EXTENDED) with constructor",
                  " ^ c r u e l $ ",
                  str,
                  Regex::Options(PCRE_MULTILINE | PCRE_EXTENDED),
                  false);

    TestOneOption("AllOptions (MULTILINE|EXTENDED) with concatenation",
                  " ^ c r u e l $ ",
                  str,
                  Regex::Options()
                  .SetMultiLine(true)
                  .SetExtended(true),
                  false);

    options.SetAllOptions(0);
    TestOneOption("AllOptions (0)", "^ c r u e l $", str, options, false, false);

}

TEST(Regex, Constructors) {
    printf("Testing constructors\n");

    Regex::Options options;
    options.SetDotAll(true);
    const char *str = "HELLO\n" "cruel\n" "world";

    Regex orig("HELLO.*world", options);
    EXPECT_TRUE(orig.FullMatch(str));

    Regex copy1(orig);
    EXPECT_TRUE(copy1.FullMatch(str));

    Regex copy2("not a match");
    EXPECT_TRUE(!copy2.FullMatch(str));
    copy2 = copy1;
    EXPECT_TRUE(copy2.FullMatch(str));
    copy2 = orig;
    EXPECT_TRUE(copy2.FullMatch(str));

    // Make sure when we assign to ourselves, nothing bad happens
    orig = orig;
    copy1 = copy1;
    copy2 = copy2;
    EXPECT_TRUE(orig.FullMatch(str));
    EXPECT_TRUE(copy1.FullMatch(str));
    EXPECT_TRUE(copy2.FullMatch(str));
}

TEST(Regex, FullMatch)
{
    EXPECT_TRUE(Regex("h.*o").FullMatch("hello"));
    EXPECT_TRUE(!Regex("h.*o").FullMatch("othello"));     // Must be anchored at front
    EXPECT_TRUE(!Regex("h.*o").FullMatch("hello!"));      // Must be anchored at end
    EXPECT_TRUE(Regex("a*").FullMatch("aaaa"));           // Fullmatch with normal op
    EXPECT_TRUE(Regex("a*?").FullMatch("aaaa"));          // Fullmatch with nongreedy op
    EXPECT_TRUE(Regex("a*?\\z").FullMatch("aaaa"));       // Two unusual ops

    // Zero-arg
    EXPECT_TRUE(Regex("\\d+").FullMatch("1001"));

    int i;

    // Single-arg
    EXPECT_TRUE(Regex("(\\d+)").FullMatch("1001",   &i));
    EXPECT_EQ(1001, i);
    EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("-123", &i));
    EXPECT_EQ(-123, i);
    EXPECT_TRUE(!Regex("()\\d+").FullMatch("10", &i));
    EXPECT_TRUE(!Regex("(\\d+)").FullMatch("1234567890123456789012345678901234567890",
                                     &i));

    // Digits surrounding integer-arg
    EXPECT_TRUE(Regex("1(\\d*)4").FullMatch("1234", &i));
    EXPECT_EQ(23, i);
    EXPECT_TRUE(Regex("(\\d)\\d+").FullMatch("1234", &i));
    EXPECT_EQ(1, i);
    EXPECT_TRUE(Regex("(-\\d)\\d+").FullMatch("-1234", &i));
    EXPECT_EQ(-1, i);
    EXPECT_TRUE(Regex("(\\d)").PartialMatch("1234", &i));
    EXPECT_EQ(1, i);
    EXPECT_TRUE(Regex("(-\\d)").PartialMatch("-1234", &i));
    EXPECT_EQ(-1, i);

    std::string s;

    // String-arg
    EXPECT_TRUE(Regex("h(.*)o").FullMatch("hello", &s));
    EXPECT_EQ(std::string("ell"), s);

    // StringPiece-arg
    StringPiece sp;
    EXPECT_TRUE(Regex("(\\w+):(\\d+)").FullMatch("ruby:1234", &sp, &i));
    EXPECT_EQ(4U, sp.size());
    EXPECT_TRUE(memcmp(sp.data(), "ruby", 4) == 0);
    EXPECT_EQ(1234, i);

    // Multi-arg
    EXPECT_TRUE(Regex("(\\w+):(\\d+)").FullMatch("ruby:1234", &s, &i));
    EXPECT_EQ(std::string("ruby"), s);
    EXPECT_EQ(1234, i);

    // Ignore non-void* NULL arg
    EXPECT_TRUE(Regex("he(.*)lo").FullMatch("hello", (char*)NULL));
    EXPECT_TRUE(Regex("h(.*)o").FullMatch("hello", (std::string*)NULL));
    EXPECT_TRUE(Regex("h(.*)o").FullMatch("hello", (StringPiece*)NULL));
    EXPECT_TRUE(Regex("(.*)").FullMatch("1234", (int*)NULL));
    EXPECT_TRUE(Regex("(.*)").FullMatch("1234567890123456", (long long*)NULL));
    EXPECT_TRUE(Regex("(.*)").FullMatch("123.4567890123456", (double*)NULL));
    EXPECT_TRUE(Regex("(.*)").FullMatch("123.4567890123456", (float*)NULL));

    // Fail on non-void* NULL arg if the match doesn't parse for the given type.
    EXPECT_TRUE(!Regex("h(.*)lo").FullMatch("hello", &s, (char*)NULL));
    EXPECT_TRUE(!Regex("(.*)").FullMatch("hello", (int*)NULL));
    EXPECT_TRUE(!Regex("(.*)").FullMatch("1234567890123456", (int*)NULL));
    EXPECT_TRUE(!Regex("(.*)").FullMatch("hello", (double*)NULL));
    EXPECT_TRUE(!Regex("(.*)").FullMatch("hello", (float*)NULL));

    // Ignored arg
    EXPECT_TRUE(Regex("(\\w+)(:)(\\d+)").FullMatch("ruby:1234", &s, (void*)NULL, &i));
    EXPECT_EQ(std::string("ruby"), s);
    EXPECT_EQ(1234, i);
}

TEST(Regex, TypedExtract) {
    // Type tests
    {
        char c;
        EXPECT_TRUE(Regex("(H)ello").FullMatch("Hello", &c));
        EXPECT_EQ('H', c);
    }
    {
        unsigned char c;
        EXPECT_TRUE(Regex("(H)ello").FullMatch("Hello", &c));
        EXPECT_EQ(static_cast<unsigned char>('H'), c);
    }
    {
        short v;
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("100",     &v));    EXPECT_EQ(100, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("-100",    &v));    EXPECT_EQ(-100, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("32767",   &v));    EXPECT_EQ(32767, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("-32768",  &v));    EXPECT_EQ(-32768, v);
        EXPECT_TRUE(!Regex("(-?\\d+)").FullMatch("-32769", &v));
        EXPECT_TRUE(!Regex("(-?\\d+)").FullMatch("32768",  &v));
    }
    {
        unsigned short v;
        EXPECT_TRUE(Regex("(\\d+)").FullMatch("100",     &v));    EXPECT_EQ(100, v);
        EXPECT_TRUE(Regex("(\\d+)").FullMatch("32767",   &v));    EXPECT_EQ(32767, v);
        EXPECT_TRUE(Regex("(\\d+)").FullMatch("65535",   &v));    EXPECT_EQ(65535, v);
        EXPECT_TRUE(!Regex("(\\d+)").FullMatch("65536",  &v));
    }
    {
        int v;
        static const int max_value = 0x7fffffff;
        static const int min_value = -max_value - 1;
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("100",         &v)); EXPECT_EQ(100, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("-100",        &v)); EXPECT_EQ(-100, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("2147483647",  &v)); EXPECT_EQ(max_value, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("-2147483648", &v)); EXPECT_EQ(min_value, v);
        EXPECT_TRUE(!Regex("(-?\\d+)").FullMatch("-2147483649", &v));
        EXPECT_TRUE(!Regex("(-?\\d+)").FullMatch("2147483648",  &v));
    }
    {
        unsigned int v;
        static const unsigned int max_value = 0xfffffffful;
        EXPECT_TRUE(Regex("(\\d+)").FullMatch("100",         &v)); EXPECT_EQ(100U, v);
        EXPECT_TRUE(Regex("(\\d+)").FullMatch("4294967295",  &v)); EXPECT_EQ(max_value, v);
        EXPECT_TRUE(!Regex("(\\d+)").FullMatch("4294967296", &v));
    }
#ifdef HAVE_LONG_LONG
# if defined(__MINGW__) || defined(__MINGW32__)
#   define LLD "%I64d"
#   define LLU "%I64u"
# else
#   define LLD "%lld"
#   define LLU "%llu"
# endif
    {
        long long v;
        static const long long max_value = 0x7fffffffffffffffLL;
        static const long long min_value = -max_value - 1;
        char buf[32];  // definitely big enough for a long long

        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("100", &v)); EXPECT_EQ(100, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("-100",&v)); EXPECT_EQ(-100, v);

        sprintf(buf, LLD, max_value);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch(buf,&v)); EXPECT_EQ(max_value, v);

        sprintf(buf, LLD, min_value);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch(buf,&v)); EXPECT_EQ(min_value, v);

        sprintf(buf, LLD, max_value);
        assert(buf[strlen(buf)-1] != '9');
        buf[strlen(buf)-1]++;
        EXPECT_TRUE(!Regex("(-?\\d+)").FullMatch(buf, &v));

        sprintf(buf, LLD, min_value);
        assert(buf[strlen(buf)-1] != '9');
        buf[strlen(buf)-1]++;
        EXPECT_TRUE(!Regex("(-?\\d+)").FullMatch(buf, &v));
    }
#endif
#if defined HAVE_UNSIGNED_LONG_LONG && defined HAVE_LONG_LONG
    {
        unsigned long long v;
        long long v2;
        static const unsigned long long max_value = 0xffffffffffffffffULL;
        char buf[32];  // definitely big enough for a unsigned long long

        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("100", &v)); EXPECT_EQ(100ULL, v);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch("-100",&v2)); EXPECT_EQ(-100, v2);

        sprintf(buf, LLU, max_value);
        EXPECT_TRUE(Regex("(-?\\d+)").FullMatch(buf,&v)); EXPECT_EQ(max_value, v);

        assert(buf[strlen(buf)-1] != '9');
        buf[strlen(buf)-1]++;
        EXPECT_TRUE(!Regex("(-?\\d+)").FullMatch(buf, &v));
    }
#endif
    {
        float v;
        EXPECT_TRUE(Regex("(.*)").FullMatch("100", &v));
        EXPECT_TRUE(Regex("(.*)").FullMatch("-100.", &v));
        EXPECT_TRUE(Regex("(.*)").FullMatch("1e23", &v));
    }
    {
        double v;
        EXPECT_TRUE(Regex("(.*)").FullMatch("100", &v));
        EXPECT_TRUE(Regex("(.*)").FullMatch("-100.", &v));
        EXPECT_TRUE(Regex("(.*)").FullMatch("1e23", &v));
    }

    int i;
    // Check that matching is fully anchored
    EXPECT_TRUE(!Regex("(\\d+)").FullMatch("x1001",  &i));
    EXPECT_TRUE(!Regex("(\\d+)").FullMatch("1001x",  &i));
    EXPECT_TRUE(Regex("x(\\d+)").FullMatch("x1001", &i)); EXPECT_EQ(1001, i);
    EXPECT_TRUE(Regex("(\\d+)x").FullMatch("1001x", &i)); EXPECT_EQ(1001, i);

    // Braces
    EXPECT_TRUE(Regex("[0-9a-f+.-]{5,}").FullMatch("0abcd"));
    EXPECT_TRUE(Regex("[0-9a-f+.-]{5,}").FullMatch("0abcde"));
    EXPECT_TRUE(!Regex("[0-9a-f+.-]{5,}").FullMatch("0abc"));

    // Complicated Regex
    EXPECT_TRUE(Regex("foo|bar|[A-Z]").FullMatch("foo"));
    EXPECT_TRUE(Regex("foo|bar|[A-Z]").FullMatch("bar"));
    EXPECT_TRUE(Regex("foo|bar|[A-Z]").FullMatch("X"));
    EXPECT_TRUE(!Regex("foo|bar|[A-Z]").FullMatch("XY"));

    // Check full-match handling (needs '$' tacked on internally)
    EXPECT_TRUE(Regex("fo|foo").FullMatch("fo"));
    EXPECT_TRUE(Regex("fo|foo").FullMatch("foo"));
    EXPECT_TRUE(Regex("fo|foo$").FullMatch("fo"));
    EXPECT_TRUE(Regex("fo|foo$").FullMatch("foo"));
    EXPECT_TRUE(Regex("foo$").FullMatch("foo"));
    EXPECT_TRUE(!Regex("foo\\$").FullMatch("foo$bar"));
    EXPECT_TRUE(!Regex("fo|bar").FullMatch("fox"));

    // Uncomment the following if we change the handling of '$' to
    // prevent it from matching a trailing newline
    if (false) {
        // Check that we don't get bitten by pcre's special handling of a
        // '\n' at the end of the string matching '$'
        EXPECT_TRUE(!Regex("foo$").PartialMatch("foo\n"));
    }
}

TEST(Regex, NumberOfArgs)
{
    // Number of args
    int a[16];
    EXPECT_TRUE(Regex("").FullMatch(""));

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d){1}").FullMatch("1",
                                      &a[0]));
    EXPECT_EQ(1, a[0]);

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d)(\\d)").FullMatch("12",
                                        &a[0],  &a[1]));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d)(\\d)(\\d)").FullMatch("123",
                                             &a[0],  &a[1],  &a[2]));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d)(\\d)(\\d)(\\d)").FullMatch("1234",
                                                  &a[0],  &a[1],  &a[2],  &a[3]));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d)(\\d)(\\d)(\\d)(\\d)").FullMatch("12345",
                                                       &a[0],  &a[1],  &a[2],
                                                       &a[3],  &a[4]));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);
    EXPECT_EQ(5, a[4]);

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)").FullMatch("123456",
                                                            &a[0],  &a[1],  &a[2],
                                                            &a[3],  &a[4],  &a[5]));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);
    EXPECT_EQ(5, a[4]);
    EXPECT_EQ(6, a[5]);

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)").FullMatch("1234567",
                                                                 &a[0],  &a[1],  &a[2],  &a[3],
                                                                 &a[4],  &a[5],  &a[6]));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);
    EXPECT_EQ(5, a[4]);
    EXPECT_EQ(6, a[5]);
    EXPECT_EQ(7, a[6]);

    memset(a, 0, sizeof(a));
    EXPECT_TRUE(Regex("(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)"
                "(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)(\\d)").FullMatch(
                    "1234567890123456",
                    &a[0],  &a[1],  &a[2],  &a[3],
                    &a[4],  &a[5],  &a[6],  &a[7],
                    &a[8],  &a[9],  &a[10], &a[11],
                    &a[12], &a[13], &a[14], &a[15]));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);
    EXPECT_EQ(5, a[4]);
    EXPECT_EQ(6, a[5]);
    EXPECT_EQ(7, a[6]);
    EXPECT_EQ(8, a[7]);
    EXPECT_EQ(9, a[8]);
    EXPECT_EQ(0, a[9]);
    EXPECT_EQ(1, a[10]);
    EXPECT_EQ(2, a[11]);
    EXPECT_EQ(3, a[12]);
    EXPECT_EQ(4, a[13]);
    EXPECT_EQ(5, a[14]);
    EXPECT_EQ(6, a[15]);
}

TEST(Regex, PartialMatch)
{
    /***** PartialMatch *****/

    printf("Testing PartialMatch\n");

    EXPECT_TRUE(Regex("h.*o").PartialMatch("hello"));
    EXPECT_TRUE(Regex("h.*o").PartialMatch("othello"));
    EXPECT_TRUE(Regex("h.*o").PartialMatch("hello!"));
    EXPECT_TRUE(Regex("((((((((((((((((((((x))))))))))))))))))))").PartialMatch("x"));
}

TEST(Regex, Pattern)
{
    const std::string kPattern = "http://([^/]+)/.*";
    const Regex Regex(kPattern);
    EXPECT_EQ(Regex.Pattern(), kPattern);
}

TEST(Regex, Error)
{
    Regex Regex("foo");
    EXPECT_TRUE(Regex.Error().empty());  // Must have no error
}

#ifdef SUPPORT_UTF8
TEST(Regex, UTF8)
{
    // Check UTF-8 handling
    {
        printf("Testing UTF-8 handling\n");

        // Three Japanese characters (nihongo)
        const unsigned char utf8_string[] = {
            0xe6, 0x97, 0xa5, // 65e5
            0xe6, 0x9c, 0xac, // 627c
            0xe8, 0xaa, 0x9e, // 8a9e
            0
        };
        const unsigned char utf8_pattern[] = {
            '.',
            0xe6, 0x9c, 0xac, // 627c
            '.',
            0
        };

        // Both should match in either mode, bytes or UTF-8
        Regex re_test1(".........");
        EXPECT_TRUE(re_test1.FullMatch(utf8_string));
        Regex re_test2("...", Regex::UTF8());
        EXPECT_TRUE(re_test2.FullMatch(utf8_string));

        // Check that '.' matches one byte or UTF-8 character
        // according to the mode.
        std::string ss;
        Regex re_test3("(.)");
        EXPECT_TRUE(re_test3.PartialMatch(utf8_string, &ss));
        EXPECT_EQ(std::string("\xe6"), ss);
        Regex re_test4("(.)", Regex::UTF8());
        EXPECT_TRUE(re_test4.PartialMatch(utf8_string, &ss));
        EXPECT_EQ(std::string("\xe6\x97\xa5"), ss);

        // Check that string matches itself in either mode
        Regex re_test5(utf8_string);
        EXPECT_TRUE(re_test5.FullMatch(utf8_string));
        Regex re_test6(utf8_string, Regex::UTF8());
        EXPECT_TRUE(re_test6.FullMatch(utf8_string));

        // Check that pattern matches string only in UTF8 mode
        Regex re_test7(utf8_pattern);
        EXPECT_TRUE(!re_test7.FullMatch(utf8_string));
        Regex re_test8(utf8_pattern, Regex::UTF8());
        EXPECT_TRUE(re_test8.FullMatch(utf8_string));
    }

    // Check that ungreedy, UTF8 regular expressions don't match when they
    // oughtn't -- see bug 82246.
    {
        // This code always worked.
        const char* pattern = "\\w+X";
        const std::string target = "a aX";
        Regex match_sentence(pattern);
        Regex match_sentence_re(pattern, Regex::UTF8());

        EXPECT_TRUE(!match_sentence.FullMatch(target));
        EXPECT_TRUE(!match_sentence_re.FullMatch(target));
    }

    {
        const char* pattern = "(?U)\\w+X";
        const std::string target = "a aX";
        Regex match_sentence(pattern);
        Regex match_sentence_re(pattern, Regex::UTF8());

        EXPECT_TRUE(!match_sentence.FullMatch(target));
        EXPECT_TRUE(!match_sentence_re.FullMatch(target));
    }
#endif  /* def SUPPORT_UTF8 */
}

TEST(Regex, ErrorReporting)
{
    { Regex Regex("a\\1"); EXPECT_TRUE(!Regex.Error().empty()); }
    {
        Regex Regex("a[x");
        EXPECT_TRUE(!Regex.Error().empty());
    }
    {
        Regex Regex("a[z-a]");
        EXPECT_TRUE(!Regex.Error().empty());
    }
    {
        Regex Regex("a[[:foobar:]]");
        EXPECT_TRUE(!Regex.Error().empty());
    }
    {
        Regex Regex("a(b");
        EXPECT_TRUE(!Regex.Error().empty());
    }
    {
        Regex Regex("a\\");
        EXPECT_TRUE(!Regex.Error().empty());
    }
}
