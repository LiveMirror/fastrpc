# ipcs_common/crypto/aes
# Author: feimat@qq.com
# Date: 12/30/2012
cc_library(
    name='aes',
    srcs=['aes.c'],
    deps=[]
)

cc_library(
    name='sha2',
    srcs=['sha2.c'],
    deps=[]
)

cc_binary(
    name='aescrypt',
    srcs=['aescrypt.cpp'],
    deps=[
        ':sha2',
        ':aes'
    ]
)

