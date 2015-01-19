# Jack
cc_library(
    name='mysql_connection',
    srcs='mysqlconn.cpp',
    deps=[
        '//thirdparty/mysql:mysqlclient',
        '//thirdparty/glog:glog'
    ]
)

cc_test(
    name='mysqlconn_test',
    srcs='mysqlconn_test.cpp',
    deps=[
        ':mysql_connection'
    ],
    heap_check='strict'
)

cc_library(
    name='mysqlconn_pool',
    srcs='mysqlconn_pool.cpp',
    deps=[
        ':mysql_connection',
        '//ipcs_common/system/concurrency:condition_variable'
    ]
)

cc_test(
    name='mysqlconn_pool_test',
    srcs='mysqlconn_pool_test.cpp',
    deps=[
        ':mysqlconn_pool'
    ],
    heap_check='strict'
)
