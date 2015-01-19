# ipcs_common/netframe
# Author: feimat@qq.com
# Date: 01/28/2013
cc_library(
    name = 'netframe',
    srcs = [
        'command_queue.cpp',
        'datagram_socket_context.cpp',
        'epoll_event_poller.cpp',
        'listen_socket_context.cpp',
        'netframe.cpp',
        'socket_context.cpp',
        'stream_socket_context.cpp',
        'work_thread.cpp',
        'wsa_event_poller.cpp'
    ],
    deps = [
        '//ipcs_common/system/concurrency:spinlock',
        '//ipcs_common/thread:base_thread',
        '//ipcs_common/thread:this_thread',
        '//ipcs_common/system/memory:mempool',
        '//ipcs_common/system/net:socket',
        '//thirdparty/glog:glog',
    ]
)

cc_binary(
    name = 'eventfd_test',
    srcs = ['eventfd_test.cpp'],
    deps = [
    ]
)

cc_test(
    name = 'command_queue_test',
    srcs = ['command_queue_test.cpp'],
    deps = ':netframe'
)

cc_test(
    name = 'netframe_test',
    srcs = ['netframe_test.cpp'],
    deps = [
        '//ipcs_common/netframe:netframe',
        '//thirdparty/glog:glog',
    ]
)
