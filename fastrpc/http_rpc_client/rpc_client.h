// Copyright (c) 2013, feimat.
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
#include <deque>
#include <iostream>

#include "RpcCommon.h"
#include "xcore_define.h"
#include "xcore_socket.h"
#include "xcore_timer.h"
#include "xcore_thread.h"
#include "xcore_mutex.h"
#include "xcore_semaphore.h"
#include "http_util.h"
#include "closure.h"
#include "xcore_rpc_common.h"

class RpcClient;

typedef struct _ParamItem {
    RpcClient* cli;
	XSocket* sock;
	XSemaphore consem;
} ParamItem;

#ifdef __WINDOWS__
#define ProcessType unsigned __stdcall
#endif//__WINDOWS__
#ifdef __GNUC__
#define ProcessType void*
#endif//__GNUC__

ProcessType SendProcess(void *argument);
ProcessType ReadProcess(void *argument);
ProcessType CallBackProcess(void *argument);

typedef void ext_process(std::string mes_name, std::string data, void* param);
typedef void event_handler(void* param);

class RpcClient : public ::google::protobuf::RpcChannel {
public:
    RpcClient(const string &host, const int port,
            int time_out=10000, bool multi_thread_mode = false) {
        _ext_processer = NULL;
        _ext_param = NULL;
        _close_handler = NULL;
        _close_handler_param = NULL;
        isstop = false;
        _is_connected = true;
        host_ = host;
        port_ = port;
        time_out_ = time_out;
        m_multi_thread_mode = multi_thread_mode;
        cli_id = 0;
        // init sock
        ParamItem* param = new ParamItem();
        param->cli = this;
        XSocket* sock = new XSocket();
        sock->open(SOCK_STREAM);
        sock->set_nonblock(true);
        bool bRet = sock->connect(XSockAddr(host, port));
        param->sock = sock;
        m_param = param;
        if (!bRet || !sock->can_send()) {
            printf("rpc connect to server fail.");
            sock->close();
            _is_connected = false;
        }

        // start recv thread
#ifdef __WINDOWS__
        unsigned threadID;
        recv_thread = (HANDLE)_beginthreadex( NULL, 0, ReadProcess, m_param, 0, &threadID );
#endif//__WINDOWS__
#ifdef __GNUC__
        pthread_t tid = 0;
        pthread_create(&tid, NULL, ReadProcess, m_param);
        recv_thread = tid;
#endif//__GNUC__

        // start send thread
#ifdef __WINDOWS__
        unsigned threadID;
        send_thread = (HANDLE)_beginthreadex( NULL, 0, SendProcess, m_param, 0, &threadID );
#endif//__WINDOWS__
#ifdef __GNUC__
        tid = 0;
        pthread_create(&tid,NULL,SendProcess,m_param);
        send_thread = tid;
#endif//__GNUC__

        // start callback thread for multi thread mode
        if (m_multi_thread_mode) {
#ifdef __WINDOWS__
            unsigned threadID;
            callback_thread = (HANDLE)_beginthreadex( NULL, 0, CallBackProcess, m_param, 0, &threadID );
#endif//__WINDOWS__
#ifdef __GNUC__
            tid = 0;
            pthread_create(&tid,NULL,CallBackProcess,m_param);
            callback_thread = tid;
#endif//__GNUC__
        }
        timer.start();
    }

    ~RpcClient() {
        CloseAll();
    }

private:
    void CloseAll() {
        _is_connected = false;
        isstop = true;
        // close sem
        m_param->consem.post();
        _sendsem.post();
        // close sock
        XSocket* sock = m_param->sock;
        sock->StopAutoConnect(true);
        sock->close();
        // close recv thread
#ifdef __WINDOWS__
        WaitForSingleObject(recv_thread, INFINITE);
        CloseHandle(recv_thread);
        recv_thread = INVALID_HANDLE_VALUE;
#endif//__WINDOWS__
#ifdef __GNUC__
        pthread_join(recv_thread, NULL);
#endif//__GNUC__

        // close send thread
#ifdef __WINDOWS__
        WaitForSingleObject(send_thread, INFINITE);
        CloseHandle(send_thread);
        send_thread = INVALID_HANDLE_VALUE;
#endif//__WINDOWS__
#ifdef __GNUC__
        pthread_join(send_thread, NULL);
#endif//__GNUC__

        if (m_multi_thread_mode) {
#ifdef __WINDOWS__
            WaitForSingleObject(callback_thread, INFINITE);
            CloseHandle(callback_thread);
            callback_thread = INVALID_HANDLE_VALUE;
#endif//__WINDOWS__
#ifdef __GNUC__
            pthread_join(callback_thread, NULL);
#endif//__GNUC__
        }

        sock = m_param->sock;
        delete sock;
        ParamItem* param = m_param;
        delete param;

        timer.stop();
    }

public:
    static int OutSideProcess(unsigned msec);

	void RegiExtProcesser(ext_process* ext_processer, void* param) {
		_ext_processer = ext_processer;
		_ext_param = param;
	}

	void RegiCloseHandler(event_handler* close_handler, void* param) {
		_close_handler = close_handler;
		_close_handler_param = param;
	}

    void PutSendQueue(std::string data) {
        _sendmux.lock();
        send_queue.push_back(data);
        _sendmux.unlock();
        _sendsem.post();
    }
    std::string GetSendQueue() {
        _sendsem.wait();
        _sendmux.lock();
        std::string data;
        if (!send_queue.empty()) {
            data = send_queue.front();
            send_queue.pop_front();
        }
        _sendmux.unlock();
        return data;
    }

    unsigned GetNextId() {
        _id_mux.lock();
        unsigned newid = (( cli_id == 0xffffffffUL ) ? 1 : cli_id+1);
        cli_id = newid;
        _id_mux.unlock();
        return newid;
    }

    void SingleThreadMode(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done);

    void MultiThreadMode(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done);

    void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done);

	bool IsConnected() { return _is_connected;}

public:
	bool _is_connected;
    ParamItem* m_param;
    std::deque<std::string> send_queue;
#ifdef __WINDOWS__
    HANDLE recv_thread;
    HANDLE send_thread;
    HANDLE callback_thread;
#endif//__WINDOWS__
#ifdef __GNUC__
    pthread_t recv_thread;
    pthread_t send_thread;
    pthread_t callback_thread;
#endif//__GNUC__
    XSemaphore _sendsem;
    XMutex _sendmux;

    int sock_num_;
    bool isstop;
    std::string host_;
    int port_;
    MutexMap callback_map;
    MutexMap cro_callback_map;
    int time_out_;
    unsigned cli_id; // 用于标识每个包的唯一标识
    XMutex _id_mux;
    xcore::XTimer timer;
    ext_process* _ext_processer;
    void* _ext_param;
    event_handler* _close_handler;
    void* _close_handler_param;
    bool m_multi_thread_mode;
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
