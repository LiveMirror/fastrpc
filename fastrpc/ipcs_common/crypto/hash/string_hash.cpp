// Copyright (c) 2012, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 12/29/12
// Description: string hash imp

#include <string.h>
#include "string_hash.h"

namespace ipcs_common {

unsigned int ELFhash(const char* url, unsigned int hash_size)
{
    unsigned int h = 0;

    while (*url)
    {
        h = (h << 4) + *url++;
        unsigned int g = h & 0xF0000000;

        if (g)
            h ^= g >> 24;

        h &= ~g;
    }

    return h % hash_size;
}

unsigned int HfIp(const char* url, unsigned int hash_size)
{
    unsigned int n = 0;
    unsigned int url_length = strlen(url);
    unsigned char* b = (unsigned char*)&n;

    for (unsigned int i = 0; i < url_length; i++)
        b[i%4] ^= url[i];

    return n % hash_size;
}

unsigned int hf(const char* url, unsigned int hash_size)
{
    int result = 0;
    const char* ptr = url;
    int c;

    for (int i = 1; (c = *ptr++); i++)
        result += c * 3 * i;

    if (result < 0)
        result = -result;

    return result % hash_size;
}

} // namespace ipcs_common
