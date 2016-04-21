用过go erlang gevent的亲们应该都会知道协程在应用中带来的方便。

如果对协程不理解的同学，通过阅读下面例子可以快速了解我们框架的协程的意义，已了解的可以跳过这部分。

协程例子：假设我们要发个Get请求获取百度首页内容；

php同步方式：$result = file_get_contents("http://www.baidu.com"), php果然是世界上最好的语言，多么简洁。

然后java和c++的同学开始不屑了: "呵呵, 同步，鄙视你不解释。"

好了那就异步吧，php也有异步的方式啦，例如使用腾讯的swoole框架，

<?php
$cli = new swoole_http_client('127.0.0.1', 8993);
$cli->get('ww.baidu.com', function ($cli) {
    echo $cli->body;
});
 执行到get请求时，当前进程不阻塞等待，可以继续去处理后面的事情，等到响应结果返回再去执行回调function里的代码。

这样是不是就完美了呢，我们来看看异步的问题：

这里这个代码被拆分成两个块调用$cli->get函数之前块和回调函数里面的块，问题就出在这里，现在假设有一个需求，我要给发Get请求之前的一个html变量追加百度返回的内容，那么我必须这么做：

<?php
$html = "init";
$cli = new swoole_http_client('127.0.0.1', 8993);
$cli->html = $html;
$cli->get('w.baidu.com', function ($cli) {
    echo $cli->html.$cli->body;
});
也就是说get之前和回调函数里是两个不同的代码区域，要在两个不同代码区域连续使用一个局部变量，那么我们的做法就是必须传值(传值有很多种方式了，这里不一一列举).

也许有的同学会说就传个值而已嘛，也没什么不好的呀。好那么我们来看看一段实际项目中的代码（不用真的去看懂）：

//代码块1
$url = $_SERVER['HTTP_HOST'] . $_SERVER["REQUEST_URI"];
$this->history_add( $wid, $sku_id, 2 );
$smarty = get_smarty();
$sku = new Sku( $sku_id );
$goods = new Goods( $sku->goods_id );
$smarty->assign('pinlei_name', $goods->goods_name );
$smarty->assign("title_name", PageListWorksItem::trans_goods_name( $goods->goods_name ));
$this->goods_id = $goods->goods_id;
$work = new Works( $wid ); 
$work_data = $work->get_data();
$s = new ImgArgs( $work_data );
//插入点 在这里插入获取百度首页内容
//代码块2
$merge_img_url = $s->get_product_merge_img_url(  $sku_id , 2  ); 
$work_data['img_url'] = $merge_img_url;
$work_data['real_price'] = $work_data['money'] + $sku->price;
// $reply_count = WorksComment::get_works_comment_list_count2(  $sku->goods_id );
$reply_count = WorksComment::get_works_comment_list_count(  $sku->goods_id );
$smarty->assign('reply_count', $reply_count );
...
后面代码省略，其中以上所有对象和变量在后续代码中都使用到
 在上面插入点这个地方 我们需要调用异步发请求去获取百度首页内容，然后在代码块2中使用到百度的响应结果，

这样我们就要把代码块2这部分全部迁移到回调函数中，接着修改的同学就要抓狂了，要把代码块1中一个一个变量去检查，看后续是否用到，然后一个一个去传值给回调函数。

同理，在中间插入点，我们要做其它操作，例如查mysql 查redis时，同样会出现上述 同步和异步的选择困难问题。

 好，到这里使用jquery的同学表示不服了，这有什么难，看我大jquery：

function asynGetBaidu(){ 
        var url = "xxx";
        var html = "init";
        $.get(url, {}, function(data,status) {
            alert(html+data);
        });
}
 你看，不是不需要传值在回调内也可以使用外部变量吗？

这是因为，jquery在底层封装了传值，把asynGetBaidu函数内的变量都可以在回调函数内作用。

那么问题又来了，如果原来函数是有返回值的，我们在这加了异步get后返回值还能有效吗？

 function asynGetBaidu(){ 
        var url = "xxx";
        var html = "init";
        $.get(url, {}, function(data,status) {
            html = html+data;
            return html；
        });
}
var res = asynGetBaidu();
alert(res);
 弹出结果是undefine。有人又说了，真无聊，网页端可以通过全局变量，或网页标签取返回值就行了，这里返回值没什么意思啦。

是的对网页端来说jquery功能完全够用了，但是对服务器转发请求来说，我们又能设多少个这样的全局变量呢？

那怎么办呢？我们假设场景是这样: 在服务器然后端 processer函数用来处理客户端来的http请求，然后再转向百度获取信息，打印返回给客户端。

