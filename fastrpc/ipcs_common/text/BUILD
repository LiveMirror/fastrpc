# Jack
cc_library(
    name = 'wildcard',
    srcs = [
        'wildcard.cpp'
    ]
)

cc_test(
    name = 'wildcard_test',
    srcs = [
        'wildcard_test.cpp'
    ],
    deps = [
        ':wildcard'
    ]
)

cc_library(
    name='csv',
    srcs=[
        'csv_parser.cpp'
    ],
    deps=[
        '//ipcs_common/base/string:ipcs_string'
    ]
)

cc_test(
    name='csv_parser_test',
    srcs=[
        'csv_parser_test.cpp'
    ],
    deps=[
        ':csv'
    ],
    testdata=[
        'goods.csv'
    ]
)

