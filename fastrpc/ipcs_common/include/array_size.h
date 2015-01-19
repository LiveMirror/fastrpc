// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: array size helper

#ifndef IPCS_COMMON_BASE_ARRAY_SIZE_H
#define IPCS_COMMON_BASE_ARRAY_SIZE_H
#pragma once

#define ARRAY_SIZE(a) \
    (sizeof(a) / sizeof(*(a))) / (size_t)(!(sizeof(a) % sizeof(*(a))))

#endif // IPCS_COMMON_BASE_ARRAY_SIZE_H
