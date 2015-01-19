/// Jack
#ifndef IPCS_COMMON_BASE_FUNCTION_PARAM_CHECK_H
#define IPCS_COMMON_BASE_FUNCTION_PARAM_CHECK_H
#pragma once

#include <stddef.h>
#include "type_traits.h"

namespace common
{

namespace internal
{

// Use the Substitution Failure Is Not An Error (SFINAE) trick to inspect T
// for the existence of AddRef() and Release() functions of the correct
// signature.
//
// http://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error
// http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence
// http://stackoverflow.com/questions/4358584/sfinae-approach-comparison
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
//
// The last link in particular show the method used below.
//
// For SFINAE to work with inherited methods, we need to pull some extra tricks
// with multiple inheritance.  In the more standard formulation, the overloads
// of Check would be:
//
//   template <typename C>
//   Yes NotTheCheckWeWant(Helper<&C::TargetFunc>*);
//
//   template <typename C>
//   No NotTheCheckWeWant(...);
//
//   static const bool value = sizeof(NotTheCheckWeWant<T>(0)) == sizeof(Yes);
//
// The problem here is that template resolution will not match
// C::TargetFunc if TargetFunc does not exist directly in C.  That is, if
// TargetFunc in inherited from an ancestor, &C::TargetFunc will not match,
// |value| will be false.  This formulation only checks for whether or
// not TargetFunc exist directly in the class being introspected.
//
// To get around this, we play a dirty trick with multiple inheritance.
// First, We create a class BaseMixin that declares each function that we
// want to probe for.  Then we create a class Base that inherits from both T
// (the class we wish to probe) and BaseMixin.  Note that the function
// signature in BaseMixin does not need to match the signature of the function
// we are probing for; thus it's easiest to just use void(void).
//
// Now, if TargetFunc exists somewhere in T, then &Base::TargetFunc has an
// ambiguous resolution between BaseMixin and T.  This lets us write the
// following:
//
//   template <typename C>
//   No GoodCheck(Helper<&C::TargetFunc>*);
//
//   template <typename C>
//   Yes GoodCheck(...);
//
//   static const bool value = sizeof(GoodCheck<Base>(0)) == sizeof(Yes);
//
// Notice here that the variadic version of GoodCheck() returns Yes here
// instead of No like the previous one. Also notice that we calculate |value|
// by specializing GoodCheck() on Base instead of T.
//
// We've reversed the roles of the variadic, and Helper overloads.
// GoodCheck(Helper<&C::TargetFunc>*), when C = Base, fails to be a valid
// substitution if T::TargetFunc exists. Thus GoodCheck<Base>(0) will resolve
// to the variadic version if T has TargetFunc.  If T::TargetFunc does not
// exist, then &C::TargetFunc is not ambiguous, and the overload resolution
// will prefer GoodCheck(Helper<&C::TargetFunc>*).
//
// This method of SFINAE will correctly probe for inherited names, but it cannot
// typecheck those names.  It's still a good enough sanity check though.
//
// Works on gcc-4.2, gcc-4.4, and Visual Studio 2008.
//
// TODO(ajwong): Move to ref_counted.h or template_util.h when we've vetted
// this works well.
template <typename T>
class SupportsAddRefAndRelease
{
    typedef char Yes[1];
    typedef char No[2];

    struct BaseMixin
    {
        void AddRef();
        void Release();
    };

    struct Base : public T, public BaseMixin
    {
    };

    template <void(BaseMixin::*)(void)>  struct Helper {};

    template <typename C>
    static No& CheckAddRef(Helper<&C::AddRef>*);

    template <typename >
    static Yes& CheckAddRef(...);

    template <typename C>
    static No& CheckRelease(Helper<&C::Release>*);

    template <typename >
    static Yes& CheckRelease(...);

public:
    static const bool Value = sizeof(CheckAddRef<Base>(0)) == sizeof(Yes) &&
                              sizeof(CheckRelease<Base>(0)) == sizeof(Yes);
};

// Helpers to assert that arguments of a recounted type are bound with a
// scoped_refptr.
template <bool IsClassType, typename T>
struct IsRefCountedTypeHelper : std::false_type
{
};

template <typename T>
struct IsRefCountedTypeHelper<true, T>
        : std::integral_constant<bool, SupportsAddRefAndRelease<T>::Value>
{
};

template <typename T>
struct IsRefCountedType : std::false_type
{
};

template <typename T>
struct IsRefCountedType<T*>
        : IsRefCountedTypeHelper<std::is_class<T>::value, T>
{
};

} // namespace internal

} // namespace common

#endif // IPCS_COMMON_BASE_FUNCTION_PARAM_CHECK_H
