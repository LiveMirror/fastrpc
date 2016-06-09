#ifndef _HTTP_RPC_SERVER_H_
#define _HTTP_RPC_SERVER_H_
#include <string>
#include <list>
#include <string.h>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>

#include "CSvr.h"
#include "CMainD.h"
#include "CBackNetD.h"
#include "CCliNetD.h"
#include "http_server_codec.h"
#include "http_util.h"
#include "hashtable.h"
//#include "base64.h"
//#include "my_lib.h"
//#include "system_information.h"
#include "CLog.h"
//#include "myconfig.h"
#include "xcore_mutex.h"
#include "xcore_semaphore.h"
#include "xcore_rpc_common.h"
#include "rpc_client.h"

using namespace std;

#define RPCIONUM 4
#define RPCWORKNUM 2
#define RPCSOCKNUM 100000

#define RPCREGI(x, y)      \
    for(int i = 0; i < x.rpc_size; ++i) {   \
        ::google::protobuf::Service *service = new y();  \
        x.AddService(i, service);  \
    }   \

#define RPCREGI_ONEP(x, y, z)      \
    for(int i = 0; i < x.rpc_size; ++i) {   \
        ::google::protobuf::Service *service = new y(z);  \
        x.AddService(i, service);  \
    }   \

#define HTTPREGI(x, y)      \
    for(int i = 0; i < x.rpc_size; ++i) {   \
        HttpHandler *handler = new y();  \
        x.AddHttpHandler(handler);  \
    }   \

void* ProcLoop(void* args);

struct RpcMethod {
public:
    RpcMethod(::google::protobuf::Service *service,
              const ::google::protobuf::Message *request,
              const ::google::protobuf::Message *response,
              const ::google::protobuf::MethodDescriptor *method)
        : service_(service),
          request_(request),
          response_(response),
          method_(method) {
          }

    ::google::protobuf::Service *service_;
    const ::google::protobuf::Message *request_;
    const ::google::protobuf::Message *response_;
    const ::google::protobuf::MethodDescriptor *method_;
};


class RpcController : public ::google::protobuf::RpcController {
public:
    RpcController (CASyncSvr* svr, unsigned cli_flow) {
        _svr = svr;
        _cli_flow = cli_flow;
    }
    RpcController();

    virtual ~RpcController() {}

    virtual void Reset() {}
    virtual bool Failed() const {return false;}
    virtual std::string ErrorText() const {return "";}
    virtual void StartCancel() {}

    virtual void SetFailed(const string& reason) {}
    virtual bool IsCanceled() const {return false;}
    virtual void NotifyOnCancel(::google::protobuf::Closure* callback) {}
    CASyncSvr* _svr;
    unsigned _cli_flow;
};

struct HandleServiceEntry {
    HandleServiceEntry(const ::google::protobuf::MethodDescriptor *method,
                       ::google::protobuf::Message *request,
                       ::google::protobuf::Message *response,
                       CASyncSvr* svr,
                       unsigned clid_flow,
                       std::string& cli_id,
                       ::google::protobuf::RpcController* rpc_controller)
        : method_(method),
          request_(request),
          response_(response),
          svr_(svr),
          clid_flow_(clid_flow),
          cli_id_(cli_id),
          rpc_controller_(rpc_controller) {
          }
    const ::google::protobuf::MethodDescriptor *method_;
    ::google::protobuf::Message *request_;
    ::google::protobuf::Message *response_;
    CASyncSvr* svr_;
    unsigned clid_flow_;
    std::string cli_id_;
    ::google::protobuf::RpcController* rpc_controller_;
};

typedef map<std::string, RpcMethod*> RpcMethodMap;

class HttpRequest {
public:
    HttpRequest (CHttpParser* _ps,
                 char* _data,
                 int _data_len,
                 unsigned _cli_flow,
                 CASyncSvr* _svr) {
        ps = _ps;
        data = _data;
        data_len = _data_len;
        cli_flow = _cli_flow;
        svr = _svr;
    }
    CHttpParser* ps;
    char* data;
    int data_len;
    unsigned cli_flow;
    CASyncSvr* svr;
    std::string response;
};

class HttpHandler {
public:
    virtual void Init(CASyncSvr* svr) {}
    virtual void OnRec(HttpRequest* request,
                       ::google::protobuf::Closure *done) {
        if (done) {
            done->Run();
        }
    }
    virtual void Finish(CASyncSvr* svr) {}
};

