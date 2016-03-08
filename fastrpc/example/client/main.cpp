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
#include <algorithm>
#include <sstream>

#include "pbext.h"
#include "xcore_atomic.h"
#include "coroutine.h"

using std::map;

#define try_time 1

XAtomic32 recv_c = 0;

class Test {
public:
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
			if (++recv_c == try_time) {
				std::cout << "finish\n";
			}
	}
};

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
        if (++recv_c == try_time) {
			std::cout << "finish\n";
        }
}

// mes name: echo.EchoResponse
void ext_processer(std::string mes_name, std::string data, void* param) {
    ::google::protobuf::Message* response = PbMgr::CreateMessage(mes_name);
    if (response) {
        response->ParseFromString(data);
        std::cout << "recv push mes, name:" << mes_name
            << " data:" << response->DebugString();
    }
}

void close_handler(void* param) {
    std::cout << "server close" << std::endl;
}

void cro_async_cb(echo::EchoRequest* request,
                  echo::EchoResponse* response,
                  std::string p) {
    delete request;
    delete response;
    std::cout << "sss:" << p << std::endl;
}

void cro_job(echo::EchoService_Stub::Stub* stub, int i) {
    std::stringstream ss;
    ss << i;
    echo::EchoRequest req;
    req.set_message("cli hello  "+ ss.str());
    echo::EchoResponse res;
    stub->Echo(NULL, &req, &res, NULL); // 同步

    //echo::EchoRequest* request = new echo::EchoRequest();
    //request->set_message("aaa");
    //echo::EchoResponse* response = new echo::EchoResponse();
    //std::string s = "aaa " + ss.str();
    //::google::protobuf::Closure* routine =
    //    pbext::NewCallback(&cro_async_cb,
    //                       request, response, s);
    //stub->Echo(NULL, request, response, routine); // 异步

    //std::cout << "222:" << res.response() << std::endl;
    if (++recv_c == try_time) {
        printf("finish\n");
    }
}

int main(int argc, char *argv[]) {
    SetCoroutineUsedByCurThread();
    Test test;
    ::google::protobuf::RpcChannel* client;
    echo::EchoService_Stub::Stub* stub;
    client = new RpcClient("127.0.0.1", 8997, 5000); // 1::host 2:port 3:超时时间 4:是否多线程模式
    if (!((RpcClient*)client)->IsConnected()) {
        delete client;
        exit(0);
    } else {
        std::cout << "connect success\n";
    }
    ((RpcClient*)client)->RegiExtProcesser(ext_processer, NULL); // 处理服务器主动推的消息
    ((RpcClient*)client)->RegiCloseHandler(close_handler, NULL); // 断开事件处理
    int i =0;
    while (++i < 5) {
        //xcore::sleep(1000);
    }
    stub = new echo::EchoService_Stub::Stub(client);

    for (int i =0; i < try_time; ++i) {
        // 使用协程来调用同步,和异步回调一样的效果
        ::google::protobuf::Closure* routine =
            ::google::protobuf::NewCallback(&cro_job,
                                            stub, i);
        ProcessWithNewCro(routine);
    }

    echo::EchoRequest req;
    req.set_message("cli hello 2");
    echo::EchoResponse res;
    CliController controller;
    stub->Echo(&controller, &req, &res, NULL); // 同步非协程模式
    std::cout << "is timeout:" << controller.IsTimeOut()
    << " sync: " << res.response() << "\n";
    ++recv_c;

    //while(recv_c < try_time + 1) {
    while(true) {
        RpcClient::OutSideProcess(100000);
    }
    printf("close\n");

	delete client;
    delete stub;


    return 0;
}
