// Copyright (c) 2014, feimat.
// All rights reserved.
//

#ifndef RPC_CHANNEL_H
#define RPC_CHANNEL_H
#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
#include <vector>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <deque>
#include <iostream>

#include "Define.h"
#include "RpcCommon.h"
#include "xcore_define.h"
#include "xcore_socket.h"
#include "xcore_timer.h"
#include "closure.h"
#include "this_thread.h"
#include "pbext.h"

class RpcClientSock;

class RpcClient : public ::google::protobuf::RpcChannel {
public:
    typedef struct _ParamItem {
        RpcClient* cli;
        XSocket* sock;
    } ParamItem;

    RpcClient(int workernum, const string &host, const int port, int time_out=10000);

    ~RpcClient();

    void CloseAll();

    RpcClientSock* GetRandomPoolIndex() {
        pthread_mutex_lock(&_pool_mux);
        next_pool_index = (++next_pool_index)%sock_num_;
        RpcClientSock* sock_pool = sock_list[next_pool_index];
        pthread_mutex_unlock(&_pool_mux);
        return sock_pool;
    }

    void PutReadQueue(std::string data) {
        pthread_mutex_lock(&_readmux);
        read_queue.push_back(data);
        pthread_mutex_unlock(&_readmux);
        sem_post(&_readsem);
    }

    std::string GetReadQueue() {
        sem_wait(&_readsem);
        pthread_mutex_lock(&_readmux);
        std::string data;
        if (!read_queue.empty()) {
            data = read_queue.front();
            read_queue.pop_front();
        }
        pthread_mutex_unlock(&_readmux);
        return data;
    }

    unsigned GetNextId() {
        pthread_mutex_lock(&_id_mux);
        unsigned newid = (( cli_id == 0xffffffffUL ) ? 1 : cli_id+1);
        cli_id = newid;
        pthread_mutex_unlock(&_id_mux);
        return newid;
    }

    void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done);

    static void *SendProcess(void *argument);
    static void *ReadProcess(void *argument);
    static void *CallBackProcess(void *argument);
public:
    std::vector<RpcClientSock*> sock_list;
    std::deque<std::string> read_queue;
    std::vector<pthread_t> callback_thread_list;
    sem_t _readsem;
    pthread_mutex_t _readmux;
    int sock_num_;
    bool isstop;
    std::string host_;
    int port_;
    MutexMap callback_map;
    int time_out_;
    unsigned cli_id; // 用于标识每个包的唯一标识
    pthread_mutex_t _id_mux;
    xcore::XTimer timer;
    int next_pool_index;
    pthread_mutex_t _pool_mux;
};

class TimeCleaner : public xcore::XTimer::ICallBack {
public:
    TimeCleaner(RpcClient* rpcclient, std::string key) {
        rpcclient_ = rpcclient;
        key_ = key;
    }
    virtual ~TimeCleaner() {
    }

    virtual void on_timer(XTimer* pTimer, uint32 id, void* ptr);
private:
    std::string key_;
    RpcClient* rpcclient_;
};

#endif //
