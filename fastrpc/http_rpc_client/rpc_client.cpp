// Copyright (c) 2013, feimat.
// All rights reserved.
//

#include "rpc_client.h"
#include "monitor.h"
#include "sstream"
#include "pbext.h"
#include "coroutine.h"

void FinishWait(Monitor* monitor) {
    monitor->Notify();
    return;
}

void PushCallBack(RpcClient &cli, ::google::protobuf::Closure* done) {
    RpcMgr::PutOutSideQueue(done);
}

void TimeCleaner::on_timer(XTimer* pTimer, uint32 id, void* ptr) {
        RpcCliMethod* rcm = rpcclient_->callback_map.Pop(key_);
        if (NULL == rcm) {
            return;
        }
        if (rcm->controller_) {
            ((CliController*)rcm->controller_)->is_timeout = true;
            ((CliController*)rcm->controller_)->SetFailed("time out");
        }
        ::google::protobuf::Closure* cro_done = rcm->cro_done_;
        if (cro_done) {
            PushCallBack(*rpcclient_, cro_done);
        } else {
            ::google::protobuf::Closure* done = rcm->done_;
            PushCallBack(*rpcclient_, done);
        }
        std::cout << key_ << "callback time out\n";
}

void RpcClient::SingleThreadMode(const ::google::protobuf::MethodDescriptor* method,
                           ::google::protobuf::RpcController* controller,
                           const ::google::protobuf::Message* request,
                           ::google::protobuf::Message* response,
                           ::google::protobuf::Closure* done) {
    std::string opcode = method->full_name();
    RpcCliMethod* cli_method = NULL;
    CroMgr mgr = GetCroMgr();
    int croid = coroutine_running(mgr);
    Monitor* monitor = NULL;
    if (NULL == done) { // done参数空表示同步模式
        ::google::protobuf::Closure* syn_done = NULL;
        ::google::protobuf::Closure* cro_done = NULL;
        bool need_copy = false;
        // 当前不是协程，用wait阻塞代替yield
        if (-1 == croid) {
            monitor = new Monitor();
            syn_done = ::google::protobuf::NewCallback(&FinishWait,monitor);
        } else { // 走协程,回调时resume
            cro_done = ::google::protobuf::NewCallback(&coroutine_resume, mgr, croid);
            need_copy = true; // 可能是局部变量，在协程外使用要复制一份
        }
        cli_method = new RpcCliMethod(controller,
                (::google::protobuf::Message*)request,
                response, syn_done, cro_done, need_copy);
    } else { // done非空表示异步模式
        // 再包装一层，异步回调执行时要用协程包着
        // 这样在里面再调用同步rpc时才可以用上yield
        ::google::protobuf::Closure* cro_closure =
            ::google::protobuf::NewCallback(&RpcMgr::RunWithCoroutine, done);
        cli_method = new RpcCliMethod(controller,
                (::google::protobuf::Message*)request,
                response, cro_closure, NULL);
    }
    unsigned newid = GetNextId();
    std::stringstream ss;
    ss << newid;
    std::string cli_id = ss.str();
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
    if (-1 != croid && NULL == done) {
        CroMgr mgr = GetCroMgr();
        coroutine_yield(mgr); // 同步协程模式要放权
        RpcCliMethod* rcm = cro_callback_map.Pop(cli_id);
        if (rcm) {
            if (controller && rcm->controller_) {
                ((CliController*)controller)->CopyFrom((CliController*)rcm->controller_);
            }
            if (!response->ParseFromArray(rcm->response_string.c_str(),
                                          rcm->response_string.size())) {
                if (controller) {
                    controller->SetFailed("parse response error");
                }
                printf("parse response error\n");
            }
            RpcSafeFree(rcm);
        }
    } else if (NULL != monitor) {
        // 同步非协程模式循环处理回调，直到该同步请求的回调被执行
        // 这样做等待时也可以干点活，但还是没有协程充分利用cpu
        while (!monitor->m_has_notify) {
            RpcClient::OutSideProcess(100000);
        }
    }
}

