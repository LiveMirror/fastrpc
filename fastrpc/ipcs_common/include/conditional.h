// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: conditional
// GLOBAL_NOLINT(whitespace/newline)
// GLOBAL_NOLINT(readability/braces)

#ifndef IPCS_COMMON_BASE_TYPE_TRAITS_CONDITIONAL_H
#define IPCS_COMMON_BASE_TYPE_TRAITS_CONDITIONAL_H
#pragma once

namespace common {
namespace type_traits {

// Primary template.
/// Define a member typedef @c type to one of two argument types.
template<bool _Cond, typename _Iftrue, typename _Iffalse>
struct conditional { typedef _Iftrue type; };

// Partial specialization for false.
template<typename _Iftrue, typename _Iffalse>
struct conditional<false, _Iftrue, _Iffalse>
{ typedef _Iffalse type; };

} // namespace type_traits
} // namespace common

#endif // IPCS_COMMON_BASE_TYPE_TRAITS_CONDITIONAL_H