class RpcServer {
public:
    RpcServer(std::string host, int port) {
        listen_fd_ = -1;
        host_ = host;
        port_ = port;
        rpc_pop_index = 0;
        http_pop_index = 0;
        has_http_handler = false;
        rpc_size = sysconf(_SC_NPROCESSORS_CONF);  
        if (rpc_size < 1) rpc_size = 1;
        rpc_list = new RpcMethodMap[rpc_size];
        pthread_mutex_init(&_mux,NULL);
        pthread_t tid = 0;
        pthread_create(&tid, NULL, ProcLoop, this);
    }

    RpcServer(int listen_fd) {
        listen_fd_ = listen_fd;
        rpc_pop_index = 0;
        http_pop_index = 0;
        has_http_handler = false;
        rpc_size = 1;
        rpc_list = new RpcMethodMap[rpc_size];
        pthread_mutex_init(&_mux,NULL);
        pthread_t tid = 0;
        pthread_create(&tid, NULL, ProcLoop, this);
    }

    ~RpcServer() {
        pthread_mutex_destroy(&_mux);
        // 先不释放内存了
    }

    bool start();

    bool start(int listen_fd);

    bool AddService(int i, ::google::protobuf::Service *service) {
        RpcMethodMap& rpc_method_map_ = ((RpcMethodMap*)rpc_list)[i];
        const ::google::protobuf::ServiceDescriptor *descriptor = service->GetDescriptor();
        for (int i = 0; i < descriptor->method_count(); ++i) {
            const ::google::protobuf::MethodDescriptor *method = descriptor->method(i);
            const google::protobuf::Message *request =
                &service->GetRequestPrototype(method);
            const google::protobuf::Message *response =
                &service->GetResponsePrototype(method);
            RpcMethod *rpc_method = new RpcMethod(service, request,
                                                  response, method);
            std::string opcode = method->full_name();
            //printf("op %s\n", opcode.c_str());
            rpc_method_map_[opcode] = rpc_method;
        }
        return true;
    }

    bool AddHttpHandler(HttpHandler* handler) {
        has_http_handler = true;
        http_handler_list.push_back(handler);
        return true;
    }

    // 单例
    bool RegiService(::google::protobuf::Service *service) {
        for(int i = 0; i < rpc_size; ++i) {
            AddService(i, service);
        }
        return true;
    }

    // 单例
    bool RegiHttpHandler(HttpHandler* handler) {
        for(int i = 0; i < rpc_size; ++i) {
            AddHttpHandler(handler);
        }
        return true;
    }

    RpcMethodMap* pop_service() {
        pthread_mutex_lock(&_mux);
        int index = -1;
        if (rpc_pop_index < rpc_size) {
            index = rpc_pop_index++;
        }
        pthread_mutex_unlock(&_mux);
        if (-1 != index) {
            RpcMethodMap* p = (RpcMethodMap*)rpc_list;
            return &p[index];
        }
        else
            return NULL;
    }

    HttpHandler* pop_http_handler() {
        pthread_mutex_lock(&_mux);
        int index = -1;
        if (http_pop_index < (int)http_handler_list.size()) {
            index = http_pop_index++;
        }
        pthread_mutex_unlock(&_mux);
        if (-1 != index) {
            HttpHandler* ret = http_handler_list[index];
            return ret;
        }
        else
            return NULL;
    }

    bool RegiClientCloseHandler(ext_handler* close_handler, void* close_handler_param) {
        if (!close_handler) {
            return false;
        }
        _close_handler = close_handler;
        _close_handler_param = close_handler_param;
        return true;
    }

    static bool PushRawDataToClient(CASyncSvr* svr, unsigned cli_flow, std::string& data) {
        assert(svr != NULL);
        svr->SendBack(cli_flow,(char*)(data.c_str()),data.length());
        return true;
    }

    static bool PushToClient(CASyncSvr* svr,
                             unsigned cli_flow,
                             google::protobuf::Message* message) {
        assert(svr != NULL);
        std::string content;
        message->SerializeToString(&content);
        std::stringstream ss;
        ss << "POST / HTTP/1.1\r\n"
            << "mes_name: " << message->GetTypeName() << "\r\n"
            << "Content-Length: " << content.length() << "\r\n"
            << "\r\n"
            << content;
        std::string resp_str = ss.str();
        svr->SendBack(cli_flow, (char*)(resp_str.c_str()), resp_str.length());
        return true;
    }

    std::string host_;
    int port_;
    int listen_fd_;
    void* rpc_list;
    int rpc_pop_index;
    int rpc_size;
    pthread_mutex_t _mux;
    ext_handler* _close_handler;
    void* _close_handler_param;
    std::vector<HttpHandler*> http_handler_list;
    int http_pop_index;
    bool has_http_handler;
};

#endif