来看看经过我们封装后python的实现（人生苦短，我用python）：

import gevent.monkey
gevent.monkey.patch_socket() 
def asynGetBaidu():
    html = "init"
    ...
    f=urllib.urlopen("ww.baidu.com")
    ...
    return html + f.read()
    
def processer(self, request, response):
    response = asynGetBaidu()
    print response
 坑谁呢，这明明就是同步发GET请求。

没错这就是协程的作用了，同步的编码方式，异步的效果。上述代码要在我们fastpy框架下使用才会真的起到异步的效果。

每当有一个http请求到来时，框架会新开一个协程，这个协程执行processer函数，当执行到urllib.urlopen时，因为有io阻塞会主动放弃对cpu的使用权，让给谁呢？让给下一个协程，下个协程又会执行processer去处理下一个http请求。

等到baidu有响应结果了，框架引擎会恢复之前暂停的协程，继续执行processer内剩余的代码，这样我们想在asynGetBaidu中插入多少段请求别的服务的代码都只需要像同步一样编写，不需要调整上下文代码结构。

试想想，urlopen处如果是mysql query或redis的get，都可以0代价的同步自动转换为异步，多么方便的一件事啊。

 协程主要原理：当代码执行带io阻塞时，把当前进程的调用的服务函数里的局部变量和函数执行堆栈拷贝到别的地方，等到io返回就绪后，再把原来的堆栈内容拷贝回来，然后既可以实现从当前代码断点继续执行，其实和异步中等待执行回调的方式是类似的，只不过这样封装后就不用再显示声明回调函数了。

 

说到底，我们其实就是在抄go语言的特性啦，而且抄的远远不如那样， 是不是感觉我们在为go做广告。

有人会说了，go语言那么好，那你们为何不用啊。---回答：在国内不好找工作啊。

言归正传，看看我们团队c++和python的两个协程解决方案：

python解决方案： fastpy -————本框架是在gevent基础上封装而成，主要面向web应用：

源代码只有800多行 项目地址： https://git.oschina.net/feimat/fastpy

性能比较如下

tornado 4kqps 多进程1wqps 
nginx+tornado 9kqps
nginx+uwsgi 8kqps

django和webpy 原生性能较差

本server 2w qps 
欢迎加入qq群339711102，一起探讨优化哦

快速入门：

1、启动：
   指定监听端口即可启动
   python fastpy.py 8992 
   （如果需要使用gevent协程功能请先安装gevent，参考链接http://www.xue163.com/exploit/138/1381297.html）

2、快速编写cgi,支持运行时修改,无需重启server

   在fastpy.py同一目录下
   随便建一个python 文件
   例如:
   example.py:


   #-*- coding:utf-8 -*-
   import sys
   #定义一个同名example类
   #定义一个tt函数：
   reload(sys)
   sys.setdefaultencoding('utf8')
   FastpyAutoUpdate=True
   class example():
       def tt(self, request, response_head):
           #print request.form
           #print request.getdic
           #fileitem = request.filedic["upload_file"]
           #fileitem.filename
           #fileitem.file.read()
           return "ccb"+request.path
   则访问该函数的url为 http://ip:port/example.tt
   协程的使用上面已经演示过这里不再重复，详情请看代码示例
    cgi所有使用例子:
    sample.py  上传文件和form表单使用等基本api的例子
    example.py 使用单例模式和线程+异步返回的例子
    WithGevent/dbsample.py  使用gevent+pymysql实现异步读写数据库的例子（gevent下线程池实现）
    WithGevent/sample.py    使用gevent实现异步发送http请求的例子
    sendfile/sendfile.py         多线程文件上传服务端代码
    sendfile/sendfile_client.py  多线程文件上传客户端代码
    proxy_server/proxy.py        正向代理服务器代码
    跨平台/   跨平台版本的fastpy.py

3、支持超大文件上传下载
   默认静态文件(包括html，js、css、图片、文件等)放在static文件夹下
   html和js、css会自动压缩加速
   例如把a.jpg放到static文件夹下
   访问的url为 http://ip:port/static/a.jpg
   支持etag 客户端缓存功能
   (server 使用sendfile进行文件发送，不占内存且快速)

