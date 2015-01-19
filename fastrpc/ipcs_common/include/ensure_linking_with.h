/// Jack
#ifndef IPCS_COMMON_BASE_ENSURE_LINKING_WITH_H
#define IPCS_COMMON_BASE_ENSURE_LINKING_WITH_H
#pragma once

namespace ipcs_common {

// Used for header files with whole inline/template implementation.
// Then make sure all code depends on it must add its library to the deps.
//
// If your design your library to be headers only, without a corresponding
// library, you will have no chance to add implementation depends without
// impacting your user.
//
// So, we strongly suggest you not writing header only libraries. To enforce
// the user must depends on your library, we introduce the following macros
// to make you achive this easily.
//
// Usage:
// template <typename T>
// class Foo : private ENSURE_LINKING_WITH(Foo) {
//     ...
// };
//
// or:
// void Foo() {
//     ASSERT_LINKING_WITH(Foo);
// }
//
// If the user forget to link with the library foo, the linker will issue
// following error message:
// undefined reference to `common::ensure_linking_with_the_library_contains_Foo_::definition()
// to prompt the user.
//
// There is no any runtime overhead for release building.

//////////////////////////////////////////////////////////////////////////////
// Implementation details.
#ifdef NDEBUG

#define DECLARE_ENSURE_LINKING_WITH_IMPL(name) \
class ensure_linking_with_the_library_contains_##name##_ { \
protected: \
    ensure_linking_with_the_library_contains_##name##_() {} \
    ~ensure_linking_with_the_library_contains_##name##_() {} \
public: \
    static void Assert() {} \
};

#define DEFINE_ENSURE_LINKING_WITH_IMPL(name)

#else // DEBUG

#define DECLARE_ENSURE_LINKING_WITH_IMPL(name) \
class ensure_linking_with_the_library_contains_##name##_ { \
protected: \
    ensure_linking_with_the_library_contains_##name##_() { Assert(); } \
    ~ensure_linking_with_the_library_contains_##name##_() {} \
public: \
    static void Assert() { definition(); } \
private: \
    static void definition(); \
};

#define DEFINE_ENSURE_LINKING_WITH_IMPL(name) \
void ensure_linking_with_the_library_contains_##name##_::definition() {}

#endif // NDEBUG

//////////////////////////////////////////////////////////////////////////////
// Interfaces

// Put this into you '.h' file before the interface definition.
#define DECLARE_ENSURE_LINKING_WITH(name) DECLARE_ENSURE_LINKING_WITH_IMPL(name)

// Put this into you '.cpp' file.
#define DEFINE_ENSURE_LINKING_WITH(name) DEFINE_ENSURE_LINKING_WITH_IMPL(name)

// Usage 1:
// As a private base class:
// class Foo : private ENSURE_LINKING_WITH(Foo) {
#define ENSURE_LINKING_WITH(name) ensure_linking_with_the_library_contains_##name##_

// Usage 2:
// Call in a function:
// void Foo() {
//      ASSERT_LINKING_WITH(Foo);
// }
#define ASSERT_LINKING_WITH(name) ENSURE_LINKING_WITH(name)::Assert()

} // namespace ipcs_common

#endif // IPCS_COMMON_BASE_ENSURE_LINKING_WITH_H
