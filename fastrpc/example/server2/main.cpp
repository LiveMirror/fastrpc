#include "rpc_server.h"
#include "echo.pb.h"
#include "rpc_client.h"
#include "xcore_resource_pool.h"
#include <sstream>

class EchoServiceImpl : public echo::EchoService {

    void PeriodPush(CASyncSvr* svr, unsigned cli_flow) {
        ::echo::EchoResponse response;
        response.set_response("period push mes");
        RpcServer::PushToClient(svr, cli_flow, &response); // 这是服务器额外主动推消息
    }

    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {

        response->set_response(response->response()+" add server2 echo");

        printf("recv request from server1\n");

        //// 演示定时推送
        //RpcController* p_con = (RpcController*)controller;
        //unsigned cli_flow = p_con->_cli_flow;
        //CASyncSvr* svr = p_con->_svr;
        //Closure<void>* period_job =
        //    NewPermanentClosure(this, &EchoServiceImpl::PeriodPush, svr, cli_flow); // 注意这里一定要用permanentclosure

        //timer_mgr.AddJob(1000, period_job, 5);

        if (done) {
            done->Run();
        }
    }
    virtual void Dummy(::google::protobuf::RpcController* controller,
                       const ::echo::DummyRequest* request,
                       ::echo::DummyResponse* response,
                       ::google::protobuf::Closure* done) {
        if (done) {
            done->Run();
        }
    }
public:
    RpcServer* _rpc_server;
    TimerMgr timer_mgr;

public:
    EchoServiceImpl(RpcServer* rpc_server) {
        _rpc_server = rpc_server;
    }
};

class MyHttpHandler : public HttpHandler {
public:
    ResourcePool<int> res_pool;
    MyHttpHandler() {
        // 测试资源池（数据库连接池可类似使用）
        for (int i = 0; i < 16; ++i) {
            int* res = new int;
            *res = i;
            res_pool.PutResQue(res);
        }
    }
    ~MyHttpHandler() {
        for (int i = 0; i < 16; ++i) {
            int* res = res_pool.GetResQue();
            delete res;
        }
    }

    virtual void Init(CASyncSvr* svr) {}
    virtual void OnRec(HttpRequest* request,
                       ::google::protobuf::Closure *done) {
        CHttpParser* ps = request->ps;
        ps->parse_form_body();
        std::string kk = ps->get_param("kk");
        string str_cmd = ps->get_object();
        string get_uri = ps->get_uri();
        std::stringstream ss;
        ss << "kk:" << kk << "<br/>"
            << "cmd:" << str_cmd << "<br/>"
            << "uri:" << get_uri << "<br/>";

        int* res = res_pool.Get();
        ss << " add res:" << *res << "<br/>"
            << "final";
        res_pool.Release(res);

        std::string content_type = "text/html";
        std::string add_head = "Connection: keep-alive\r\n";
        CHttpResponseMaker::make_string(ss.str(),
                                        request->response,
                                        content_type,
                                        add_head);

        if (done) {
            done->Run();
        }
    }
    virtual void Finish(CASyncSvr* svr) {}
};

int close_handler(CASyncSvr* svr, unsigned cli_flow, void* param) {
    std::stringstream ss;
    ss << "svr_id:" << svr->_svr_id
        << " cli:" << cli_flow
        << " param:" << *((int*)param) << "\n";
    //printf(ss.str().c_str());
    return 0;
}

int main(int argc, char *argv[])
{
    RpcServer server("127.0.0.1", 8996);
    ::google::protobuf::Service *rpc_service = new EchoServiceImpl(&server);
    server.RegiService(rpc_service);
    HttpHandler *http_handler = new MyHttpHandler();
    server.RegiHttpHandler(http_handler);
    int p = 123;
    server.RegiClientCloseHandler(close_handler, &p);
    server.start();

    return 0;
}
