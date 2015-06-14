// Copyright (c) 2014, Jack.
// All rights reserved.
//
// Author: feimat <512284622@qq.com>
// Created: 07/10/14
// Description:

#include "rpc_client.h"
#include "echo.pb.h"
#include "xcore_thread.h"
#include "xcore_base64.h"
#include <string>
#include <list>
#include <string.h>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <string.h>
#include <algorithm>
#include <sstream>

#include "pbext.h"
#include "xcore_atomic.h"
#include "coroutine.h"
#include "xcore_socket.h"
#include "xcore_mutex.h"

using std::map;

#define try_time 10

XAtomic64 recv_c = 0;

void hook_sock_job(int i) {
    // 开始sys hook后
    // 系统的socket操作函数遇到阻塞会自动放权
    // 不用修改任何代码，可将同步socket变异步
    co_enable_hook_sys();
    XSocket sock;
    sock.open(SOCK_STREAM);
    if (!sock.connect(XSockAddr("127.0.0.1", 8998))) {
        printf("connect fail\n");
        abort();
    }
    std::stringstream ss;
    ss << "GET http://127.0.0.1:8998/aaa?kk=" << i << " HTTP/1.1\r\n"
        << "Host: 127.0.0.1:8998\r\n"
        << "\r\n";
    std::string send_str = ss.str();
    printf("%d send %d\n", sock.get_handle(), i);
    sock.send_n(send_str.c_str(), send_str.size(), 100000);
    char buf[10240] = {0};
    while (strstr(buf, "final") == NULL) {
        sock.recv(buf, 10240);
    }
    printf("%d recv %d\n", sock.get_handle(), i);

    if (++recv_c == try_time) {
        printf("send all finish\n");
    }

    sock.close();
    co_disable_hook_sys();
}

static TimerMgr hook_timer_mgr;

int main(int argc, char *argv[]) {
    for (int i =0; i < try_time; ++i) {
        ::google::protobuf::Closure* routine =
            ::google::protobuf::NewCallback(&hook_sock_job,i);
        ProcessWithNewCro(routine);
    }

    while(true) {
        RpcClient::OutSideProcess(100000);
    }
    printf("close\n");

    return 0;
}
