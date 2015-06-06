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
//#include <signal.h>
//#include <pthread.h>
//#include <semaphore.h>
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

typedef int ext_process(CHttpParser& ps, std::string& data, void* param);
typedef void event_handler(void* param);

class CliController : public ::google::protobuf::RpcController {
public:
    CliController() {
        is_timeout = false;
    }
    virtual ~CliController() {}

    virtual void Reset() {}
    virtual bool Failed() const {return false;}
    virtual std::string ErrorText() const {return "";}
    virtual void StartCancel() {}

    virtual void SetFailed(const string& reason) {}
    virtual bool IsCanceled() const {return false;}
    virtual void NotifyOnCancel(::google::protobuf::Closure* callback) {}
    bool is_timeout;
    bool IsTimeOut() {return is_timeout;}
};

class RpcClient : public ::google::protobuf::RpcChannel {
public:
    RpcClient(const int sock_num, const string &host, const int port, int time_out=10000) {
		_ext_processer = NULL;
		_ext_param = NULL;
		_close_handler = NULL;
		_close_handler_param = NULL;
        isstop = false;
	    _is_connected = true;
        host_ = host;
        port_ = port;
        sock_num_ = sock_num;
        time_out_ = time_out;
        cli_id = 0;
        for (int i = 0; i < sock_num_; ++i) {
            ParamItem* param = new ParamItem();
            param->cli = this;
            XSocket* sock = new XSocket();
            sock->open(SOCK_STREAM);
            sock->set_nonblock(true);
			sock->connect(XSockAddr(host, port));
            param->sock = sock;
            param_list.push_back(param);
			if (!sock->can_send()) {
                printf("rpc connect to server fail.");
                sock->close();
                _is_connected = false;
				sock_num_ = i+1;
				break;
            }
        }
        for (int i = 0; i < sock_num_; ++i) {
#ifdef __WINDOWS__
            HANDLE hThread;
            unsigned threadID;
            hThread = (HANDLE)_beginthreadex( NULL, 0, ReadProcess, param_list[i], 0, &threadID );
            read_thread_list.push_back(hThread);
#endif//__WINDOWS__
#ifdef __GNUC__
            pthread_t tid = 0;
            pthread_create(&tid,NULL,ReadProcess,param_list[i]);
            read_thread_list.push_back(tid);
#endif//__GNUC__
        }
        for (int i = 0; i < sock_num_; ++i) {
#ifdef __WINDOWS__
            HANDLE hThread;
            unsigned threadID;
            hThread = (HANDLE)_beginthreadex( NULL, 0, SendProcess, param_list[i], 0, &threadID );
            send_thread_list.push_back(hThread);
#endif//__WINDOWS__
#ifdef __GNUC__
            pthread_t tid = 0;
            pthread_create(&tid,NULL,SendProcess,param_list[i]);
            send_thread_list.push_back(tid);
#endif//__GNUC__
        }
        for (int i = 0; i < sock_num_ && i < 8; ++i) {
#ifdef __WINDOWS__
            HANDLE hThread;
            unsigned threadID;
            hThread = (HANDLE)_beginthreadex( NULL, 0, CallBackProcess, param_list[i], 0, &threadID );
            callback_thread_list.push_back(hThread);
#endif//__WINDOWS__
#ifdef __GNUC__
            pthread_t tid = 0;
            pthread_create(&tid,NULL,CallBackProcess,param_list[i]);
            callback_thread_list.push_back(tid);
#endif//__GNUC__
        }
        timer.start();
    }

    ~RpcClient() {
        CloseAll();
    }

