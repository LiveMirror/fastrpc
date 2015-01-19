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

struct RpcCliMethod {
public:
    RpcCliMethod(::google::protobuf::Message *request,
                 ::google::protobuf::Message *response,
                 ::google::protobuf::Closure* done)
        : request_(request),
          response_(response),
          done_(done) {
          }

    ::google::protobuf::Message *request_;
    ::google::protobuf::Message *response_;
    ::google::protobuf::Closure *done_;
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
        std::map<std::string, RpcCliMethod*>::iterator it = rpc_method_map_.begin();
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
