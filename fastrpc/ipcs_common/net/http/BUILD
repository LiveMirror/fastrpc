# Jack
cc_library(
    name = "http",
    srcs = [
        "http_base.cpp",
        "http_client.cpp",
        "http_connection.cpp",
        "http_connection_manager.cpp",
        "http_downloader.cpp",
        "http_handler.cpp",
        "http_handler_manager.cpp",
        "http_headers.cpp",
        "http_message.cpp",
        "http_request.cpp",
        "http_response.cpp",
        "http_server.cpp",
        "http_time.cpp",
        "http_stats.cpp"
    ],
    deps = [
        "//ipcs_common/base/string:ipcs_string",
        "//ipcs_common/collections:collections",
        "//ipcs_common/net/mime:mime",
        "//ipcs_common/net/uri:url",
        "//ipcs_common/netframe:netframe",
        "//ipcs_common/system/net:domain_resolver",
        '//ipcs_common/system/concurrency:condition_variable',
        '//ipcs_common/system/time:posix_time',
        "//thirdparty/glog:glog"
    ]
)

cc_test(
    name = 'http_stats_test',
    srcs = 'http_stats_test.cpp',
    deps = [
        ':http',
        '//ipcs_common/system/time:posix_time'
    ]
)

cc_test(
    name = 'http_headers_test',
    srcs = 'http_headers_test.cpp',
    deps = ':http'
)

cc_test(
    name = 'http_message_test',
    srcs = 'http_message_test.cpp',
    deps = ':http'
)

cc_test(
    name = 'http_request_test',
    srcs = 'http_request_test.cpp',
    deps = ':http'
)

cc_test(
    name = 'http_response_test',
    srcs = 'http_response_test.cpp',
    deps = ':http'
)

cc_test(
    name = 'http_time_test',
    srcs = 'http_time_test.cpp',
    deps = ':http'
)

cc_test(
    name = "http_client_server_test",
    srcs = [
        "http_client_server_test.cpp"
    ],
    deps = [
        ':http',
        '//ipcs_common/thread:thread_pool',
        '//ipcs_common/thread:base_thread'
    ]
)

cc_test(
    name = "http_downloader_test",
    srcs = [
        "http_downloader_test.cpp"
    ],
    deps = [
        ":http",
        '//ipcs_common/system/concurrency:condition_variable'
    ]
)