    void CloseAll() {
		_is_connected = false;
        isstop = true;
        for (int i = 0; i < sock_num_; ++i) {
			param_list[i]->consem.post();
            _readsem.post();
            _sendsem.post();
        }
		for (int i = 0; i < sock_num_; ++i) {
			XSocket* sock = param_list[i]->sock;
			sock->StopAutoConnect(true);
			sock->close();
		}
		for (int i = 0; i < read_thread_list.size(); ++i) {
#ifdef __WINDOWS__
			HANDLE m_handle = read_thread_list[i];
			//TerminateThread(m_handle, 0);
			WaitForSingleObject(m_handle, INFINITE);
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
#endif//__WINDOWS__
#ifdef __GNUC__
			pthread_t m_id = read_thread_list[i];
			//pthread_cancel(m_id);
			pthread_join(m_id, NULL);
#endif//__GNUC__
		}
		for (int i = 0; i < send_thread_list.size(); ++i) {
#ifdef __WINDOWS__
			HANDLE m_handle = send_thread_list[i];
			//TerminateThread(m_handle, 0);
			WaitForSingleObject(m_handle, INFINITE);
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
#endif//__WINDOWS__
#ifdef __GNUC__
			pthread_t m_id = send_thread_list[i];
			//pthread_cancel(m_id);
			pthread_join(m_id, NULL);
#endif//__GNUC__
		}
		for (int i = 0; i < callback_thread_list.size(); ++i) {
#ifdef __WINDOWS__
			HANDLE m_handle = callback_thread_list[i];
			//TerminateThread(m_handle, 0);
			WaitForSingleObject(m_handle, INFINITE);
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
#endif//__WINDOWS__
#ifdef __GNUC__
			pthread_t m_id = callback_thread_list[i];
			//pthread_cancel(m_id);
			pthread_join(m_id, NULL);
#endif//__GNUC__
		}
		for (int i = 0; i < sock_num_; ++i) {
			XSocket* sock = param_list[i]->sock;
			delete sock;
		}
        for (int i = 0; i < sock_num_; ++i) {
            ParamItem* param = param_list[i];
            delete param;
        }
        param_list.clear();
        read_thread_list.clear();
        send_thread_list.clear();
        callback_thread_list.clear();
        timer.stop();
    }

	void RegiExtProcesser(ext_process* ext_processer, void* param) {
		_ext_processer = ext_processer;
		_ext_param = param;
	}

	void RegiCloseHandler(event_handler* close_handler, void* param) {
		_close_handler = close_handler;
		_close_handler_param = param;
	}

    void PutSendQueue(std::string data) {
        //std::cout << data << "put send\n";
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

    void PutReadQueue(std::string data) {
        _readmux.lock();
        read_queue.push_back(data);
        _readmux.unlock();
        _readsem.post();
    }

    std::string GetReadQueue() {
        _readsem.wait();
        _readmux.lock();
        std::string data;
        if (!read_queue.empty()) {
            data = read_queue.front();
            read_queue.pop_front();
        }
        _readmux.unlock();
        return data;
    }

    unsigned GetNextId() {
        _id_mux.lock();
        unsigned newid = (( cli_id == 0xffffffffUL ) ? 1 : cli_id+1);
        cli_id = newid;
        _id_mux.unlock();
        return newid;
    }

    void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done);

	bool IsConnected() { return _is_connected;}

public:
	bool _is_connected;
    std::vector<ParamItem*> param_list;
    std::deque<std::string> send_queue;
    std::deque<std::string> read_queue;
#ifdef __WINDOWS__
    std::vector<HANDLE> read_thread_list;
    std::vector<HANDLE> send_thread_list;
    std::vector<HANDLE> callback_thread_list;
#endif//__WINDOWS__
#ifdef __GNUC__
    std::vector<pthread_t> read_thread_list;
    std::vector<pthread_t> send_thread_list;
    std::vector<pthread_t> callback_thread_list;
#endif//__GNUC__
    XSemaphore _readsem;
    XSemaphore _sendsem;
    XMutex _readmux;
    XMutex _sendmux;
    int sock_num_;
    bool isstop;
    std::string host_;
    int port_;
    MutexMap callback_map;
    int time_out_;
    unsigned cli_id; // 用于标识每个包的唯一标识
    XMutex _id_mux;
    xcore::XTimer timer;
	ext_process* _ext_processer;
	void* _ext_param;
	event_handler* _close_handler;
	void* _close_handler_param;
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
