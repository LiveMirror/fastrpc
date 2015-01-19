# Jack
cc_library(
    name = "url",
    srcs = [
        'uri.cpp',
        'cgi_params.cpp'
    ],
    deps = [
        '//ipcs_common/base/string:ipcs_string',
        '//ipcs_common/encoding:encoding',
    ]
)

cc_test(
    name = "uri_test",
    srcs = ['uri_test.cpp'],
    deps = [
        ':url',
        '//ipcs_common/system/io:io',
        '//ipcs_common/system/time:posix_time'
    ],
    testdata = [
        'uris.txt',
        'baduris.txt'
    ]
)

cc_test(
    name = "cgi_params_test",
    srcs = ['cgi_params_test.cpp'],
    deps = [':url']
)
