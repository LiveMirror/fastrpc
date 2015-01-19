# ipcs_common/timer
# Author: feimat@qq.com
# Date: 02/20/2013
cc_library(
    name='timer_manager',
    srcs=[
        'timer_manager.cpp'
    ],
    deps=[
        '//ipcs_common/thread:base_thread',
        '//ipcs_common/system/concurrency:condition_variable',
        '//ipcs_common/system/time:posix_time'
    ]
)

cc_test(
    name='timer_manager_test',
    srcs=[
        'timer_manager_test.cpp'
    ],
    deps=[
        ':timer_manager'
    ],
    heap_check='strict'
)
