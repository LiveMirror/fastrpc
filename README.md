arpc_client_for_pc 为客户端使用 
<br/>
<br/>::google::protobuf::RpcChannel* client;
<br/>    echo::EchoService_Stub::Stub* stub;
<br/>    client = new RpcClient(2, "192.168.1.13", 8999, 5000); // 1:并发sock数 2:host 3:ip 4:超时时间
<br/>    if (!((RpcClient*)client)->IsConnected()) {
<br/>    	delete client;
<br/>		exit(0);
<br/>	} else {
<br/>		std::cout << "connect success\n";
<br/>	}
<br/>	((RpcClient*)client)->RegiExtProcesser(ext_processer, NULL); // 处理服务器主动推的消息
<br/>	((RpcClient*)client)->RegiCloseHandler(close_handler, NULL); // 断开事件处理
<br/>    stub = new echo::EchoService_Stub::Stub(client);
<br/>    request->set_message("client_hello");
<br/>    stub->Echo(NULL, request, response, callback); // 异步, callback为空则是同步
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>
<br/>arpc_server 为服务器使用
<br/>
<br/>class EchoServiceImpl : public echo::EchoService { // rpc 函数实现
<br/>    virtual void Echo(::google::protobuf::RpcController* controller,
<br/>                      const ::echo::EchoRequest* request,
<br/>                      ::echo::EchoResponse* response,
<br/>                      ::google::protobuf::Closure* done) {
<br/>
<br/>        response->set_response(request->message()+" server_hello");
<br/>        //RpcController* p_con = (RpcController*)controller;
<br/>        //unsigned cli_flow = p_con->_cli_flow;
<br/>        //CASyncSvr* svr = p_con->_svr;
<br/>        //RpcServer::PushToClient(svr, cli_flow, response); // 这是服务器额外主动推消息
<br/>        if (done) {
<br/>            done->Run();
<br/>        }
<br/>    }
<br/>};
<br/>
<br/>class MyHttpHandler : public HttpHandler { // 同时可处理http请求
<br/>public:
<br/>    virtual void Init(CASyncSvr* svr) {}
<br/>    virtual void OnRec(HttpRequest* request,
<br/>                       ::google::protobuf::Closure *done) {
<br/>        CHttpParser* ps = request->ps;
<br/>        ps->parse_form_body();
<br/>        std::string kk = ps->get_param("kk");
<br/>        //string str_cmd = ps->get_object();
<br/>        //string get_uri = ps->get_uri();
<br/>        //std::stringstream ss;
<br/>        //ss << "kk:" << kk << "<br/>"
<br/>        //    << "cmd:" << str_cmd << "<br/>"
<br/>        //    << "uri:" << get_uri << "<br/>";
<br/>
<br/>        std::string content_type = "text/html";
<br/>        std::string add_head = "Connection: keep-alive\r\n";
<br/>        CHttpResponseMaker::make_string(kk,
<br/>                                        request->response,
<br/>                                        content_type,
<br/>                                        add_head);
<br/>
<br/>        if (done) {
<br/>            done->Run();
<br/>        }
<br/>    }
<br/>    virtual void Finish(CASyncSvr* svr) {}
<br/>};
<br/>
<br/>
<br/>int close_handler(CASyncSvr* svr, unsigned cli_flow, void* param) {
<br/>    //std::stringstream ss;
<br/>    //ss << "svr_id:" << svr->_svr_id
<br/>    //    << " cli:" << cli_flow
<br/>    //    << " param:" << *((int*)param) << "\n";
<br/>    //printf(ss.str().c_str());
<br/>    return 0;
<br/>}
<br/>
<br/>int main(int argc, char *argv[])
<br/>{
<br/>    RpcServer server("192.168.1.13", 8999);
<br/>    RPCREGI_ONEP(server, EchoServiceImpl);
<br/>    HTTPREGI(server, MyHttpHandler);
<br/>    server.RegiClientCloseHandler(close_handler, NULL);
<br/>    server.start();
<br/>    return 0;
<br/>}
<br/>
<br/>参考各自目录下的main.cpp即可
<br/>
<br/>
<br/>欢迎bugfix，有协程编程经验同学将其改进成协程的编程方式