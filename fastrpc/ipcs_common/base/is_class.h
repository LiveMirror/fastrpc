// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: is class
// GLOBAL_NOLINT(whitespace/newline)

#ifndef IPCS_COMMON_BASE_TYPE_TRAITS_IS_CLASS_H
#define IPCS_COMMON_BASE_TYPE_TRAITS_IS_CLASS_H
#pragma once

#include <tr1/type_traits>

namespace common {
namespace type_traits {

template<typename _Tp>
struct __is_class_helper
{
private:
    template<typename _Up>
    static char __test(int _Up::*);
    template<typename>
    static int __test(...);

public:
    static const bool __value = sizeof(__test<_Tp>(0)) == 1;
};

// Extension.
template<typename _Tp>
struct is_class
: public std::tr1::integral_constant<bool, __is_class_helper<_Tp>::__value>
{ };

} // namespace type_traits
} // namespace common

#endif // IPCS_COMMON_BASE_TYPE_TRAITS_IS_CLASS_H
