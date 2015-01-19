// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: enable if
// GLOBAL_NOLINT(whitespace/newline)

#ifndef IPCS_COMMON_BASE_TYPE_TRAITS_ENABLE_IF_H
#define IPCS_COMMON_BASE_TYPE_TRAITS_ENABLE_IF_H
#pragma once

namespace common {
namespace type_traits {

// Define a nested type if some predicate holds.
// Primary template.
template<bool, typename _Tp = void> struct enable_if {};

// Partial specialization for true.
template<typename _Tp>
struct enable_if<true, _Tp> { typedef _Tp type; };

} // namespace type_traits
} // namespace common

#endif // IPCS_COMMON_BASE_TYPE_TRAITS_ENABLE_IF_H
