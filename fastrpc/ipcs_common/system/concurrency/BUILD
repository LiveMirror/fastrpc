# ipcs_common/system/concurrency
# Author: feimat@qq.com
# Date: 02/20/2013
cc_test(
    name='mutex_test',
    srcs='mutex_test.cpp',
    deps=[
        '//ipcs_common/system:check_error'
    ],
    heap_check='strict'
)

cc_library(
    name='condition_variable',
    srcs='condition_variable.cpp',
    deps=[
        '//ipcs_common/system/time:posix_time',
        '//ipcs_common/system:check_error',
        '#pthread'
    ]
)

cc_test(
    name='condition_variable_test',
    srcs=['condition_variable_test.cpp'],
    deps=[
        ':condition_variable',
        '//ipcs_common/thread:thread'
    ]
)
