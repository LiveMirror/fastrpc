简单易用，几乎和调用本地方法一样使用即可。
<br/>使用protobuf生成代码 支持同时处理pbrpc调用和http请求
<br/>客户端支持 windows linux android使用
<br/>支持同步异步方式调用 支持单向推送 
<br/>支持闲时关闭链接 支持自动重连与断开事件处理
<br/>同步压测 4核机子虚拟机
<br/>简单echo(因为nginx是读文件公平竞争 我们也是读文件) 1000并发 6wqps; nginx 4.5wqps
<br/>异步压测1000并发 40+w qps
<br/>另有400多行实现的 python web服务器，原理一样的，以供参考（通用的多线程reactor模式，用非阻塞模拟异步io）
<br/>http://www.oschina.net/p/fastpy
<br/>
<br/>例子
<br/>
<br/>首先定义protobuf 协议
<br/>
<br/>package echo;
<br/>message EchoRequest
<br/>{
<br/>required string message = 1;
<br/>};
<br/>message EchoResponse
<br/>{
<br/>required string response = 1;
<br/>};
<br/>service EchoService
<br/>{
<br/>rpc Echo(EchoRequest) returns (EchoResponse);
<br/>};
<br/>option cc_generic_services = true;
<br/>
<br/>然后客户端调用：
<br/>RpcClient client(10, "192.168.1.13", 8999, 1000); // 1:并发sock数 2:host 3:ip 4:超时时间
<br/>echo::EchoService::Stub stub(&client);
<br/>
<br/>echo::EchoRequest req;
<br/>req.set_message("cli hello");
<br/>echo::EchoResponse res;
<br/>stub.Echo(NULL, &req, &res, NULL); // 同步
<br/>
<br/>服务器调用：
<br/>class EchoServiceImpl : public echo::EchoService { // 实现pb生成的接口
<br/>    virtual void Echo(::google::protobuf::RpcController* controller,
<br/>                      const ::echo::EchoRequest* request,
<br/>                      ::echo::EchoResponse* response,
<br/>                      ::google::protobuf::Closure* done) {
<br/>        response->set_response(request->message()+" server_hello");
<br/>        if (done) {
<br/>            done->Run();
<br/>        }
<br/>    }
<br/>};
<br/>
<br/>RpcServer server("192.168.1.13", 8999);
<br/>RPCREGI_ONEP(server, EchoServiceImpl);
<br/>server.start();
<br/>
<br/>服务器http 处理：
<br/>
<br/>class MyHttpHandler : public HttpHandler {
<br/>public:
<br/>    virtual void Init(CASyncSvr* svr) {}
<br/>    virtual void OnRec(HttpRequest* request,
<br/>                       ::google::protobuf::Closure *done) {
<br/>        CHttpParser* ps = request->ps;
<br/>        ps->parse_form_body(); // 解析post数据
<br/>        std::string kk = ps->get_param("kk");
<br/>        std::string res = kk + " server hello":
<br/>        std::string content_type = "text/html";
<br/>        std::string add_head = "Connection: keep-alive\r\n";
<br/>        CHttpResponseMaker::make_string(kk,
<br/>                                        request->response,
<br/>                                        content_type,
<br/>                                        add_head);
<br/>        if (done) {
<br/>            done->Run();
<br/>        }
<br/>    }
<br/>    virtual void Finish(CASyncSvr* svr) {}
<br/>};
<br/>
<br/>RpcServer server("192.168.1.13", 8999);
<br/>RPCREGI(server, EchoServiceImpl);
<br/>HTTPREGI(server, MyHttpHandler);
<br/>server.start();
<br/>
<br/>浏览器访问 http://192.168.1.13:8999/xxx?kk=hello
<br/> 更多丰富功能请参阅代码，如果要用到python开发包请参考这里安装http://web.49jie.com/?p=1644