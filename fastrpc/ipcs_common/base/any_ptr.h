// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: any ptr

#ifndef IPCS_COMMON_BASE_ANY_PTR_H
#define IPCS_COMMON_BASE_ANY_PTR_H

#include <assert.h>
#include <stddef.h>
#include <stdexcept>
#include <typeinfo>

namespace ipcs_common {

/// type safed generic pointer
class AnyPtr
{
public:
    AnyPtr():
        m_Ptr(NULL),
        m_TypeInfo(&typeid(void*)),
        m_deleter(NULL)
    {
    }

    template <typename T>
    AnyPtr(T* ptr): // NOLINT(runtime/explicit)
        m_Ptr(ptr),
        m_TypeInfo(&typeid(T*)),
        m_deleter(&AnyPtr::Deleter<T>)
    {
    }

    template <typename T>
    AnyPtr& operator=(T* ptr)
    {
        m_Ptr = ptr;
        m_TypeInfo = &typeid(T*);
        m_deleter = &AnyPtr::Deleter<T>;
        return *this;
    }

    /// if mismatch, return NULL, and report error in debug mode
    template <typename T>
    T CastTo() const
    {
        (void) sizeof(*T()); // ensure T is a pointer type

        if (typeid(T) == *m_TypeInfo)
        {
            return reinterpret_cast<T>(m_Ptr);
        }
        assert(!"type mismatch");
        return NULL;
    }

    /// if mismatch, report error
    template <typename T>
    T CheckedCastTo() const
    {
        (void) sizeof(*T()); // ensure T is a pointer type

        if (typeid(T) == *m_TypeInfo)
        {
            return reinterpret_cast<T>(m_Ptr);
        }
        throw std::invalid_argument("Invalid type in Any::CheckedCast");
    }

    /// if mismatch, return NULL
    template <typename T>
    T TryCastTo() const
    {
        (void) sizeof(*T()); // ensure T is a pointer type

        if (typeid(T) == *m_TypeInfo)
        {
            return reinterpret_cast<T>(m_Ptr);
        }
        return NULL;
    }

    bool IsNull() const
    {
        return m_Ptr == NULL;
    }

    /// delete operation is type-safed
    /// must confirm the m_Ptr is deletable
    void Delete()
    {
        if (m_deleter && m_Ptr)
        {
            m_deleter(m_Ptr);
            m_Ptr = NULL;
            m_deleter = NULL;
            m_TypeInfo = NULL;
        }
    }
private:
    /// Type specific deleter
    template <typename T>
    static void Deleter(void* p)
    {
        delete static_cast<T*>(p);
    }
private:
    void* m_Ptr;
    const std::type_info* m_TypeInfo;
    void (*m_deleter)(void* p);
};

} // namespace ipcs_common

#endif // IPCS_COMMON_BASE_ANY_PTR_H
