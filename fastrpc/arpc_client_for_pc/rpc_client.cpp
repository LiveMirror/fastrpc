// Copyright (c) 2014, feimat.
// All rights reserved.
//

#include "rpc_client.h"
#include "monitor.h"
#include "sstream"
//#include "CNetfun.h"
//#include "CRWCache.h"
//#include "http_server_codec.h"

void FinishWait(Monitor* monitor) {
    monitor->Notify();
    return;
}

void TimeCleaner::on_timer(XTimer* pTimer, uint32 id, void* ptr) {
        RpcCliMethod* rcm = rpcclient_->callback_map.Pop(key_);
        if (NULL == rcm) {
            return;
        }
        if (rcm->controller_) {
            ((CliController*)rcm->controller_)->is_timeout = true;
        }
        ::google::protobuf::Closure* done = rcm->done_;
        done->Run();
        delete rcm;
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
    RpcCliMethod* cli_method = new RpcCliMethod(controller,
                                                (::google::protobuf::Message*)request,
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
    PutSendQueue(ss.str());
    if (NULL != monitor) {
        monitor->Wait();
        delete monitor;
    }

}

ProcessType SendProcess(void *argument) {
    ParamItem &param = (  *((ParamItem*)argument) );
    RpcClient &cli = ( *((RpcClient*)param.cli) );
    XSocket &sock = ( *((XSocket*)param.sock) );
    while (!cli.isstop) {
        if (!sock.can_send(cli.time_out_))
            continue;
        std::string data = cli.GetSendQueue();
        if (data.empty())
            continue;
        while (true) {
            int ret = sock.send_n(data.c_str(), data.length(), cli.time_out_);
            if (ret > 0) {
                break;
            }
            else if (ret < 0 && !cli.isstop){
                ret = sock.reconnect(XSockAddr(cli.host_, cli.port_));
                if (ret < 0) {
                    printf("stop for remote connect fail\n");
                    cli.isstop = true;
                    cli._is_connected = false;
                    break;
                }
                param.consem.post();
            }
        }
    }
    return NULL;
}

ProcessType ReadProcess(void *argument) {
    ParamItem &param = (  *((ParamItem*)argument) );
    RpcClient &cli = ( *((RpcClient*)param.cli) );
    XSocket &sock = ( *((XSocket*)param.sock) );
    int buf_size = 1024*100;
    char *buf = (char*)malloc(buf_size);
    //char* buf = new char[buf_size];
    char *last_left = buf;
    int left_len = 0;
    char* http_head = NULL;
    while (!cli.isstop) {
        int one_len = sock.recv_one_http(buf, buf_size, last_left, left_len, http_head, cli.time_out_);
        if (one_len > 0) {
            std::string data;
            data.assign(http_head, one_len);
            cli.PutReadQueue(data);
        }
        else if (one_len < 0 && !cli.isstop){
            sock.mutex_close();
            if (cli._close_handler) {
                cli._close_handler(cli._close_handler_param);
            }
            param.consem.wait();
            /*int ret = sock.reconnect(XSockAddr(cli.host_, cli.port_));
              if (ret < 0) {
              printf("stop for remote connect fail\n");
              cli.isstop = true;
              cli._is_connected = false;
              break;
              }*/
        }
    }
    free(buf);
    //delete []buf;
    return NULL;
}

std::string first_line = "POST / HTTP/1.1";

ProcessType CallBackProcess(void *argument) {
    ParamItem &param = (  *((ParamItem*)argument) );
    RpcClient &cli = ( *((RpcClient*)param.cli) );
    while (!cli.isstop) {
        std::string data = cli.GetReadQueue();
        if (data.empty()) {
            continue;
        }
        CHttpParser ps;
        //data = data.replace(0,15, first_line);
        int ret = ps.parse_head(data.c_str(), data.length());
        if ( ret <= 0 ) {
            std::cout << data << " len: " << data.length() << " parse head error\n";
            continue;
        }
        string cli_id = ps.get_head_field("cli_id");
        if (cli_id.empty() && cli._ext_processer) {
            cli._ext_processer(ps, data,cli._ext_param);
            continue;
        }
        RpcCliMethod* rcm = cli.callback_map.Pop(cli_id);
        if (NULL == rcm) {
            std::cout << " cli: " << cli_id << "null callback error\n";
            continue;
        }
        int content_len = ps.getContentLen();
        int headlen = data.length() - content_len;
        const char* content = data.c_str() + headlen;
        ::google::protobuf::Message* response = rcm->response_;
        if (!response->ParseFromArray(content, content_len)) {
            printf("parse response error\n");
        }
        ::google::protobuf::Closure* done = rcm->done_;
        done->Run();
        delete rcm;
    }
    return NULL;
}
