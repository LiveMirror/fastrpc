// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_TIME_H
#define IPCS_COMMON_NET_HTTP_HTTP_TIME_H

#include <time.h>
#include <string>

bool ParseHttpTime(const char* str, time_t* time);
inline bool ParseHttpTime(const std::string& str, time_t* time)
{
    return ParseHttpTime(str.c_str(), time);
}

size_t FormatHttpTime(time_t time, char* str, size_t str_length);
bool FormatHttpTime(time_t time, std::string* str);

#endif // IPCS_COMMON_NET_HTTP_HTTP_TIME_H
