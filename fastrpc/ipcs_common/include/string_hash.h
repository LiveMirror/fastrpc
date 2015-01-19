// Copyright (c) 2012, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 12/29/12
// Description: sring hash library

#ifndef IPCS_COMMON_CRYPTO_HASH_STRING_HASH_H
#define IPCS_COMMON_CRYPTO_HASH_STRING_HASH_H
#pragma once

namespace ipcs_common {

unsigned int HfIp(const char* url, unsigned int hash_size);
unsigned int hf(const char* url, unsigned int hash_size);

// System - v hash method, it is nice.
unsigned int ELFhash(const char* url, unsigned int hash_size);

} // ipcs_common

#endif // IPCS_COMMON_CRYPTO_HASH_STRING_HASH_H
