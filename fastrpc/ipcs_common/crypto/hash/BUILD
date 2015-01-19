# ipcs_common/crypto/hash hash_string module
# Author: feimat@qq.com
# Date: 12/30/2012
cc_library(
    name="string_hash",
    srcs=[
        'string_hash.cpp'
    ]
)

cc_test(
    name='string_hash_test',
    srcs=[
        'string_hash_test.cpp'
    ],
    deps=[
        ':string_hash',
        '//thirdparty/gtest:gtest'
    ]
)
