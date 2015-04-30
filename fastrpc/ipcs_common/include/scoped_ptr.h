// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 02/01/13
// Description: scoped_ptr details

#ifndef IPCS_COMMON_BASE_SCOPED_PTR_H
#define IPCS_COMMON_BASE_SCOPED_PTR_H
#pragma once

#include <assert.h>
#include <cstddef>
#include "uncopyable.h"

namespace ipcs_common
{

// A scoped_ptr looks like a normal ptr C*, it holds
// a pointer to the address of class C object and destructs
// itself automatically.
template <class C>
class scoped_ptr : private UnCopyable
{
public:
    explicit scoped_ptr(C* p = NULL) : m_p(p) {}

    virtual ~scoped_ptr()
    {
        enum { must_be_a_complete_type = sizeof(C) };
        delete m_p;
        m_p = reinterpret_cast<C*>(-1);
    }

    C& operator*() const
    {
        assert(m_p != NULL);
        return *m_p;
    }

    C* operator->() const
    {
        assert(m_p != NULL);
        return m_p;
    }

    C* get() const
    {
        return m_p;
    }

    void reset(C* p)
    {
        if (p != m_p) {
            enum { must_be_a_complete_type = sizeof(C) };
            delete m_p;
            m_p = p;
        }
    }

    C* release()
    {
        C* tmp = m_p;
        m_p = NULL;
        return tmp;
    }

    void swap(scoped_ptr& p)
    {
        C* tmp = m_p;
        m_p = p.m_p;
        p.m_p = tmp;
    }

    operator bool() const
    {
        return m_p != NULL;
    }

    bool operator!() const
    {
        return m_p == NULL;
    }

    bool operator==(const scoped_ptr& p) const
    {
        return m_p == p.get();
    }

    bool operator==(const C* p) const
    {
        return m_p == p;
    }

    bool operator!=(const scoped_ptr& p) const
    {
        return m_p != p.get();
    }

    bool operator!=(const C* p) const
    {
        return m_p != p;
    }

private:
    template <class NotC>
    bool operator==(const scoped_ptr<NotC>& p) const;

    template <class NotC>
    bool operator!=(const scoped_ptr<NotC>& p) const;

private:
    C* m_p;
};

// scoped_array which holds an array allocated by new[]
template <class C>
class scoped_array : private UnCopyable
{
public:
    explicit scoped_array(C* array = NULL) : m_array(array) {}

    virtual ~scoped_array()
    {
        enum { must_be_a_complete_type = sizeof(C) };
        delete[] m_array;
        m_array = reinterpret_cast<C*>(-1);
    }

    C& operator[](std::ptrdiff_t d) const
    {
        assert(m_array != NULL);
        assert(d >= 0);
        return m_array[d];
    }

    void reset(C* array)
    {
        if (m_array != array) {
            enum { must_be_a_complete_type = sizeof(C) };
            delete[] m_array;
            m_array = array;
        }
    }

    C* release()
    {
        C* tmp = m_array;
        m_array = NULL;
        return tmp;
    }

    C* get() const
    {
        return m_array;
    }

    void swap(scoped_array& array)
    {
        C* tmp = m_array;
        m_array = array.m_array;
        array.m_array = tmp;
    }

    operator bool() const
    {
        return m_array != NULL;
    }

    bool operator!() const
    {
        return m_array == NULL;
    }

    bool operator==(const scoped_array& array) const
    {
        return m_array == array.get();
    }

    bool operator==(const C* array) const
    {
        return m_array == array;
    }

    bool operator!=(const scoped_array& array) const
    {
        return m_array != array.get();
    }

    bool operator!=(const C* array) const
    {
        return m_array != array;
    }

private:
    template <class NotC>
    bool operator==(const scoped_array& array) const;

    template <class NotC>
    bool operator!=(const scoped_array& array) const;

private:
    C* m_array;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_BASE_SCOPED_PTR_H
