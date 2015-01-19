/***************************************************************************
 *
 * Copyright (c) 2013 FastRpc
 *
 **************************************************************************/

/**
 * @file codeconverter.cpp
 * @author hemingzhe(feimat@qq.com)
 * @date 2013/05/03
 * @brief
 *
 **/

#include "codeconverter.h"


#ifndef WIN32
#include <string.h>
#include <wchar.h>
#else
#include <wchar.h>
#include <windows.h>
#endif

namespace {

// mapping between ucs4 and utf-8
// U-00000000 - U-0000007F: 0xxxxxxx
// U-00000080 - U-000007FF: 110xxxxx 10xxxxxx
// U-00000800 - U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
// U-00010000 - U-001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

// UTF-8 Length table base on first character
unsigned char kUTF8LengthTable[256] = {
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 5,5,5,5,6,6,0,0};

#ifndef WIN32
size_t wcstombs_r(char *output, const wchar_t *input, size_t len) {
    mbstate_t state;
    memset(&state, 0, sizeof(state));

    return wcsrtombs(output, &input, len, &state);
}
#else
size_t wcstombs_r(char *output, const wchar_t *input, size_t len) {
    // windows 这里先暂时只支持UTF8
    return WideCharToMultiByte(CP_UTF8, 0, input, -1, output, len, NULL, NULL) - 1;
}
#endif

#ifndef WIN32
size_t mbstowcs_r(wchar_t *output, const char *input, size_t len) {
    mbstate_t state;
    memset(&state, 0, sizeof(state));

    return mbsrtowcs(output, &input, len, &state);
}
#else
size_t mbstowcs_r(wchar_t *output, const char *input, size_t len) {
    // windows 这里先暂时只支持UTF8
    return MultiByteToWideChar(CP_UTF8, 0, input, -1, output, len) - 1;
}
#endif

} // end of anonymous namespace

// get utf8 char length
size_t CodeConverter::GetUTF8CharLength(const char *ch) {
    if (ch == NULL) {
        return 0;
    }
    return kUTF8LengthTable[*reinterpret_cast<const unsigned char*>(ch)];
}

// get utf8 str length
size_t CodeConverter::GetUTF8StrLength(const char *input, size_t len) {
    const char *begin = input;
    const char *end = begin + len;
    size_t number = 0;
    while (begin < end) {
        ++number;
        begin += GetUTF8CharLength(begin);
    }
    return number;
}

bool CodeConverter::IsValidUTF8String(const string &input) {
    const unsigned char *begin = reinterpret_cast<const unsigned char *>(input.data());
    const unsigned char *end = reinterpret_cast<const unsigned char *>(begin + input.size());
    while (begin < end) {
        size_t len = GetUTF8CharLength(reinterpret_cast<const char *>(begin));
        if (len == 0) {
            return false;
        }
        if (begin + len > end) {
            return false;
        }
        // ascii
        if (begin[0] < 0x80) {
            begin += len;
            continue;
        }
        if(!(0xC2 <= begin[0] && begin[0] <= 0xF4)) {
            return false;
        }
        if(begin[0] == 0xE0 && begin[1] < 0xA0) {
            return false;
        }
        if(begin[0] == 0xED && begin[1] > 0x9F) {
            return false;
        }
        if(begin[0] == 0xF0 && begin[1] < 0x90) {
            return false;
        }
        if(begin[0] == 0xF4 && begin[1] > 0x8F) {
            return false;
        }
        for (size_t i = 1; i < len; ++i) {
            // 10xx xxxx
            if (!(0x80 <= begin[i] && begin[i] <= 0xBF)) {
                return false;
            }
        }
        begin += len;
    }
    return true;
}

/*
 *  简单地检查 utf-8 字符是否合法, IsValidUTF8String 会认为
 *  绘文字所在的编码是非法的
 */
bool CodeConverter::IsValidUTF8String_Simple_Check(const string &input) {
    const unsigned char *begin = reinterpret_cast<const unsigned char *>(input.data());
    const unsigned char *end = reinterpret_cast<const unsigned char *>(begin + input.size());
    while (begin < end) {
        size_t len = GetUTF8CharLength(reinterpret_cast<const char *>(begin));
        if (len == 0) {
            return false;
        }
        if (begin + len > end) {
            return false;
        }
        // ascii
        if (begin[0] < 0x80) {
            begin += len;
            continue;
        }
        for (size_t i = 1; i < len; ++i) {
            // 10xx xxxx
            if (!(0x80 <= begin[i] && begin[i] <= 0xBF)) {
                return false;
            }
        }
        begin += len;
    }
    return true;
}

