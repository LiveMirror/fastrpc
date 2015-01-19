# ipcs_common/netframe
# Author: feimat@qq.com
# Date: 01/28/2013

cc_binary(
    name = "test_echo",
    srcs = [
        "test_echo.cpp"
    ],
    deps = [
        "//ipcs_common/netframe:netframe"
    ]
)

cc_binary(
    name = "test_shortconn_client",
    srcs = [
        "test_shortconn_client.cpp"
    ],
    deps = [
        '//ipcs_common/netframe:netframe',
        '//ipcs_common/system/time:posix_time'
    ]
)

cc_binary(
    name = "test_shortconn_server",
    srcs = [
        "test_shortconn_server.cpp"
    ],
    deps = [
        "//ipcs_common/netframe:netframe"
    ]
)

