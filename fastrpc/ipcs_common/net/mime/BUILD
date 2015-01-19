# Jack
cc_library(
    name = 'mime',
    srcs = 'mime.cpp',
    deps = [
        '//ipcs_common/base/string:ipcs_string',
        '//thirdparty/glog:glog'
    ]
)

cc_test(
    name = 'test_mime',
    srcs = [
        'test_mime.cpp'
    ],
    deps = [
        ':mime'
    ]
)
