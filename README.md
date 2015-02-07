简单易用，几乎和调用本地方法一样使用即可。
使用protobuf生成代码 支持同时处理pbrpc调用和http请求
客户端支持 windows linux android使用
支持同步异步方式调用 支持单向推送 
支持闲时关闭链接 支持自动重连与断开事件处理
同步压测 4核机子虚拟机
简单echo(因为nginx是读文件公平竞争 我们也是读文件) 1000并发 6wqps; nginx 4.5wqps
异步压测1000并发 40+w qps
另有400多行实现的 python web服务器 以供参考
http://www.oschina.net/p/fastpy

例子

首先定义protobuf 协议

package echo;
message EchoRequest
{
required string message = 1;
};
message EchoResponse
{
required string response = 1;
};
service EchoService
{
rpc Echo(EchoRequest) returns (EchoResponse);
};
option cc_generic_services = true;

然后客户端调用：
RpcClient client(10, "192.168.1.13", 8999, 1000); // 1:并发sock数 2:host 3:ip 4:超时时间
echo::EchoService::Stub stub(&client);

echo::EchoRequest req;
req.set_message("cli hello");
echo::EchoResponse res;
stub.Echo(NULL, &req, &res, NULL); // 同步

服务器调用：
class EchoServiceImpl : public echo::EchoService { // 实现pb生成的接口
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {
        response->set_response(request->message()+" server_hello");
        if (done) {
            done->Run();
        }
    }
};

RpcServer server("192.168.1.13", 8999);
RPCREGI_ONEP(server, EchoServiceImpl);
server.start();

服务器http 处理：

class MyHttpHandler : public HttpHandler {
public:
    virtual void Init(CASyncSvr* svr) {}
    virtual void OnRec(HttpRequest* request,
                       ::google::protobuf::Closure *done) {
        CHttpParser* ps = request->ps;
        ps->parse_form_body(); // 解析post数据
        std::string kk = ps->get_param("kk");
        std::string res = kk + " server hello":
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

RpcServer server("192.168.1.13", 8999);
RPCREGI(server, EchoServiceImpl);
HTTPREGI(server, MyHttpHandler);
server.start();

浏览器访问 http://192.168.1.13:8999/xxx?kk=hello