/***************************************************************************
 *
 * Copyright (c) 2013 FastRpc
 *
 **************************************************************************/

/**
 * @file codeconverter.h
 * @author hemingzhe(feimat@qq.com)
 * @date 2013/05/03
 * @brief
 *
 **/

#ifndef __CODECONVERTER_H_
#define __CODECONVERTER_H_

#include <stddef.h>
#include <string>
#include <vector>
#include <stdint.h>

#include "scopelocale.h"

using std::wstring;
using std::vector;
using std::string;

class CodeConverter {
public:
    static size_t GetUTF8CharLength(const char *ch);
    static size_t GetUTF8StrLength(const char *input, size_t len);
    static size_t GetUTF8StrLength(const string &input);

    static bool IsValidUTF8String(const string &input);
    static bool IsValidUTF8String_Simple_Check(const string &input);

    static void UCS2ToUTF8(uint16_t c, char *res);
    static size_t UCS4ToUTF8(uint32_t c, char *res);
    static string UCS4ToUTF8(uint32_t c);

    static uint16_t UTF8ToUCS2(const char *begin, const char *end, size_t *mblen);
    static uint32_t UTF8ToUCS4(const char *begin, const char *end, size_t *mblen);
    static uint32_t UTF8ToUCS4(const string &input);

    static size_t UTF16ToUCS4(const uint16_t* pwUTF16, uint32_t& dwUCS4);
    static size_t UCS4ToUTF16(uint32_t dwUCS4, uint16_t* pwUTF16, size_t len);
    static size_t UTF8StrToUTF16Str(const char* pbszUTF8Str, uint16_t* pwszUTF16Str, size_t len);
    static size_t UTF16StrToUTF8Str(const uint16_t* pwszUTF16Str, char* pbszUTF8Str);

    static void UTF8ToUCS4List(const string &input, vector<uint32_t> &output);

    static void SubString(const string &input, const size_t start, const size_t length, string *result);
    static void SubString(const string &input, const size_t start, string *result);
    static string SubString(const string &input, const size_t start, const size_t length);
    static string SubString(const string &input, const size_t start);

    static string Trim(const string &input);

    static void UCS4ToUTF8Append(uint32_t c, string *output);
    static void UCS4ListToUTF8Append(const vector<uint32_t> &array, string *output);

    static string WideToUTF8(const wchar_t *input);
    static string WideToUTF8(const wstring &input);

    static wstring UTF8ToWide(const char *input);
    static wstring UTF8ToWide(const string &input);

    static wstring GBKStringToWString(const string& str);
    static string WStringToGBKString(const wstring& wstr);
};

#endif // __CODECONVERTER_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
