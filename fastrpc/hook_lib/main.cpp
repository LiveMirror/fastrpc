// Copyright (c) 2014, Jack.
// All rights reserved.
//
// Author: feimat <512284622@qq.com>
// Created: 07/10/14
// Description:

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

#include "tconnd_xcore_common.h"
#include "tconnd_xcore_json.h"

using namespace xcore;


using std::map;

#define send_time 1

bool GetJsonContent(string& http_data, string& json_content)
{
    size_t start = http_data.find('{');
    size_t end = http_data.rfind('}');
    if (string::npos == start || string::npos == end) return false;
    json_content = http_data.substr(start, end-start+1);
    return true;
}

class Test {
public:
    void SendAndGetHttpJob(int i) {
        co_enable_hook_sys();
        printf("aaaaa\n");
        gethostbyname("");
        printf("bbbbb\n");
        co_disable_hook_sys();
        return;
        
        // 开始sys hook后
        // 系统的socket操作函数遇到阻塞会自动放权
        // 不用修改任何代码，可将同步socket变异步


        int timeout = 1000; // ms
        std::string get_url = "10.12.234.223:8089/v1/security/key?gameid=128509493";

        //co_enable_hook_sys();
        printf("1111\n");
        XSocket sock;
        sock.open(SOCK_STREAM);
        sock.set_nonblock(true);

        do {
        printf("22222\n");
            XSockAddr addr(get_url);
        printf("33333\n");
            int connect_ret = sock.connect(addr, timeout);
            if (connect_ret < 0)
            {
                printf("connect to %s fail\n", get_url.c_str());
                break;
            }

            std::stringstream send_stream;
            send_stream << "GET " << get_url << " HTTP/1.1\r\n"
                << "Host: " << addr.m_host << ":" << addr.m_port << "\r\n\r\n";

            std::string send_data = send_stream.str();
            int send_size = sock.send_n(send_data.c_str(), send_data.size(), timeout);
            if (send_size != (int)send_data.size()) {
                printf("send get gamekey req fail");
                break;
            }

            std::string res;
            int recv_size = sock.recv_one_http(res, timeout);
            if (-2 == recv_size) {
                printf("recv timeout in getting gamekey");
                break;
            } else if (0 >= recv_size) {
                printf("recv error or close by peer in getting gamekey");
                break;
            }
            string json_content;
            GetJsonContent(res, json_content);
            XJsonValue root;
            XJsonReader reader;
            if (reader.parse(json_content, root))
            {
                //vector<XJsonValue>& game_key_list = root["game_key"].as_array();
                XJsonValue NullObj;
                vector<XJsonValue>& game_key_list = NullObj.as_array();
                if (game_key_list.size() == 0) printf("#############\n");
                printf("[");
                for (vector<XJsonValue>::iterator it = game_key_list.begin();
                        it != game_key_list.end(); ++it)
                {
                    printf("%s,", it->as_string().c_str());
                }
                printf("]\n");

            }

            printf("parse and get json_content:%s\n", json_content.c_str());

            printf("fresh game key suc!!!\n");
        } while(false);

        sock.close();
        //co_disable_hook_sys();
    }
};

int main(int argc, char *argv[]) {

    SetCoroutineUsedByCurThread();
    Test test;

    for (int i =0; i < send_time; ++i) {
        Closure<void>* routine = NewClosure(&test,&Test::SendAndGetHttpJob,i);
        ProcessWithNewCro(routine);
    }

    int i = 0;
    while(++i < 1000) {
        XcoreMgr::Update(1); //等待1ms直到有回调
        printf("######\n");
    }

    //CloseUseCoroutine();

    printf("close\n");

    return 0;
}