4、支持网页模板编写
   模版引擎代码只有十几行 在WithGevent/common/core.py 文件里的
   class FeimaTpl
   模版用法很简单
   1、用于写python代码的控制块 <% for item in data { %>
   <% %> 中间支持python的 if else for while等程序控制块，
   不同是模版用{ }来代替python 晦涩的缩进来区分控制块范围
   2、取值块 <%=item["id"]> 
   <%= %> 里写的是python的变量指即可，可在1中控制块内使用
   
   下面看个例子
   创建一个模板 a.html


    <html>
        <HEAD><TITLE><%=title%></TITLE></HEAD>
        <BODY>
            <% for item in data{ %>
            <%=item["id"]%>,<%= item["name"] %>
            array data:
            <% for i in item["array"] {%><%= i %><%}%>
            </br>
            <%}%>
        </BODY>
    </html>
   则对应的使用


   from common import core
   tpl = core.FeimaTpl(filepath="./a.html")
   d = []
   d.append({"id":1,"name":"name1","array":[2,4,5,6]})
   d.append({"id":2,"name":"name2","array":[1,3,5,7,9]})
   tpl.assign("title", "my title")
   tpl.assign("data", d)
   print tpl.render()
   

   则生成：
    <html>
        <HEAD><TITLE>my title</TITLE></HEAD>
        <BODY>
            1,name1
            array data:
            2456
            </br>   
            2,name2
            array data:
            13579
            </br>
        </BODY>
    </html>

5、支持http/https透明代理
   python proxy.py 8995
   启动后再浏览器配置代理即可使用，可以弥补nginx 不支持https代理的不足

 

 

c++解决方案: fastrpc——本框架就是为c++语言从应用的角度，封装了尽量易用协程特性，包括了：

1、协程下同步编码异步化（使得mysql/redis/socket在不用修改任何代码情况下同步自动转异步)

2、协程下定时器

3、协程下生产者消费者队列

4、甚至协程下的线程池等，

同时结合了rpc server、http server和游戏中的应用，提供完整的协程示例解决方案。

 项目地址: https://git.oschina.net/feimat/fastrpc

下面是例子

演示例子包括：

1、同步、异步方式

2、client发请求的协程方式

3、server转发请求的协程方式

4、http的请求处理

5、将他人的同步接口变异步

6、协程怎样和多线程切换使用

7、定时器使用和单向推送演示

8、网络异常处理等回调的使用

 

1、同步、异步方式

首先定义 protobuf 文件 （rpc作用和protobuf的作用请自行百度，这里不详述）

package echo;
message EchoRequest {
required string message = 1;
};
message EchoResponse {
required string response = 1;
};
service EchoService
{
rpc Echo(EchoRequest) returns (EchoResponse);
};
option cc_generic_services = true;
使用protoc 生成代码 echo.pb.h echo.pb.cc

然后服务端实现 service：

#include "rpc_server.h"
#include "echo.pb.h" 
class EchoServiceImpl : public echo::EchoService {
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {
        response->set_response(response->response()+" server hello");
        if (done) {
            done->Run();
        }
    }
}
服务端启动：

int main(int argc, char *argv[])
{
    RpcServer server("127.0.0.1", 8996);
    ::google::protobuf::Service *rpc_service = new EchoServiceImpl(&server);
    server.RegiService(rpc_service);
    server.start();
    return 0;
}
客户端如何请求：

同步方式

RpcClient client("192.168.1.13", 8999, 10000,true); // 1host 2port 3超时时间 4是否使用多线程模式
echo::EchoService::Stub stub(&client);
echo::EchoRequest req;
req.set_message("client hello");
echo::EchoResponse res;
CliController controller;
stub.Echo(&controller,&req,&res,NULL); // 最后一个参数为回调，回调为空是同步，不为空是异步
// controller 用于记录运行信息，如超时、错误内容
std::cout << "is timeout:" << controller.IsTimeOut() << "error:" << controller.ErrorText();
异步方式

Test test； 
echo::EchoRequest* request = new echo::EchoRequest();
request->set_message("client hello");
echo::EchoResponse* response = new echo::EchoResponse();
Closure* callback_callback = NULL; // 可以递归无限回调
Closure* callback = pbext::NewCallback(&test,&Test::echo_done,request,response);
stub->Echo(NULL,request,response,callback); // 异步
class Test {
public:
    void echo_done(echo::EchoRequest* request, echo::EchoResponse* response, Closure* done) {
        std::string res = response->response();
        printf("async: %s\n", res.c_str());
        if (done) done->Run(); // 如果有下个回调，就继续执行下个回调
        delete request; // 没用智能指针c++要记得释放
        deleted response;
    }
};
2、client发请求的协程方式

    上面使用中同步调用会阻塞一个线程， 异步模式的话又要注册回调，这样会导致代码不可观。


