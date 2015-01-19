// Copyright (c) 2014, feimat.
// All rights reserved.
//

#include "rpc_client.h"
#include "http_util.h"
#include "monitor.h"
#include "sstream"
#include "rpc_client_sock.h"
#include "xcore_atomic.h"
#include "http_server_codec.h"

void FinishWait(Monitor* monitor) {
    monitor->Notify();
    return;
}

RpcClient::RpcClient(int workernum, const string &host, const int port, int time_out) {
    timer.start();
    isstop = false;
    host_ = host;
    port_ = port;
    sock_num_ = workernum;
    time_out_ = time_out;
    cli_id = 0;
    next_pool_index = 0;
    pthread_mutex_init(&_readmux,NULL);
    if((sem_init(&_readsem,0,0))<0){
        printf("read sem init fail");
        exit(1);
    }
    pthread_mutex_init(&_id_mux,NULL);
    pthread_mutex_init(&_pool_mux,NULL);
    for (int i =0; i < sock_num_; ++i) {
        RpcClientSock* rcsock = new RpcClientSock(this, 10000, 10000, 10000);
        rcsock->Start();
        sock_list.push_back(rcsock);
    }
    for (int i = 0; i < sock_num_; ++i) {
        pthread_t tid = 0;
        pthread_create(&tid,NULL,RpcClient::CallBackProcess,this);
        callback_thread_list.push_back(tid);
    }
}

RpcClient::~RpcClient() {
    CloseAll();
}

void RpcClient::CloseAll() {
    isstop = true;
    for (int i = 0; i < sock_num_; ++i) {
        sem_post(&_readsem);
    }
    callback_thread_list.clear();
    pthread_mutex_destroy(&_readmux);
    pthread_mutex_destroy(&_id_mux);
    pthread_mutex_destroy(&_pool_mux);
    sem_destroy(&_readsem);
    timer.stop();
}

void TimeCleaner::on_timer(XTimer* pTimer, uint32 id, void* ptr) {
        RpcCliMethod* rcm = rpcclient_->callback_map.Pop(key_);
        if (NULL == rcm) {
            return;
        }
        ::google::protobuf::Closure* done = rcm->done_;
        done->Run();
        std::cout << key_ << "callback time out\n";
}

void RpcClient::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                           ::google::protobuf::RpcController* controller,
                           const ::google::protobuf::Message* request,
                           ::google::protobuf::Message* response,
                           ::google::protobuf::Closure* done) {
    std::string opcode = method->full_name();
    Monitor* monitor = NULL;
    if (NULL == done) {
        monitor = new Monitor();
        done = ::google::protobuf::NewCallback(&FinishWait,
                                               monitor);
    }
    RpcCliMethod* cli_method = new RpcCliMethod((::google::protobuf::Message*)request,
                                                response,
                                                done);
    unsigned newid = GetNextId();
    std::stringstream ss;
    ss << newid;
    callback_map.Insert(ss.str(), cli_method);
    TimeCleaner* tc = new TimeCleaner(this, ss.str());
    std::string content;
    request->SerializeToString(&content);
    ss.str("");
    ss << "POST / HTTP/1.1\r\n"
        << "op: " << opcode << "\r\n"
        << "cli_id: " << newid << "\r\n"
        << "Content-Length: " << content.length() << "\r\n"
        << "\r\n"
        << content;
    timer.schedule(tc, (uint32)time_out_);
    RpcClientSock* cli = GetRandomPoolIndex();
    cli->PushData(ss.str());

    if (NULL != monitor) {
        monitor->Wait();
        delete monitor;
    }

}

void *RpcClient::CallBackProcess(void *argument) {
    RpcClient &cli = ( *((RpcClient*)argument) );
    while (!cli.isstop) {
        std::string data = cli.GetReadQueue();
        if (data.empty()) continue;
        CHttpParser ps;
        int ret = ps.parse_head(data.c_str(), data.length());
        if ( ret <= 0 ) {
            printf("parse head fail,data:%s\n",data.c_str());
            continue;
        }
        string cli_id = ps.get_head_field("cli_id");
        RpcCliMethod* rcm = cli.callback_map.Pop(cli_id);
        if (NULL == rcm) {
            printf("rpc client method is null\n");
            continue;
        }
        int content_len = ps.getContentLen();
        int headlen = data.length() - content_len;
        const char* content = data.c_str() + headlen;
        ::google::protobuf::Message* response = rcm->response_;
        if (!response->ParseFromArray(content, content_len)) {
        }
        ::google::protobuf::Closure* done = rcm->done_;
        done->Run();
    }
    return NULL;
}
