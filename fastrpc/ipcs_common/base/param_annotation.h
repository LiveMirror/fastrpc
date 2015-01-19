// This defines a set of argument wrappers and related factory methods that
// can be used specify the refcounting and reference semantics of arguments
// that are bound by the Bind() function in base/bind.h.
//
// The public functions are base::Unretained() and base::ConstRef().
// Unretained() allows Bind() to bind a non-refcounted class.
// ConstRef() allows binding a constant reference to an argument rather
// than a copy.
//
//
// EXAMPLE OF Unretained():
//
//     class Foo {
//     public:
//         void func() { cout << "Foo:f" << endl;
//     };
//
//     // In some function somewhere.
//     Foo foo;
//     Function<void(void)> foo_function =
//         Bind(&Foo::func, Unretained(&foo));
//     foo_function.Run();  // Prints "Foo:f".
//
// Without the Unretained() wrapper on |&foo|, the above call would fail
// to compile because Foo does not support the AddRef() and Release() methods.
//
//
// EXAMPLE OF ConstRef();
//     void foo(int arg) { cout << arg << endl }
//
//     int n = 1;
//     Function<void(void)> no_ref = Bind(&foo, n);
//     Function<void(void)> has_ref = Bind(&foo, ConstRef(n));
//
//     no_ref.Run();  // Prints "1"
//     has_ref.Run();  // Prints "1"
//
//     n = 2;
//     no_ref.Run();  // Prints "1"
//     has_ref.Run();  // Prints "2"
//
// Note that because ConstRef() takes a reference on |n|, |n| must outlive all
// its bound functions.

#ifndef IPCS_COMMON_BASE_FUNCTION_PARAM_ANNOTATION_H
#define IPCS_COMMON_BASE_FUNCTION_PARAM_ANNOTATION_H
#pragma once

#include <stddef.h>
#include "param_check.h"
#include "type_traits.h"

namespace common
{

namespace internal
{

template <typename T>
class UnretainedWrapper
{
public:
    explicit UnretainedWrapper(T* o) : m_obj(o) {}
    T* Get() { return m_obj; }
private:
    T* m_obj;
};

template <typename T>
class ConstRefWrapper
{
public:
    explicit ConstRefWrapper(const T& o) : m_ptr(&o) {}
    const T& Get() { return *m_ptr; }
private:
    const T* m_ptr;
};

// Unwrap the stored parameters for the wrappers above.
template <typename T>
T Unwrap(T o) { return o; }

template <typename T>
T* Unwrap(UnretainedWrapper<T> unretained) { return unretained.Get(); }

template <typename T>
const T& Unwrap(ConstRefWrapper<T> const_ref)
{
    return const_ref.Get();
}

// Utility for handling different refcounting semantics in the Bind()
// function.
template <typename ref, typename T>
struct MaybeRefCount;

template <typename T>
struct MaybeRefCount<std::false_type, T>
{
    static void AddRef(const T&) {}
    static void Release(const T&) {}
};

template <typename T, size_t n>
struct MaybeRefCount<std::false_type, T[n]>
{
    static void AddRef(const T*) {}
    static void Release(const T*) {}
};

template <typename T>
struct MaybeRefCount<std::true_type, T>
{
    static void AddRef(const T& o) {}
    static void Release(const T& o) {}
};

template <typename T>
struct MaybeRefCount<std::true_type, UnretainedWrapper<T> >
{
    static void AddRef(const UnretainedWrapper<T>&) {}
    static void Release(const UnretainedWrapper<T>&) {}
};

template <bool>
struct RefCountManager
{
    template <typename T>
    static void AddRef(const T* o) { o->AddRef(); }
    template <typename T>
    static void Release(const T* o) { o->Release(); }
};

template <>
struct RefCountManager<false>
{
    template <typename T>
    static void AddRef(const T* o) {}
    template <typename T>
    static void Release(const T* o) {}
};

template <typename T>
struct MaybeRefCount<std::true_type, T*>
{
    static void AddRef(T* o) { RefCountManager<IsRefCountedType<T*>::value>::AddRef(o); }
    static void Release(T* o) { RefCountManager<IsRefCountedType<T*>::value>::Release(o); }
};

template <typename T>
struct MaybeRefCount<std::true_type, const T*>
{
    static void AddRef(const T* o) { RefCountManager<IsRefCountedType<T*>::value>::AddRef(o); }
    static void Release(const T* o) { RefCountManager<IsRefCountedType<T*>::value>::Release(o); }
};

}  // namespace internal

template <typename T>
inline internal::UnretainedWrapper<T> Unretained(T* o)
{
    return internal::UnretainedWrapper<T>(o);
}

template <typename T>
inline internal::ConstRefWrapper<T> ConstRef(const T& o)
{
    return internal::ConstRefWrapper<T>(o);
}

}  // namespace common

#endif // IPCS_COMMON_BASE_FUNCTION_PARAM_ANNOTATION_H
