// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: type traits

#ifndef IPCS_COMMON_BASE_TYPE_TRAITS_H
#define IPCS_COMMON_BASE_TYPE_TRAITS_H
#pragma once

#include <features.h>

#if __GNUC_PREREQ(4, 1)
#define COMMON_HAS_STD_TR1_TYPE_TRAITS
#endif

#if __GNUC_PREREQ(4, 5) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#define COMMON_HAS_STD_TYPE_TRAITS
#endif

#if defined COMMON_HAS_STD_TYPE_TRAITS
#include "std.h"
#elif defined COMMON_HAS_STD_TR1_TYPE_TRAITS
#include "tr1.h"
#else
#include "missing.h"
#endif

#undef COMMON_HAS_STD_TYPE_TRAITS
#undef COMMON_HAS_STD_TR1_TYPE_TRAITS

#endif // IPCS_COMMON_BASE_TYPE_TRAITS_H
