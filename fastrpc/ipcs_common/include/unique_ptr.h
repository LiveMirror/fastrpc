// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: unique ptr

#ifndef IPCS_COMMON_BASE_UNIQUE_PTR_H
#define IPCS_COMMON_BASE_UNIQUE_PTR_H

#include <assert.h>
#include <stdio.h>
#include <algorithm>

#include "strict_bool.h"

/// delete pointer deleter, using delete
template <typename T>
struct default_delete
{
    void operator()(T* p)
    {
        delete p;
    }
public:
    typedef char T_must_be_complete_type[sizeof(T) > 0 ? 1 : -1];
};

/// default_delete, specify for array, using delete[]
template <typename T>
struct default_delete<T []>
{
    void operator()(T* p)
    {
        delete[] p;
    }
private: // disable polymorphism array delete
    typedef char T_must_be_complete_type[sizeof(T) > 0 ? 1 : -1];
    template <typename U>
    void operator()(U* p) const;
};

/// delete pointer deleter, using delete
template <>
struct default_delete<FILE>
{
    void operator()(FILE* p)
    {
        if (p)
            fclose(p);
    }
};

/// unique_ptr_base, put common members here
template <
    typename Type,
    typename Deleter
>
class unique_ptr_base
{
public:
    typedef Type element_type;
    typedef Deleter deleter_type;
    typedef Type* pointer;

protected:
    /// default ctor, null pointer initialized
    unique_ptr_base() : m_ptr()
    {
    }

    /// construct from plain pointer
    explicit unique_ptr_base(pointer ptr) : m_ptr(ptr)
    {
    }

    /// construct from plain pointer and deleter
    unique_ptr_base(pointer ptr, Deleter deleter):
        m_ptr(ptr), m_deleter(deleter)
    {
    }

    ~unique_ptr_base()
    {
        m_deleter(m_ptr);
#ifndef NDEBUG
        // set to NULL in debug mode to detect wild pointer error
        m_ptr = pointer(); // or 0xCFCFCFCFCFCFCFCF or 0xDeadBeefDeadBeef
#endif
    }
public:
    /// assign with plain pointer
    unique_ptr_base& operator=(pointer ptr)
    {
        reset(ptr);
        return *this;
    }

    /// swap with another unique_ptr
    void swap(unique_ptr_base& rhs)
    {
        std::swap(m_ptr, rhs.m_ptr);
        std::swap(m_deleter, rhs.m_deleter);
    }

    /// release control of stored pointer and return it
    pointer release()
    {
        pointer p = m_ptr;
        m_ptr = pointer();
        return p;
    }

    /// assigns ptr to the stored pointer
    void reset(pointer ptr = pointer())
    {
        if (m_ptr != ptr)
        {
            m_deleter(m_ptr);
            m_ptr = ptr;
        }
    }

    /// get stored pointer
    pointer get() const
    {
        return m_ptr;
    }

    /// get stored deleter
    Deleter& get_deleter()
    {
        return m_deleter;
    }

    /// get stored deleter
    const Deleter& get_deleter() const
    {
        return m_deleter;
    }

    /* explicit */
    operator strict_bool() const
    {
        return m_ptr != pointer();
    }

    bool operator!() const
    {
        return !m_ptr;
    }
private: // disable copy and assign
    unique_ptr_base(const unique_ptr_base& rhs);
    unique_ptr_base& operator=(const unique_ptr_base& rhs);

private:
    pointer m_ptr;        // stored pointer
    Deleter m_deleter;    // stored deleter
};

/// unique_ptr for single object
/// @tparam Type object type
/// @tparam Deleter deleter functor
template <
    typename Type,
    typename Deleter = default_delete<Type>
>
class unique_ptr: public unique_ptr_base<Type, Deleter>
{
    typedef unique_ptr_base<Type, Deleter> base;
public:
    typedef typename base::element_type element_type;
    typedef typename base::deleter_type deleter_type;
    typedef typename base::pointer pointer;

public:
    /// make null constructed pointer
    unique_ptr() : base()
    {
    }

    /// construct with a plain pointer
    explicit unique_ptr(pointer ptr) : base(ptr)
    {
    }

    /// construct with a plain pointer and a deleter
    unique_ptr(pointer ptr, Deleter deleter):
        base(ptr, deleter)
    {
    }

    /// assign with plain pointer
    unique_ptr& operator=(pointer ptr)
    {
        base::operator=(ptr);
        return *this;
    }

    /// overloaded operator *
    element_type& operator*() const
    {
        assert(base::get());
        return *base::get();
    }

    /// overloaded operator ->
    typename base::pointer operator->() const
    {
        assert(base::get());
        return base::get();
    }
};

/// unique_ptr for dynamic allocated array
/// @tparam Type object type
/// @tparam Deleter deleter functor
template <
    typename Type,
    typename Deleter
>
class unique_ptr<Type[], Deleter> : public unique_ptr_base<Type, Deleter>
{
    typedef unique_ptr_base<Type, Deleter> base;
public:
    typedef typename base::element_type element_type;
    typedef typename base::deleter_type deleter_type;
    typedef typename base::pointer pointer;

public:
    /// make null constructed pointer
    unique_ptr() { }

    /// construct with a plain pointer
    explicit unique_ptr(pointer ptr) : base(ptr) { }

    /// construct with a plain pointer and a deleter
    unique_ptr(pointer ptr, Deleter deleter):
        base(ptr, deleter)
    {
    }
private: // disable polymorphism array
    template <typename Type2>
    explicit unique_ptr(Type2 ptr);

    template <typename Type2>
    unique_ptr(Type2 ptr, Deleter deleter);

public:
    /// assign with plain pointer
    unique_ptr& operator=(pointer ptr)
    {
        base::operator=(ptr);
        return *this;
    }

    /// overloaded operator[] for array
    element_type& operator[](size_t index) const
    {
        return base::get()[index];
    }

    /// swap with another unique_ptr
    void swap(unique_ptr& rhs)
    {
        base::swap(rhs);
    }
};

namespace std
{

/// specify swap for unique_ptr
template <class Type, class Deleter>
void swap(unique_ptr<Type, Deleter>& lhs, unique_ptr<Type, Deleter>& rhs)
{
    lhs.swap(rhs);
}

} // namespace std

// } // namespace common

// namespace common {

template <
    typename Type,
    typename Deleter = default_delete<Type[]>
>
class unique_array
{
    typedef unique_ptr<Type[], Deleter> unique_ptr_type;
public:
    typedef typename unique_ptr_type::element_type element_type;
    typedef typename unique_ptr_type::deleter_type deleter_type;
    typedef typename unique_ptr_type::pointer pointer;
public:
    unique_array(pointer ptr, size_t array_size):
        m_ptr(ptr), m_size(array_size)
    {
    }
    size_t size() const
    {
        return m_size;
    }
    element_type& operator[](size_t index)
    {
        assert(index < m_size);
        return m_ptr[index];
    }
    const element_type& operator[](size_t index) const
    {
        assert(index < m_size);
        return m_ptr[index];
    }
private:
    unique_ptr<Type[], Deleter> m_ptr;
    size_t m_size;
};

#endif // IPCS_COMMON_BASE_UNIQUE_PTR_H
