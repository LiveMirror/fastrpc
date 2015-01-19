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
// Author: Sanjay Ghemawat

#ifndef REGEXARG_H
#define REGEXARG_H

#include <stdlib.h>    // for NULL
#include <string>
#include "pcre.h"
#include "regex_internal.h"
#include "string_piece.h"

// namespace common {

class StringPiece;

// Hex/Octal/Binary?

// Special class for parsing into objects that define a ParseFrom() method
template <class T>
class MatchObject {
public:
    static inline bool Parse(const char* str, int n, void* dest) {
        if (dest == NULL) return true;
        T* object = reinterpret_cast<T*>(dest);
        return object->ParseFrom(str, n);
    }
};

class REGEX_EXPORT_DEFINE ValueParser {
public:
    // Empty constructor so we can declare arrays of ValueParser
    ValueParser();

    // Constructor specially designed for NULL arguments
    ValueParser(void*);
    typedef bool (*Parser)(const char* str, int n, void* dest);

    // Type-specific parsers
#define MAKE_TYPE_PARSER(type, name)                                           \
    ValueParser(type* p) : m_arg(p), m_parser(name) { }                     \
    ValueParser(type* p, Parser parser) : m_arg(p), m_parser(parser) { }


    MAKE_TYPE_PARSER(char,               parse_char);
    MAKE_TYPE_PARSER(unsigned char,      parse_uchar);
    MAKE_TYPE_PARSER(short,              parse_short);
    MAKE_TYPE_PARSER(unsigned short,     parse_ushort);
    MAKE_TYPE_PARSER(int,                parse_int);
    MAKE_TYPE_PARSER(unsigned int,       parse_uint);
    MAKE_TYPE_PARSER(long,               parse_long);
    MAKE_TYPE_PARSER(unsigned long,      parse_ulong);
    MAKE_TYPE_PARSER(long long,          parse_longlong);
    MAKE_TYPE_PARSER(unsigned long long, parse_ulonglong);
    MAKE_TYPE_PARSER(float,              parse_float);
    MAKE_TYPE_PARSER(double,             parse_double);
    MAKE_TYPE_PARSER(std::string,        parse_string);
    MAKE_TYPE_PARSER(StringPiece,        parse_stringpiece);

#undef MAKE_TYPE_PARSER

    // Generic constructor
    template <class T> ValueParser(T*, Parser parser);
    // Generic constructor template
    template <class T> ValueParser(T* p)
        : m_arg(p), m_parser(MatchObject<T>::Parse)
    {
    }

    // Parse the data
    bool Parse(const char* str, int n) const;

private:
    void*         m_arg;
    Parser        m_parser;

    static bool parse_null          (const char* str, int n, void* dest);
    static bool parse_char          (const char* str, int n, void* dest);
    static bool parse_uchar         (const char* str, int n, void* dest);
    static bool parse_float         (const char* str, int n, void* dest);
    static bool parse_double        (const char* str, int n, void* dest);
    static bool parse_string        (const char* str, int n, void* dest);
    static bool parse_stringpiece   (const char* str, int n, void* dest);

#define DECLARE_INTEGER_PARSER(name)                                     \
private:                                                                      \
    static bool parse_ ## name(const char* str, int n, void* dest);           \
    static bool parse_ ## name ## _radix(                                     \
    const char* str, int n, void* dest, int radix);                           \
public:                                                                       \
    static bool parse_ ## name ## _hex(const char* str, int n, void* dest);   \
    static bool parse_ ## name ## _octal(const char* str, int n, void* dest); \
    static bool parse_ ## name ## _cradix(const char* str, int n, void* dest)

    DECLARE_INTEGER_PARSER(short);
    DECLARE_INTEGER_PARSER(ushort);
    DECLARE_INTEGER_PARSER(int);
    DECLARE_INTEGER_PARSER(uint);
    DECLARE_INTEGER_PARSER(long);
    DECLARE_INTEGER_PARSER(ulong);
    DECLARE_INTEGER_PARSER(longlong);
    DECLARE_INTEGER_PARSER(ulonglong);

#undef DECLARE_INTEGER_PARSER
};

inline ValueParser::ValueParser() : m_arg(NULL), m_parser(parse_null) { }
inline ValueParser::ValueParser(void* p) : m_arg(p), m_parser(parse_null) { }

inline bool ValueParser::Parse(const char* str, int n) const {
    return (*m_parser)(str, n, m_arg);
}

// This part of the parser, appropriate only for ints, deals with bases
#define DEFINE_INTEGER_PARSER(type, name) \
inline ValueParser Hex(type* ptr) { \
    return ValueParser(ptr, ValueParser::parse_ ## name ## _hex); } \
inline ValueParser Octal(type* ptr) { \
    return ValueParser(ptr, ValueParser::parse_ ## name ## _octal); } \
inline ValueParser CRadix(type* ptr) { \
    return ValueParser(ptr, ValueParser::parse_ ## name ## _cradix); }

DEFINE_INTEGER_PARSER(short,              short)     /*                        */
DEFINE_INTEGER_PARSER(unsigned short,     ushort)    /*                        */
DEFINE_INTEGER_PARSER(int,                int)       /* Don't use semicolons   */
DEFINE_INTEGER_PARSER(unsigned int,       uint)      /* after these statement  */
DEFINE_INTEGER_PARSER(long,               long)      /* because they can cause */
DEFINE_INTEGER_PARSER(unsigned long,      ulong)     /* compiler warnings if   */
DEFINE_INTEGER_PARSER(long long,          longlong)  /* turned up high enough. */
DEFINE_INTEGER_PARSER(unsigned long long, ulonglong) /*                        */
#undef DEFINE_INTEGER_PARSER

// } // namespace common
#endif /* REGEXARG_H */
