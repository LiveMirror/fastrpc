// Defines several base class and registers for testing. We intentionally
// define them in a separate file as some compilers don't correctly support to
// define static variable in inline function, they create a separate copy each
// time it's included. We want to make sure it doesn't happen to us.

#ifndef IPCS_COMMON_BASE_CLASS_REGISTER_TEST_HELPER_H
#define IPCS_COMMON_BASE_CLASS_REGISTER_TEST_HELPER_H

#include <string>
#include "class_register.h"

class Mapper {
public:
    Mapper() {}
    virtual ~Mapper() {}

    virtual std::string GetMapperName() const = 0;
};

CLASS_REGISTER_DEFINE_REGISTRY(mapper_register, Mapper);

#define REGISTER_MAPPER(mapper_name) \
    CLASS_REGISTER_OBJECT_CREATOR( \
        mapper_register, Mapper, #mapper_name, mapper_name)

#define CREATE_MAPPER(mapper_name_as_string) \
    CLASS_REGISTER_CREATE_OBJECT(mapper_register, mapper_name_as_string)

CLASS_REGISTER_DEFINE_REGISTRY(second_mapper_register, Mapper);

#define REGISTER_SECONDARY_MAPPER(mapper_name) \
    CLASS_REGISTER_OBJECT_CREATOR( \
        second_mapper_register, Mapper, #mapper_name, mapper_name)

#define CREATE_SECONDARY_MAPPER(mapper_name_as_string) \
    CLASS_REGISTER_CREATE_OBJECT(second_mapper_register, \
                                 mapper_name_as_string)

class Reducer {
public:
    Reducer() {}
    virtual ~Reducer() {}

    virtual std::string GetReducerName() const = 0;
};

CLASS_REGISTER_DEFINE_REGISTRY(reducer_register, Reducer);

#define REGISTER_REDUCER(reducer_name) \
    CLASS_REGISTER_OBJECT_CREATOR( \
        reducer_register, Reducer, #reducer_name, reducer_name)

#define CREATE_REDUCER(reducer_name_as_string) \
    CLASS_REGISTER_CREATE_OBJECT(reducer_register, reducer_name_as_string)

class FileImpl {
public:
    FileImpl() {}
    virtual ~FileImpl() {}

    virtual std::string GetFileImplName() const = 0;
};

CLASS_REGISTER_DEFINE_REGISTRY(file_impl_register, FileImpl);

#define REGISTER_FILE_IMPL(path_prefix_as_string, file_impl_name) \
    CLASS_REGISTER_OBJECT_CREATOR_WITH_SINGLETON( \
        file_impl_register, FileImpl, path_prefix_as_string, file_impl_name)

#define CREATE_FILE_IMPL(path_prefix_as_string) \
    CLASS_REGISTER_CREATE_OBJECT(file_impl_register, path_prefix_as_string)

#define GET_FILE_IMPL_SINGLETON(path_prefix_as_string) \
    CLASS_REGISTER_GET_SINGLETON(file_impl_register, path_prefix_as_string)

#define FILE_IMPL_COUNT() \
    CLASS_REGISTER_CREATOR_COUNT(file_impl_register)

#define FILE_IMPL_NAME(i) \
    CLASS_REGISTER_CREATOR_NAME(file_impl_register, i)

#endif  // IPCS_COMMON_BASE_CLASS_REGISTER_TEST_HELPER_H
