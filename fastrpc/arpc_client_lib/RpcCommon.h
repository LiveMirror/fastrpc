#ifndef _RPC_COMMON_H_
#define _RPC_COMMON_H_

#include <deque>
#include <stdlib.h>
#include <pthread.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <map>
#include "monitor.h"

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
        pthread_mutex_init(&_mux,NULL);
    }

    ~MutexMap() {
        pthread_mutex_destroy(&_mux);
    }

    bool Insert(std::string key, RpcCliMethod* rpc_method) {
        pthread_mutex_lock(&_mux);
        rpc_method_map_[key] = rpc_method;
        pthread_mutex_unlock(&_mux);
        return true;
    }

    RpcCliMethod* Pop(std::string& key) {
        pthread_mutex_lock(&_mux);
        RpcCliMethod* ret = NULL;
        std::map<std::string, RpcCliMethod*>::iterator it = rpc_method_map_.find(key);
        if (it != rpc_method_map_.end()) {
            ret = it->second;
            rpc_method_map_.erase(it);
        }
        pthread_mutex_unlock(&_mux);
        return ret;
    }

    RpcCliMethod* Get(std::string& key) {
        pthread_mutex_lock(&_mux);
        RpcCliMethod* ret = NULL;
        std::map<std::string, RpcCliMethod*>::iterator it = rpc_method_map_.begin();
        if (it != rpc_method_map_.end()) {
            ret = it->second;
        }
        pthread_mutex_unlock(&_mux);
        return ret;
    }

    std::map<std::string, RpcCliMethod*> rpc_method_map_;

    pthread_mutex_t _mux;
};


#endif
