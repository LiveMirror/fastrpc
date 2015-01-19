# ipcs_common/base/closure module
# Author: feimat@qq.com
# Date: 01/28/2013
cc_library(
    name = 'binary_version',
    srcs = 'binary_version.cpp',
    deps = '//thirdparty/gflags:gflags'
)

cc_test(
    name = 'binary_version_test',
    srcs = 'binary_version_test.cpp',
    deps = [
        ':binary_version'
    ]
)

cc_test(
    name='closure_test',
    srcs=[
        'closure_test.cpp'
    ],
    deps=[
        '//thirdparty/gtest:gtest'
    ],
    heap_check='strict'
)

cc_test(
    name='scoped_ptr_test',
    srcs=[
        'scoped_ptr_test.cpp'
    ],
    deps=[
        '//thirdparty/gtest:gtest'
    ],
    heap_check='strict'
)

cc_test(
    name='static_assert_test',
    srcs=['static_assert_test.cpp'
         ],
    deps=[]
)

cc_test(
    name='singleton_test',
    srcs=[
        'singleton_test.cpp'
    ],
    deps=[]
)

cc_library(
    name = 'export_variable',
    srcs = [
        'export_variable.cpp',
    ],
    deps = [
        '//ipcs_common/system:check_error',
        '//thirdparty/jsoncpp:jsoncpp',
        '#pthread'
    ]
)

cc_library(
    name = 'ref_counted',
    srcs = 'ref_counted.cpp'
)

cc_test(
    name = 'ref_counted_test',
    srcs = 'ref_counted_test.cpp',
    deps = ':ref_counted'
)

cc_library(
    name = 'function',
    srcs = [
        'function/function_base.cpp'
    ],
    deps = ':ref_counted'
)

cc_test(
    name = 'function_test',
    srcs = [
        'function_test.cpp'
    ],
    deps = [
        '//ipcs_common/base:function',
        '//thirdparty/gmock:gmock'
    ]
)

cc_test(
    name = 'export_variable_test',
    srcs = 'export_variable_test.cpp',
    deps = [
        ':export_variable'
    ]
)

cc_test(
    name='static_resource_test',
    srcs='static_resource_test.cpp',
    deps=[]
)

cc_test(
    name='class_register_test',
    srcs=[
         'class_register_test.cpp',
         'class_register_test_helper.cpp'
    ],
    deps=[]
)