协程模式客户端：

 void cro_job(echo::EchoService_Stub::Stub* stub, int i) {
    std::stringstream ss;
    ss << i;
    echo::EchoRequest req;
    req.set_message("cli hello  "+ ss.str());
    echo::EchoResponse res;
    stub->Echo(NULL, &req, &res, NULL); // 同步编码，异步的效果
 }
    stub = new echo::EchoService_Stub::Stub(client);
    for (int i =0; i < try_time; ++i) {
        // 这个函数内都是可以同步转异步的
        ::google::protobuf::Closure* routine =
            ::google::protobuf::NewCallback(&cro_job,
                                            stub, i);
        ProcessWithNewCro(routine);
    }
 

3、server转发请求的协程方式

这里client 请求 server1 再请求server2

客户端和上面代码一样使用即可，

Server1调用 rpc例子

class EchoServiceImpl : public echo::EchoService {
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {
        // 再向server2发请求这里是协程同步会放权，不用担心阻塞
        echo_service->Echo(NULL, request, response, NULL);
        printf("recv request from client and send to server2\n");
        response->set_response(response->response()+" add server1 echo");
        if (done) {
            done->Run();
        }
    }
public:
RpcClient* m_rpc_client;
}
int main(int argc, char *argv[])
{
    RpcServer server("127.0.0.1", 8996);
    ::google::protobuf::Service *rpc_service = new EchoServiceImpl(&server);
    // 创建rpcclient和server2(8998端口)通信
    rpc_service->m_rpc_client = new RpcClient("127.0.0.1", 8996, 5000);  
    server.RegiService(rpc_service);
    server.start();
    return 0;
}
在刚刚的server echo实现里，再使用rpcclient 透传请求给server2。这里使用协程同步方式，

不会造成线程阻塞，和异步回调取结果是一样的效果。

注意：本框架默认业务逻辑worker是单线程协程的模式，要使用多线程协程，请参考后面的

线程池使用例子。

4、处理http请求例子

注册httphandler

class MyHttpHandler : public HttpHandler {
public:
    MyHttpHandler(::google::protobuf::RpcChannel* a_rpc_client) {
        m_rpc_client = (RpcClient*)a_rpc_client;
        echo_service = new echo::EchoService_Stub::Stub(m_rpc_client);
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
        CHttpResponseMaker::make_string(res.response(),request->response,content_type,add_head);
        if (done) done->Run();
    }
    virtual void Finish(CASyncSvr* svr) {}
    RpcClient* m_rpc_client;
    echo::EchoService_Stub::Stub* echo_service; 
 }
int main(int argc, char *argv[])
{
    RpcServer server("127.0.0.1", 8996);
    ::google::protobuf::Service *rpc_service = new EchoServiceImpl(&server);
    // 创建rpcclient和server2(8998端口)通信
    rpc_service->m_rpc_client = new RpcClient("127.0.0.1", 8996, 5000);  
    server.RegiService(rpc_service);
    HttpHandler *http_handler = new MyHttpHandler(rpc_service->m_rpc_client);
    server.RegiHttpHandler(http_handler);
    server.start();
    return 0;
}
浏览器访问地址为 http://127.0.0.1:8999/static/index.html?kk=23423424

支持http头解释、url解释和postform解析，一般作为网页http接入接口然后再通过rpc和内部服务器之间通信

5、将他人的同步接口变异步

   在很多时候，我们业务中可能需要用到别人的同步接口，例如mysql查询数据库，这个

时候协程里就会有阻塞，会一定程度上影响性能（python框架里是通过加这两句实现：import gevent.monkey gevent.monkey.patch_socket() ）。

   我们这里提供了对sys sockethook，当执行到系统的socket操作时，会hook住然后放权，把事件触发

交给epoll处理，当有事件过来时再resume执行，这样就不会造成线程阻塞并且可以开很多的同步并发了。

    // 开始sys hook后
    // 系统的socket操作函数遇到阻塞会自动放权
    // 不用修改任何代码，可将同步socket变异步
    co_enable_hook_sys();
    XSocket sock;
    sock.open(SOCK_STREAM);
    if (sock.connect(XSockAddr("127.0.0.1", 8998), 1000) < 0) {
        printf("connect fail\n");
        abort();
    }
    std::stringstream ss;
    ss << "GET http://127.0.0.1:8998/aaa?kk=" << i << " HTTP/1.1\r\n"
        << "Host: 127.0.0.1:8998\r\n"
        << "\r\n";
    std::string send_str = ss.str();
    printf("%d send %d\n", sock.get_handle(), i);
    sock.send_n(send_str.c_str(), send_str.size(), 100000);
    string res;
    sock.recv_one_http(res, 2000);
    printf("%d recv %s\n", sock.get_handle(), res.c_str());
    if (++recv_c == try_time) {
        printf("send all finish\n");
    }
    sock.close();
    co_disable_hook_sys();
