/// Jack
#ifndef IPCS_COMMON_BASE_FUNCTION_PARAM_TRAITS_H
#define IPCS_COMMON_BASE_FUNCTION_PARAM_TRAITS_H
#pragma once

#include <stddef.h>

namespace common
{
namespace internal
{

// This is a typetraits object that's used to take an argument type, and
// extract a suitable type for storing and forwarding arguments.
//
// In particular, it strips off references, and converts arrays to
// pointers for storage; and it avoids accidentally trying to create a
// "reference of a reference" if the argument is a reference type.
//
// This array type becomes an issue for storage because we are passing bound
// parameters by const reference. In this case, we end up passing an actual
// array type in the initializer list which C++ does not allow.  This will
// break passing of C-string literals.
template <typename T>
struct ParamTraits
{
    typedef const T& ForwardType;
    typedef T StorageType;
};

// The Storage should almost be impossible to trigger unless someone manually
// specifies type of the bind parameters.  However, in case they do,
// this will guard against us accidentally storing a reference parameter.
//
// The ForwardType should only be used for unbound arguments.
template <typename T>
struct ParamTraits<T&>
{
    typedef T& ForwardType;
    typedef T StorageType;
};

// Note that for array types, we implicitly add a const in the conversion. This
// means that it is not possible to bind array arguments to functions that take
// a non-const pointer. Trying to specialize the template based on a "const
// T[n]" does not seem to match correctly, so we are stuck with this
// restriction.
template <typename T, size_t n>
struct ParamTraits<T[n]>
{
    typedef const T* ForwardType;
    typedef const T* StorageType;
};

// See comment for ParamTraits<T[n]>.
template <typename T>
struct ParamTraits<T[]>
{
    typedef const T* ForwardType;
    typedef const T* StorageType;
};

}  // namespace internal
}  // namespace common

#endif // IPCS_COMMON_BASE_FUNCTION_PARAM_TRAITS_H
