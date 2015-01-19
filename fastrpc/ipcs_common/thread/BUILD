# ipcs_common/thread
# Author: feimat@qq.com
# Date: 02/20/2013
cc_library(
    name='thread_types',
    srcs='thread_types.cpp',
    deps=[
        '//ipcs_common/system:check_error',
        '#pthread'
    ]
)

cc_library(
    name='this_thread',
    srcs='this_thread.cpp',
    deps=[
        '#pthread'
    ]
)

cc_library(
    name='base_thread',
    srcs='base_thread.cpp',
    deps=[
        ':this_thread',
        '//ipcs_common/system:check_error',
    ]
)

cc_library(
    name='thread',
    srcs='thread.cpp',
    deps=[
        ':base_thread',
        '//ipcs_common/base:function'
    ]
)

cc_library(
    name='thread_pool',
    srcs='thread_pool.cpp',
    deps=[
        ':thread',
        '//ipcs_common/system/concurrency:condition_variable',
    ]
)

cc_library(
    name='thread_group',
    srcs='thread_group.cpp',
    deps=[
        '//ipcs_common/thread:thread'
    ]
)

cc_test(
    name='thread_group_test',
    srcs=[
        'thread_group_test.cpp'
    ],
    deps=[
        ':thread_group'
    ]
)

cc_test(
    name='base_thread_test',
    srcs='base_thread_test.cpp',
    deps=[
        ':base_thread'
    ],
    heap_check='strict'
)

cc_test(
    name='thread_test',
    srcs='thread_test.cpp',
    deps=[
        ':thread'
    ],
    heap_check='strict'
)

cc_test(
    name='thread_pool_test',
    srcs='thread_pool_test.cpp',
    deps=[
        ':thread_pool'
    ],
    heap_check='strict'
)
