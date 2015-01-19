/// Jack
/// Description: global initialize

#ifndef IPCS_COMMON_BASE_GLOBAL_INITIALIZE_H
#define IPCS_COMMON_BASE_GLOBAL_INITIALIZE_H

/// global initialize, run some code before main
/// @param name module name
#define GLOBAL_INITIALIZE(name) \
static void name##_init(); \
static bool call_global_##name##_init() { \
    name##_init(); \
    return true; \
} \
bool name##_initialized = call_global_##name##_init(); \
static void name##_init()

#if 0

// example
GLOBAL_INITIALIZE(this_test) {
    FLAGS_gtest_death_test_style = "threadsafe";
    print("test initialized");
}

#endif

#endif // IPCS_COMMON_BASE_GLOBAL_INITIALIZE_H
