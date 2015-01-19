# Jack
cc_library(
    name='redis_connection',
    srcs=[
        'redis_connection.cpp'
    ],
    deps=[
        '//thirdparty/hiredis:hiredis',
        '//thirdparty/protobuf:protobuf',
        '//thirdparty/glog:glog'
    ]
)

cc_test(
    name='redis_connection_test',
    srcs=[
        'redis_connection_test.cpp'
    ],
    deps=[
        ':redis_connection',
        ':redis_pb_test_info'
    ],
    heap_check='strict'
)

cc_library(
    name='redis_pool',
    srcs='redis_pool.cpp',
    deps=[
        ':redis_connection',
        '//ipcs_common/system/concurrency:condition_variable'
    ]
)

cc_test(
    name='redis_pool_test',
    srcs='redis_pool_test.cpp',
    deps=[
        ':redis_pool'
    ],
    heap_check='strict'
)

proto_library(
    name='redis_pb_test_info',
    srcs=[
        'redis_pb_test_info.proto'
    ]
)
