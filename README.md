arpc_client_for_pc 为客户端使用 

::google::protobuf::RpcChannel* client;
    echo::EchoService_Stub::Stub* stub;
    client = new RpcClient(2, "192.168.1.13", 8999, 5000); // 1:并发sock数 2:host 3:ip 4:超时时间
    if (!((RpcClient*)client)->IsConnected()) {
		delete client;
		exit(0);
	} else {
		std::cout << "connect success\n";
	}
	((RpcClient*)client)->RegiExtProcesser(ext_processer, NULL); // 处理服务器主动推的消息
	((RpcClient*)client)->RegiCloseHandler(close_handler, NULL); // 断开事件处理
    stub = new echo::EchoService_Stub::Stub(client);
    request->set_message("client_hello");
    stub->Echo(NULL, request, response, callback); // 异步, callback为空则是同步












arpc_server 为服务器使用

class EchoServiceImpl : public echo::EchoService { // rpc 函数实现
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {

        response->set_response(request->message()+" server_hello");
        //RpcController* p_con = (RpcController*)controller;
        //unsigned cli_flow = p_con->_cli_flow;
        //CASyncSvr* svr = p_con->_svr;
        //RpcServer::PushToClient(svr, cli_flow, response); // 这是服务器额外主动推消息
        if (done) {
            done->Run();
        }
    }
};

class MyHttpHandler : public HttpHandler { // 同时可处理http请求
public:
    virtual void Init(CASyncSvr* svr) {}
    virtual void OnRec(HttpRequest* request,
                       ::google::protobuf::Closure *done) {
        CHttpParser* ps = request->ps;
        ps->parse_form_body();
        std::string kk = ps->get_param("kk");
        //string str_cmd = ps->get_object();
        //string get_uri = ps->get_uri();
        //std::stringstream ss;
        //ss << "kk:" << kk << "<br/>"
        //    << "cmd:" << str_cmd << "<br/>"
        //    << "uri:" << get_uri << "<br/>";

        std::string content_type = "text/html";
        std::string add_head = "Connection: keep-alive\r\n";
        CHttpResponseMaker::make_string(kk,
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
    //std::stringstream ss;
    //ss << "svr_id:" << svr->_svr_id
    //    << " cli:" << cli_flow
    //    << " param:" << *((int*)param) << "\n";
    //printf(ss.str().c_str());
    return 0;
}

int main(int argc, char *argv[])
{
    RpcServer server("192.168.1.13", 8999);
    RPCREGI_ONEP(server, EchoServiceImpl);
    HTTPREGI(server, MyHttpHandler);
    server.RegiClientCloseHandler(close_handler, NULL);
    server.start();
    return 0;
}

参考各自目录下的main.cpp即可


欢迎bugfix，有协程编程经验同学将其改进成协程的编程方式
