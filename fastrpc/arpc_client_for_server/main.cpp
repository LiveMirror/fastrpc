// Copyright (c) 2014, feimat.
// All rights reserved.
//
// Author: feimat <512284622@qq.com>
// Created: 07/10/14
// Description:

#include "rpc_client.h"
#include "echo.pb.h"
#include "xcore_thread.h"
#include <string>
#include <list>
#include <string.h>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


#include "pbext.h" // pb扩展，原生回调最多支持两个参数
#include "this_thread.h"
#include "timer_manager.h"
#include "closure.h"
#include "http_server_codec.h"

int recv_c = 0;

void echo_done(echo::EchoRequest* request,
               echo::EchoResponse* response,
               ::google::protobuf::Closure* done) {
    std::string res = response->response();
    //if (!res.empty())
    //    printf("async: %s\n", response->response().c_str());
    //printf("async: %s\n", request->message().c_str());
    if (done) {
        done->Run();
    }
    delete request; // 回调要注意内存释放
    delete response;
    if (++recv_c == 50000) {
        std::cout << "success\n";
        exit(0);
    }
}

int main(int argc, char *argv[]) {

    RpcClient client(4, "127.0.0.1", 8998, 3000); // 1:并发线程数 2:host 3:ip 4:超时时间
    echo::EchoService::Stub stub(&client);

    for (int i =0; i < 50000; ++i) {
        echo::EchoRequest* request = new echo::EchoRequest();
        request->set_message("client_hello");
        echo::EchoResponse* response = new echo::EchoResponse();
        ::google::protobuf::Closure* callback_callback = NULL; // 可以递归无限回调
        ::google::protobuf::Closure *callback = pbext::NewCallback(&echo_done,
                                                                   request,
                                                                   response,
                                                                   callback_callback);

        stub.Echo(NULL, request, response, callback); // 异步, callback为空则是同步
    }

    echo::EchoRequest req;
    req.set_message("cli hello 2");
    echo::EchoResponse res;
    //stub.Echo(NULL, &req, &res, NULL); // 同步
    std::cout << "sync: " << res.response() << "\n";

    xcore::sleep(3000);

    return 0;
}




//////////////http服务默认配置，走http则无需改动////////////////////
/*
is_complete 和 check_recv_one 返回值
ret<0 小于零,返回失败
ret>0 大于零,返回得到的字节数
ret=0 等于零,还要继续接收
注意如果包大于 CLIENT_COMPLETE_MAX_BUFFER 和 BACK_COMPLETE_MAX_BUFFER，需要放弃包。
*/
int CClientSocketReq::is_complete(char *data,unsigned data_len)
{
    return http_complete_func(data,data_len);
}

int CBackSocketReq::is_complete(char *data,unsigned data_len)
{
    return http_complete_func(data,data_len);
}

//业务逻辑默认同步模式
xCallbackObj *CMainAsyncSvr::CreateAsyncObj(CDataBuf *item)
{
    return NULL;
}
CTask *CASyncSvr::CCreatePoolTask()
{
    return NULL;
}
