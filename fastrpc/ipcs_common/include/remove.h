// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: remove

// GLOBAL_NOLINT(whitespace/newline)
// GLOBAL_NOLINT(readability/braces)

#ifndef IPCS_COMMON_BASE_TYPE_TRAITS_REMOVE_H
#define IPCS_COMMON_BASE_TYPE_TRAITS_REMOVE_H
#pragma once

namespace common {
namespace type_traits {

// const-volatile modifications.

/// remove_const
template<typename _Tp>
struct remove_const
{ typedef _Tp     type; };

template<typename _Tp>
struct remove_const<_Tp const>
{ typedef _Tp     type; };

/// remove_volatile
template<typename _Tp>
struct remove_volatile
{ typedef _Tp     type; };

template<typename _Tp>
struct remove_volatile<_Tp volatile>
{ typedef _Tp     type; };

/// remove_cv
template<typename _Tp>
struct remove_cv
{
    typedef typename
        remove_const<typename remove_volatile<_Tp>::type>::type     type;
};

} // namespace type_traits
} // namespace common

#endif // IPCS_COMMON_BASE_TYPE_TRAITS_REMOVE_H
