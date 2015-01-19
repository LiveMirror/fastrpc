/// Jack
#ifndef IPCS_COMMON_BASE_REF_COUNTED_H
#define IPCS_COMMON_BASE_REF_COUNTED_H
#pragma once

#include <stddef.h>
#include "uncopyable.h"

namespace common
{

class IRefCounted
{
public:
    /// return true iif ref count is 1
    bool IsUnique() const;
    int GetRefCount() const;

protected:
    IRefCounted();
    explicit IRefCounted(int base_ref_count);
    ~IRefCounted();

    /// return new ref count
    int AddRef() const;

    /// Returns true if the object should self-delete.
    bool Release() const;

private:
    mutable int m_ref_count;
    DECLARE_UNCOPYABLE(IRefCounted);
};

// Forward declaration.
template <class T, typename Deleter> class RefCountedBase;

// Default traits for RefCountedBase<T>.  Deletes the object when its ref
// count reaches 0.  Overload to delete it on a different thread etc.
template<typename T>
struct DefaultRefCountedDeleter
{
    static void Delete(const T* x)
    {
        // Delete through RefCountedBase to make child classes only need to be
        // friend with RefCountedBase instead of this struct, which is an
        // implementation detail.
        RefCountedBase<T, DefaultRefCountedDeleter >::DeleteInternal(x);
    }
};

// A thread-safe RefCountedBase<T> base class template
//
//   class MyFoo : public base::RefCountedBase<MyFoo>
//   {
//      ...
//   };
//
// If you're using the default trait, then you should add compile time
// asserts that no one else is deleting your object.  i.e.
//     private:
//         friend class base::RefCountedBase<MyFoo>;
//         ~MyFoo();
template < class T, typename Deleter = DefaultRefCountedDeleter<T> >
class RefCountedBase : public IRefCounted
{
public:
    RefCountedBase() { }
    ~RefCountedBase() { }

    int AddRef() const
    {
        return IRefCounted::AddRef();
    }

    bool Release() const
    {
        bool result = IRefCounted::Release();
        if (result)
            Deleter::Delete(static_cast<const T*>(this));
        return result;
    }

private:
    friend struct DefaultRefCountedDeleter<T>;
    static void DeleteInternal(const T* x) { delete x; }
    DECLARE_UNCOPYABLE(RefCountedBase);
};

// A wrapper for some piece of data so we can place other things in
// scoped_refptrs<>.
template<typename T>
class RefCounted : public RefCountedBase< RefCounted<T> >
{
public:
    RefCounted() : value() {}
    RefCounted(const T& in_value) : // NOLINT(runtime/explicit)
        value(in_value)
    {
    }

public:
    // allow direct access to this member is better than makeing it private
    // and provide a group of trivia accessors
    T value;
};

}  // namespace common

using ::common::IRefCounted;
using ::common::RefCountedBase;
using ::common::RefCounted;

#endif // IPCS_COMMON_BASE_REF_COUNTED_H