void RpcClient::MultiThreadMode(const ::google::protobuf::MethodDescriptor* method,
                           ::google::protobuf::RpcController* controller,
                           const ::google::protobuf::Message* request,
                           ::google::protobuf::Message* response,
                           ::google::protobuf::Closure* done) {
    std::string opcode = method->full_name();
    Monitor* monitor = NULL;
    RpcCliMethod* cli_method = NULL;
    // 多线程模式不支持协程，一个协程只能在一个线程内使用
    // 如果业务要多线程,那可能要加锁,协程内使用锁是很危险
    // 所以还不如多进程内单线程模式。
    CroMgr mgr = GetCroMgr();
    int croid = coroutine_running(mgr);
    assert(-1 == croid);
    if (NULL == done) { // sync mode
        monitor = new Monitor();
        ::google::protobuf::Closure* syn_done = ::google::protobuf::NewCallback(&FinishWait,
                monitor);
        cli_method = new RpcCliMethod(controller,
                (::google::protobuf::Message*)request,
                response, syn_done, NULL);
    } else { // async mode
        cli_method = new RpcCliMethod(controller,
                (::google::protobuf::Message*)request,
                response, done, NULL);
    }
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

void RpcClient::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                           ::google::protobuf::RpcController* controller,
                           const ::google::protobuf::Message* request,
                           ::google::protobuf::Message* response,
                           ::google::protobuf::Closure* done) {
    if (m_multi_thread_mode) {
        MultiThreadMode(method, controller, request, response, done);
    } else {
        SingleThreadMode(method, controller, request, response, done);
    }
}

ProcessType SendProcess(void *argument) {
    ParamItem &param = (  *((ParamItem*)argument) );
    RpcClient &cli = ( *((RpcClient*)param.cli) );
    XSocket &sock = ( *((XSocket*)param.sock) );
    while (!cli.isstop) {
        //if (!sock.can_send(cli.time_out_)) {
        //    continue;
        //}
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

void ProcessReturnData(std::string data, RpcClient* pcli) {
    RpcClient& cli = *pcli;
    CHttpParser ps;
    int ret = ps.parse_head(data.c_str(), data.length());
    if ( ret <= 0 ) {
        std::cout << data << " len: " << data.length() << " parse head error\n";
        return;
    }
    string cli_id = ps.get_head_field("cli_id");
    int content_len = ps.getContentLen();
    int headlen = data.length() - content_len;
    const char* content = data.c_str() + headlen;
    if (cli_id.empty() && cli._ext_processer) {
        std::string mes_name = ps.get_head_field("mes_name");
        std::string content_str;
        content_str.assign(content, content_len);
        ::google::protobuf::Closure* done = pbext::NewCallback(cli._ext_processer,
                mes_name, content_str, cli._ext_param);
        PushCallBack(cli, done);
        return;
    }
    RpcCliMethod* rcm = cli.callback_map.Pop(cli_id);
    if (NULL == rcm) {
        std::cout << " cli: " << cli_id << "null callback error\n";
        return;
    }
    ::google::protobuf::Closure* cro_done = rcm->cro_done_;
    if (cro_done) {
        rcm->response_string.assign(content, content_len);
        cli.cro_callback_map.Insert(cli_id, rcm);
        cro_done->Run(); // 协程resume,rcm在原处释放
        return;
    }
    ::google::protobuf::Message* response = rcm->response_;
    if (!response->ParseFromArray(content, content_len)) {
        if (rcm->controller_) {
            rcm->controller_->SetFailed("parse response error");
        }
        printf("parse response error\n");
    }
    ::google::protobuf::Closure* done = rcm->done_;
    PushCallBack(cli, done);
    delete rcm;
}

ProcessType ReadProcess(void *argument) {
    ParamItem &param = (  *((ParamItem*)argument) );
    RpcClient &cli = ( *((RpcClient*)param.cli) );
    XSocket &sock = ( *((XSocket*)param.sock) );
    int buf_size = 1024*100;
    char *buf = (char*)malloc(buf_size);
    char *last_left = buf;
    int left_len = 0;
    char* http_head = NULL;
    while (!cli.isstop) {
        int one_len = sock.recv_one_http(buf, buf_size, last_left, left_len, http_head, cli.time_out_);
        if (one_len > 0) {
            std::string data;
            data.assign(http_head, one_len);
            ::google::protobuf::Closure* done = ::google::protobuf::NewCallback(&ProcessReturnData,
                                                                                data, &cli);
            PushCallBack(cli, done);
            //ProcessReturnData(data, cli);
        }
        else if (one_len < 0 && !cli.isstop){
            sock.mutex_close();
            if (cli._close_handler) {
                ::google::protobuf::Closure* done = ::google::protobuf::NewCallback(cli._close_handler,
                        cli._close_handler_param);
                PushCallBack(cli, done);
            }
            param.consem.wait();
        }
    }
    free(buf);
    return NULL;
}

ProcessType CallBackProcess(void *argument) {
    ParamItem &param = (  *((ParamItem*)argument) );
    RpcClient &cli = ( *((RpcClient*)param.cli) );
    while (!cli.isstop) {
        RpcClient::OutSideProcess(100000);
    }
    return NULL;
}

int RpcClient::OutSideProcess(unsigned msec) {
    ::google::protobuf::Closure* done = RpcMgr::GetOutSideQueue(msec);
    if (NULL == done) {
        return 0;
    }
    done->Run();
    return 1;
}
