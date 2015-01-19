cc_library(
    name = 'regex',
    srcs = [
        'regex.cpp',
        'regex_scanner.cpp',
        'value_parser.cpp',
    ],
    deps = '//thirdparty/pcre:pcre'
)

cc_test(
    name = 'regex_scanner_test',
    srcs = [
        'regex_scanner_test.cpp'
    ],
    deps = [
        ':regex'
    ]
)

cc_test(
    name = 'regex_test',
    srcs = [
        'regex_test.cpp'
    ],
    deps = [
        ':regex'
    ]
)