6、协程和多线程混合编程（线程池的使用）

   目前我们只提供了socket的hook。如果我们使用中需要用到一些非c socket的同步接口

例如c++调用python, 在python内部会调用到一些同步接口。

这个时候我们做常用的做法是起另外一个线程去做然后yield，做完了再回来告诉我，继续往下做。

我们针对协程提供了线程池接口：

class EchoServiceImpl : public echo::EchoService {
    void TestThreadPool(::echo::EchoResponse* response) {
        response->set_response(response->response() + " add tp echo");
    }
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {
        echo_service->Echo(NULL, request, response, NULL);
        printf("recv request from client and send to server2\n");
        response->set_response(response->response()+" add server1 echo");
        // 切换到多线程执行,放权,等线程池执行完后resume
        tp_mgr->TPRun(this, &EchoServiceImpl::TestThreadPool, response);
        if (done) {done->Run();}
    }
public:
    RpcServer* _rpc_server;
    RpcClient* m_rpc_client;
    echo::EchoService_Stub::Stub* echo_service;
    TPMgr* tp_mgr;
上面例子中我们起了一个线程池，当要支付时就把他扔到线程去做，当前协程yield放权，等到线程池

把支付任务处理完了，再resume，继续进行下面的任务。

另外在线程池内部也会起协程，所以内部调用rpcclient也是会自动同步转异步的哦。

线程池还有异步接口TPAsynRun.

可以看到多线程和协程之间切换编程多么轻松。

7、单向推送例子与定时器使用

这里的功能需求是 client 向server发去一个请求后，server会注册一个定时任务，每隔一秒向client发一个消息，连续发5次。

首先客户端要注册单向推送处理handler

// mes name: echo.EchoResponse
void ext_processer(std::string mes_name, std::string data, void* param) {
    ::google::protobuf::Message* response = PbMgr::CreateMessage(mes_name);
    if (response) {
        response->ParseFromString(data);
        std::cout << "recv push mes, name:" << mes_name
            << " data:" << response->DebugString();
    }
}
RpcClient client("192.168.1.13", 8999, 10000,true); // 1host 2port 3超时时间 4是否使用多线程模式
echo::EchoService::Stub stub(&client); 
client.RegiExtProcesser(ext_processer, NULL);
服务器则要在收到请求后注册定时任务

class EchoServiceImpl : public echo::EchoService {
    void PeriodPush(CASyncSvr* svr, unsigned cli_flow) {
        ::echo::EchoResponse response;
        response.set_response("period push mes");
        RpcServer::PushToClient(svr, cli_flow, &response); // 这是服务器向客户端定时推消息
    }
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::echo::EchoRequest* request,
                      ::echo::EchoResponse* response,
                      ::google::protobuf::Closure* done) {
        echo_service->Echo(NULL, request, response, NULL);
        // 演示定时推送
        RpcController* p_con = (RpcController*)controller;
        unsigned cli_flow = p_con->_cli_flow;
        CASyncSvr* svr = p_con->_svr;
        Closure<void>* period_job =
            NewPermanentClosure(this, &EchoServiceImpl::PeriodPush, svr, cli_flow); // 注意这里一定要用permanentclosure
        timer_mgr.AddJob(1000, period_job, 5);
    
        tp_mgr->TPRun(this, &EchoServiceImpl::TestThreadPool, response);
    
        if (done) {
            done->Run();
        }
    }
public:
    TimerMgr timer_mgr;
然后就客户端发出一次请求后就可以看到陆续收到5个推送消息，每个相隔一秒

recv push mes, name:echo.EchoResponse data:response: "period push mes"

由上面我们看到协程结合线程池、定时器编程是多么轻松。

自由的控制定时任务由哪些线程或协程按分什么顺序执行，完全面向对象的编码风格，同步的编码方式，获得异步的效果。

8、断开事件处理

功能需求是假设是游戏客户端，在客户端异常断开时，服务器应该要触发事件，以便服务器在这里改变游戏角色的在线状态，另外客户端也应该在与服务器断开是有提示或做相应重连处理

服务器注册断开事件处理函数

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
    int p = 123; // 附带参数
    server.RegiClientCloseHandler(close_handler, &p);
}
 客户端和上面服务器一样定义处理函数和注册即可

以上所有handler 包括http、close 和单向推送的handler都是在协程里面处理的，也就是说handler里面都是可以使用 rpc client协程同步模式去向别的服务器发请求，而不会阻塞线程的

两个框架项目地址: https://git.oschina.net/feimat

欢迎加入qq群339711102，一起探讨优化哦
