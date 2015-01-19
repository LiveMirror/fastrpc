// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: strict bool

#ifndef IPCS_COMMON_BASE_STRICT_BOOL_H
#define IPCS_COMMON_BASE_STRICT_BOOL_H
#pragma once

#include "static_assert.h"
#include "type_traits.h"

namespace common {

class strict_bool
{
    typedef bool (strict_bool::*SafeBool)() const;

public:
    strict_bool() : m_value(false) {}

    template <typename T>
    strict_bool(T src,
                typename std::enable_if<std::is_same<T, bool>::value>::type* dummy = 0) :
        m_value(src)
    {
    }

    strict_bool& operator=(bool src)
    {
        m_value = src;
        return *this;
    }

    // operator SafeBool() const { return m_value ? &strict_bool::operator! : 0; }
    template <typename T>
    operator T() const
    {
        typedef typename std::enable_if<std::is_same<T, bool>::value>::type type;
        return value();
    }

    bool operator!() const
    {
        return !m_value;
    }
    bool value() const
    {
        return m_value;
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, bool>::value, bool>::type
    operator==(T rhs)
    {
        return value() == rhs;
    }

    bool operator==(strict_bool rhs) const
    {
        return value() == rhs.value();
    }

    template <typename T>
    typename std::enable_if<std::is_same<T, bool>::value, bool>::type
    operator!=(T rhs)
    {
        return value() != rhs;
    }

    bool operator!=(strict_bool rhs) const
    {
        return value() != rhs.value();
    }

private:
    bool m_value;
};

template <typename T>
typename std::enable_if<std::is_same<T, bool>::value, bool>::type
operator==(T lhs, const strict_bool& rhs)
{
    return rhs.value() == lhs;
}

template <typename T>
typename std::enable_if<std::is_same<T, bool>::value, bool>::type
operator!=(T lhs, const strict_bool& rhs)
{
    return rhs.value() != lhs;
}

} // namespace common

using ::common::strict_bool;

#endif // IPCS_COMMON_BASE_STRICT_BOOL_H
