#include "rpc_server.h"
#include "echo.pb.h"
#include "rpc_client.h"

class EchoServiceImpl : public echo::EchoService {

    void PeriodPush(CASyncSvr* svr, unsigned cli_flow) {
        ::echo::EchoResponse response;
        response.set_response("period push mes");
        RpcServer::PushToClient(svr, cli_flow, &response); // 这是服务器额外主动推消息
    }

    void TestThreadPool(::echo::EchoResponse* response) {
        response->set_response(response->response() + " add tp echo");
    }

    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {

        // 再向server2发请求这里是协程同步会放权，不用担心阻塞
        echo_service->Echo(NULL, request, response, NULL);

        response->set_response(response->response()+" add server1 echo");

        // 演示定时推送
        RpcController* p_con = (RpcController*)controller;
        unsigned cli_flow = p_con->_cli_flow;
        CASyncSvr* svr = p_con->_svr;
        Closure<void>* period_job =
            NewPermanentClosure(this, &EchoServiceImpl::PeriodPush, svr, cli_flow); // 注意这里一定要用permanentclosure

        timer_mgr.AddJob(1000, period_job, 5);


        // 切换到多线程执行,放权,等线程池执行完后resume
        tp_mgr->TPRun(this, &EchoServiceImpl::TestThreadPool, response);

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
    RpcClient* m_rpc_client;
    echo::EchoService_Stub::Stub* echo_service;
    TimerMgr timer_mgr;
    TPMgr* tp_mgr;
public:
    EchoServiceImpl(RpcServer* rpc_server,
            ::google::protobuf::RpcChannel* a_rpc_client) {
        _rpc_server = rpc_server;
        m_rpc_client = (RpcClient*)a_rpc_client;
        echo_service = new echo::EchoService_Stub::Stub(m_rpc_client);
        tp_mgr = new TPMgr(3);
    }
};

class MyHttpHandler : public HttpHandler {
public:
    MyHttpHandler(::google::protobuf::RpcChannel* a_rpc_client) {
        m_rpc_client = (RpcClient*)a_rpc_client;
        echo_service = new echo::EchoService_Stub::Stub(m_rpc_client);
        tp_mgr = new TPMgr(2);
    }

    virtual void Init(CASyncSvr* svr) {}
    void test_tp_run(HttpRequest* request,
            ::google::protobuf::Closure *done) {

        ::echo::EchoRequest req;
        ::echo::EchoResponse res;
        req.set_message("browse req");

        // 再向server2发请求这里是协程同步会放权，不用担心阻塞
        echo_service->Echo(NULL, &req, &res, NULL);


        CHttpParser* ps = request->ps;
        ps->parse_form_body();
        std::string kk = ps->get_param("kk");
        string str_cmd = ps->get_object();
        string get_uri = ps->get_uri();
        std::stringstream ss;
        ss << "kk:" << kk << "<br/>"
            << "cmd:" << str_cmd << "<br/>"
            << "uri:" << get_uri << "<br/>"
            << "rpc res:" << res.DebugString();

        std::string content_type = "text/html";
        std::string add_head = "Connection: keep-alive\r\n";
        CHttpResponseMaker::make_string(res.response(),
                                        request->response,
                                        content_type,
                                        add_head);

        if (done) {
            done->Run();
        }
    }

    virtual void OnRec(HttpRequest* request,
                       ::google::protobuf::Closure *done) {
        // 扔到线程池去执行，线程池中rpc调用也会同步转异步
        tp_mgr->TPRun(this, &MyHttpHandler::test_tp_run, request, done);
        //test_tp_run(request, done);
    }
    virtual void Finish(CASyncSvr* svr) {}

    RpcClient* m_rpc_client;
    echo::EchoService_Stub::Stub* echo_service;
    TPMgr* tp_mgr;
};

int close_handler(CASyncSvr* svr, unsigned cli_flow, void* param) {
    std::stringstream ss;
    ss << "svr_id:" << svr->_svr_id
        << " cli:" << cli_flow
        << " param:" << *((int*)param) << "\n";
    //printf(ss.str().c_str());
    return 0;
}

// mes name: echo.EchoResponse
void ext_processer(std::string mes_name, std::string data, void* param) {
    ::google::protobuf::Message* response = PbMgr::CreateMessage(mes_name);
    if (response) {
        response->ParseFromString(data);
        std::cout << "recv push mes, name:" << mes_name
            << " data:" << response->DebugString();
    }
}

int StartFun(int fd) {
    // 多进程模式创建server1
    RpcServer server(fd);

    // 创建rpcclient和server2(8998端口)通信
    ::google::protobuf::RpcChannel* client =
        new RpcClient("127.0.0.1", 8998, 5000); // 1::host 2:port 3:超时时间
    ((RpcClient*)client)->RegiExtProcesser(ext_processer, NULL); // 处理服务器主动推的消息
    // 注册rpc服务
    ::google::protobuf::Service *rpc_service = new EchoServiceImpl(&server, client);
    server.RegiService(rpc_service);
    // 注册http服务
    HttpHandler *http_handler = new MyHttpHandler(client);
    server.RegiHttpHandler(http_handler);
    int p = 123;
    server.RegiClientCloseHandler(close_handler, &p);
    server.start();
}

int main(int argc, char *argv[])
{
    // 多进程方式启动 ip, port, 启动函数指针, 启动进程个数(不填则自动为cpu个数)
    StartWithMultProc("127.0.0.1", 8997, StartFun, 2);

    return 0;
}
