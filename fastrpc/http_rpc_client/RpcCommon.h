#ifndef _RPC_COMMON_H_
#define _RPC_COMMON_H_

#include <deque>
#include <stdlib.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <map>

#include "xcore_mutex.h"
#include "xcore_semaphore.h"

#define RpcSafeFree(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \

class CliController : public ::google::protobuf::RpcController {
public:
    CliController() {
        is_timeout = false;
        has_error = false;
    }
    virtual ~CliController() {}

    virtual void Reset() {}
    virtual bool Failed() const {return has_error;}
    virtual std::string ErrorText() const {return err_string;}
    virtual void StartCancel() {}

    virtual void SetFailed(const string& reason) {
        has_error = true;
        err_string = reason;
    }
    virtual bool IsCanceled() const {return false;}
    virtual void NotifyOnCancel(::google::protobuf::Closure* callback) {}
    bool IsTimeOut() {return is_timeout;}

    void CopyFrom(CliController* rpctr) {
        if (this == rpctr) return;
        if (rpctr) {
            is_timeout = rpctr->is_timeout;
            has_error = rpctr->has_error;
            err_string = rpctr->err_string;
        }
    }

    bool is_timeout;
    bool has_error;
    std::string err_string;
};

struct RpcCliMethod {
public:
    RpcCliMethod(::google::protobuf::RpcController* controller,
                 ::google::protobuf::Message *request,
                 ::google::protobuf::Message *response,
                 ::google::protobuf::Closure* done,
                 ::google::protobuf::Closure* cro_done,
                 bool need_copy = false) {
        m_need_copy = need_copy;
        request_ = request;
        response_ = response;
        done_ = done;
        cro_done_ = cro_done;
        controller_ = NULL;
        if (m_need_copy) {
            if (controller) controller_ = new CliController();
        } else {
            controller_ = controller;
        }
    }

    ~RpcCliMethod() {
        if (m_need_copy) {
            RpcSafeFree(controller_);
        }
    }

    ::google::protobuf::RpcController* controller_;
    ::google::protobuf::Message *request_;
    ::google::protobuf::Message *response_;
    ::google::protobuf::Closure *done_;
    ::google::protobuf::Closure *cro_done_;
    bool m_need_copy;
    std::string response_string;
};

struct RpcCallBack {
public:
    RpcCallBack(::google::protobuf::Message *request,
              ::google::protobuf::Message *response,
              ::google::protobuf::Closure *done)
        : request_(request),
          response_(response),
          done_(done) {
          }

    ::google::protobuf::Message *request_;
    ::google::protobuf::Message *response_;
    ::google::protobuf::Closure *done_;
};

class MutexMap {
public:
    MutexMap() {
    }

    ~MutexMap() {
    }

    bool Insert(std::string key, RpcCliMethod* rpc_method) {
        _mux.lock();
        rpc_method_map_[key] = rpc_method;
        _mux.unlock();
        return true;
    }

    RpcCliMethod* Pop(std::string& key) {
        _mux.lock();
        RpcCliMethod* ret = NULL;
        std::map<std::string, RpcCliMethod*>::iterator it = rpc_method_map_.find(key);
        if (it != rpc_method_map_.end()) {
            ret = it->second;
            rpc_method_map_.erase(it);
        }
        _mux.unlock();
        return ret;
    }

    RpcCliMethod* Get(std::string& key) {
        _mux.lock();
        RpcCliMethod* ret = NULL;
        std::map<std::string, RpcCliMethod*>::iterator it = rpc_method_map_.find(key);
        if (it != rpc_method_map_.end()) {
            ret = it->second;
        }
        _mux.unlock();
        return ret;
    }

    std::map<std::string, RpcCliMethod*> rpc_method_map_;

    XMutex _mux;
};


#endif