// get utf8 str length
size_t CodeConverter::GetUTF8StrLength(const string &input) {
    return GetUTF8StrLength(input.data(), input.size());
}

// convert ucs2 to utf8
void CodeConverter::UCS2ToUTF8(uint16_t c, char *res) {
    UCS4ToUTF8(static_cast<uint32_t>(c), res);
}

// convert ucs4 to utf8
size_t CodeConverter::UCS4ToUTF8(uint32_t c, char *res) {
    const uint32_t bound [] = {0x80, 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000};

    size_t index = 0;
    size_t bound_size = sizeof(bound)/sizeof(bound[0]);
    for(; index < bound_size; ++index) {
        if (c < bound[index]) {
            break;
        }
    }

    if (index == bound_size) {
        res[0] = '\0';
        return 0;
    }

    const unsigned char head [] = {0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
    const unsigned char headmask [] = {0xFF, 0x1F, 0x0F, 0x0F, 0x07, 0x03, 0x01};

    for (size_t i = index; i > 0; --i) {
        res[i] = static_cast<char>((c & 0x3F) | 0x80);
        c >>= 6;
    }

    res[0] = static_cast<char>((c & headmask[index]) | head[index]);

    res[index+1] = '\0';
    return index+1;
}

// convert ucs4 to utf8
string CodeConverter::UCS4ToUTF8(uint32_t c) {
    char buf[7];
    CodeConverter::UCS4ToUTF8(c, buf);
    string output = buf;
    return output;
}

// convert utf8 to ucs2
uint16_t CodeConverter::UTF8ToUCS2(const char *begin, const char *end, size_t *mblen) {
    return static_cast<uint16_t>(UTF8ToUCS4(begin, end, mblen));
}

// convert utf8 to ucs4
uint32_t CodeConverter::UTF8ToUCS4(const char *begin, const char *end, size_t *mblen) {
    if (begin == NULL || end == NULL) {
        *mblen = 0;
        return 0;
    };

    const size_t len = static_cast<size_t>(end - begin);
    if (len == 0) {
        *mblen = 0;
        return 0;
    }

    if (static_cast<unsigned char>(begin[0]) < 0x80) {
        *mblen = 1;
        return static_cast<unsigned char>(begin[0]);
    } else if (len >= 2 && (begin[0] & 0xe0) == 0xc0) {
        *mblen = 2;
        return ((begin[0] & 0x1f) << 6) | (begin[1] & 0x3f);
    } else if (len >= 3 && (begin[0] & 0xf0) == 0xe0) {
        *mblen = 3;
        return ((begin[0] & 0x0f) << 12) |
            ((begin[1] & 0x3f) << 6) | (begin[2] & 0x3f);
    } else if (len >= 4 && (begin[0] & 0xf8) == 0xf0) {
        *mblen = 4;
        return ((begin[0] & 0x07) << 18) |
            ((begin[1] & 0x3f) << 12) | ((begin[2] & 0x3f) << 6) |
            (begin[3] & 0x3f);
    }  else if (len >= 5 && (begin[0] & 0xfc) == 0xf8) {
        *mblen = 5;
        return ((begin[0] & 0x03) << 24) |
            ((begin[1] & 0x3f) << 18) | ((begin[2] & 0x3f) << 12) |
            ((begin[3] & 0x3f) << 6) | (begin[4] & 0x3f);
    } else if (len >= 6 && (begin[0] & 0xfe) == 0xfc) {
        *mblen = 6;
        return ((begin[0] & 0x01) << 30) |
            ((begin[1] & 0x3f) << 24) | ((begin[2] & 0x3f) << 18) |
            ((begin[3] & 0x3f) << 12) | ((begin[4] & 0x3f) << 6) |
            (begin[5] & 0x3f);
    } else {
        *mblen = 1;
        return 0;
    }
}

// convert utf8 to ucs4
uint32_t CodeConverter::UTF8ToUCS4(const string &input) {
    size_t mblen = 0;
    return UTF8ToUCS4(input.data(), input.data() + input.size(), &mblen);
}

// UCS4 to UCS2
size_t CodeConverter::UCS4ToUTF16(uint32_t dwUCS4, uint16_t* pwUTF16, size_t len) {
    if (pwUTF16 == NULL) {
        return 0;
    }

    if (dwUCS4 <= 0xFFFF) {
        *pwUTF16 = static_cast<uint16_t>(dwUCS4);
        return 1;
    } else if (dwUCS4 <= 0x10FFFF) {
        if (len >= 2) {
            pwUTF16[0] = static_cast<uint16_t>( 0xD800 + (dwUCS4 >> 10) - 0x40 );    // high 10 bits
            pwUTF16[1] = static_cast<uint16_t>( 0xDC00 + (dwUCS4 & 0x03FF) );        // low 10 bits
            return 2;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

// UCS2 to UCS4
size_t CodeConverter::UTF16ToUCS4(const uint16_t* pwUTF16, uint32_t& dwUCS4) {
    uint16_t    w1, w2;
    if (pwUTF16 == NULL) {
        return 0;
    }

    w1 = pwUTF16[0];
    if (w1 >= 0xD800 && w1 <= 0xDFFF) {
        if (w1 < 0xDC00) {
            w2 = pwUTF16[1];
            if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
                dwUCS4 = (w2 & 0x03FF) + (((w1 & 0x03FF) + 0x40) << 10);
                return 2;
            }
        }
        return 0;
    } else {
        dwUCS4 = w1;
        return 1;
    }
}

// UTF8 to UTF16
size_t CodeConverter::UTF8StrToUTF16Str(const char* pbszUTF8Str, uint16_t* pwszUTF16Str, size_t pwszUTF16Str_len) {
    size_t        iNum;
    size_t        iLen;
    uint32_t    dwUCS4;
    const char*         endUTF8Str;
    size_t        length = strlen(pbszUTF8Str);

    if (pbszUTF8Str == NULL || length == 0 || pwszUTF16Str == NULL) {
        return 0;
    }

    endUTF8Str = pbszUTF8Str + length;
    iNum = 0;
    // UTF8 to UCS4
    while (*pbszUTF8Str  && pbszUTF8Str < endUTF8Str) {
        dwUCS4 = UTF8ToUCS4(pbszUTF8Str, endUTF8Str, &iLen);
        if (iLen == 0) {
            return 0;
        }
        pbszUTF8Str += iLen;

        // UCS4 to UTF16
        iLen = UCS4ToUTF16(dwUCS4, pwszUTF16Str, pwszUTF16Str_len - iNum);
        if (iLen == 0) {
            return 0;
        }

        if (iNum + iLen < pwszUTF16Str_len) {
            pwszUTF16Str += iLen;
        } else {
            return 0;
        }
        iNum += iLen;
    }

    *pwszUTF16Str = 0;

    return iNum;
}

// UTF16 to UTF8
size_t  CodeConverter::UTF16StrToUTF8Str (const uint16_t* pwszUTF16Str, char* pbszUTF8Str) {
    size_t      iNum, iLen;
    uint32_t    dwUCS4;

    if (pwszUTF16Str == NULL) {
        return 0;
    }

    iNum = 0;
    // UTF16 to UCS4
    while (*pwszUTF16Str) {
        iLen = UTF16ToUCS4(pwszUTF16Str, dwUCS4);
        if (iLen == 0) {
            return 0;
        }

        pwszUTF16Str += iLen;

        // UCS4 to UTF8
        iLen = UCS4ToUTF8(dwUCS4, pbszUTF8Str);
        if (iLen == 0) {
            return 0;
        }

        if (pbszUTF8Str != NULL) {
            pbszUTF8Str += iLen;
        }
        iNum += iLen;
    }

    if (pbszUTF8Str != NULL) {
        *pbszUTF8Str = 0;
    }
    return iNum;
}

// utf-8 substring
void CodeConverter::SubString(const string &input, const size_t start, const size_t length, string *result) {
    size_t l = start;
    const char *begin = input.data();
    const char *end = begin + input.size();
    while (l > 0) {
        begin += CodeConverter::GetUTF8CharLength(begin);
        --l;
    }

    l = length;
    while (l > 0 && begin < end) {
        const size_t len = CodeConverter::GetUTF8CharLength(begin);
        result->append(begin, len);
        begin += len;
        --l;
    }

    return;
}

// utf-8 substring
void CodeConverter::SubString(const string &input, const size_t start, string *result) {
    size_t l = start;
    const char *begin = input.data();
    const char *end = begin + input.size();
    while (l > 0) {
        begin += CodeConverter::GetUTF8CharLength(begin);
        --l;
    }

    while (begin < end) {
        const size_t len = CodeConverter::GetUTF8CharLength(begin);
        result->append(begin, len);
        begin += len;
    }

    return;
}

// utf-8 substring
string CodeConverter::SubString(const string &input, const size_t start, const size_t length) {
    string result = "";
    SubString(input, start, length, &result);
    return result;
}

// utf-8 substring
string CodeConverter::SubString(const string &input, const size_t start) {
    string result = "";
    SubString(input, start, &result);
    return result;
}

// trim
string CodeConverter::Trim(const string &input) {
    if (input.empty()) {
        return "";
    }

    string res("");
    size_t begin = 0;
    for (; begin < input.size(); ++begin) {
        if (input[begin] == '\r' || input[begin] == '\n' ||
            input[begin] == '\t' || input[begin] == ' ') {
        } else {
            break;
        }
    }
    size_t end = input.size() - 1;
    for (; end > begin; --end) {
        if (input[end] == '\r' || input[end] == '\n' ||
            input[end] == '\t' || input[end] == ' ') {
        } else {
            break;
        }
    }
    if (end >= begin) {
        res.assign(input.begin() + begin, input.begin() + end + 1);
    }
    return res;
}

// convert utf8 str to ucs4 list
void CodeConverter::UTF8ToUCS4List(const string &input, vector<uint32_t> &output) {
    const char *begin = input.data();
    const char *end = begin + input.size();

    output.clear();
    while (begin < end) {
        size_t mblen = 0;
        output.push_back(UTF8ToUCS4(begin, end, &mblen));
        begin += mblen;
    }
}

// convert ucs4 to utf8 str
void CodeConverter::UCS4ToUTF8Append(uint32_t c, string *output) {
    char buf[7];
    CodeConverter::UCS4ToUTF8(c, buf);
    *output += buf;
}

// convert ucs4 list to utf8 str
void CodeConverter::UCS4ListToUTF8Append(const vector<uint32_t> &array, string *output) {
    for (size_t i = 0; i < array.size(); ++i) {
        UCS4ToUTF8Append(array[i], output);
    }
}

// convert wide string to multibyte string
string CodeConverter::WideToUTF8(const wchar_t *input) {
    UTF8AutoScopeLocale utf8asl;
    size_t buffer_length = wcstombs_r(NULL, input, 0);
    char *buffer = new char[buffer_length + 1];
    wcstombs_r(buffer, input, buffer_length + 1);
    string output = buffer;
    delete [] buffer;

    return output;
}

// convert wide string to multibyte string
string CodeConverter::WideToUTF8(const wstring &input) {
    return WideToUTF8(input.c_str());
}

// convert multibyte string to wide string
wstring CodeConverter::UTF8ToWide(const char *input) {
    UTF8AutoScopeLocale utf8asl;
    size_t buffer_length = mbstowcs_r(NULL, input, 0);
    wchar_t *buffer = new wchar_t[buffer_length + 1];
    mbstowcs_r(buffer, input, buffer_length + 1);
    wstring output = wstring(buffer);
    delete [] buffer;

    return output;
}

// convert multibyte string to wide string
wstring CodeConverter::UTF8ToWide(const string &input) {
    return UTF8ToWide(input.c_str());
}

string CodeConverter::WStringToGBKString(const wstring &w_str) {
    GBKAutoScopeLocale asl;
    size_t buffer_length = wcstombs_r(NULL, w_str.c_str(), 0);
    char *buffer = new char[buffer_length + 1];
    wcstombs_r(buffer, w_str.c_str(), buffer_length + 1);
    string str = buffer;
    delete [] buffer;

    return str;
}

wstring CodeConverter::GBKStringToWString(const string &str) {
    GBKAutoScopeLocale asl;
    size_t w_term_length = mbstowcs_r(NULL, str.c_str(), 0);
    wchar_t *w_term = new wchar_t[w_term_length+1];
    mbstowcs_r(w_term, str.c_str(), w_term_length + 1);
    wstring w_str = wstring(w_term);
    delete [] w_term;

    return w_str;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
