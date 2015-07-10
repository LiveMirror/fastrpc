#include "rpc_server.h"

#include "xcore_atomic.h"
#include "pbext.h"

#include <unistd.h>


void* ProcLoop(void* args) {
    SetCoroutineUsedByCurThread();
    //RpcServer* rpc_server = (RpcServer*)args;
    while(true) {
        PbClosure* pb_closure = RpcMgr::GetOutSideQueue(1000000);
        if (NULL == pb_closure) {
            continue;
        }
        pb_closure->Run();
    }
    return NULL;
}

static void HandleServiceDone(HandleServiceEntry *entry) {
    std::string cli_id = entry->cli_id_;
    std::string content;
    entry->response_->SerializeToString(&content);
    std::stringstream ss;
    ss << "POST / HTTP/1.1\r\n"
        << "cli_id: " << cli_id << "\r\n"
        << "Content-Length: " << content.length() << "\r\n"
        << "\r\n"
        << content;
    std::string resp_str = ss.str();

    entry->svr_->SendBack(entry->clid_flow_, (char*)(resp_str.c_str()), resp_str.length());

    delete entry->request_;
    delete entry->response_;
    delete entry->rpc_controller_;
    delete entry;
}

static void HandleHttpDone(HttpRequest *request) {
    request->svr->SendBack(request->cli_flow,
                           (char*)(request->response.c_str()),
                           request->response.length());

    char* data = request->data;
    delete request->ps;
    delete []data;
    delete request;
}

class CUploadDownWebSvr : public CTask
{
public:
    CUploadDownWebSvr(){}
    ~CUploadDownWebSvr(){}
    map<std::string, RpcMethod*>* rpc_method_map_;
    HttpHandler* http_handler;
    RpcServer* rpc_svr;

    int Initial(CASyncSvr* svr){
        RpcServer &rpc_server = (  *((RpcServer*)svr->rpc_param) );
        rpc_svr = (RpcServer*)(svr->rpc_param);
        svr->RegiCloseHandler(rpc_server._close_handler,
                              rpc_server._close_handler_param);
        rpc_method_map_ = rpc_server.pop_service();
        if (rpc_method_map_ == NULL) {
            printf("rpc service is null");
            assert(false);
        }
        http_handler = rpc_server.pop_http_handler();
        if (rpc_server.has_http_handler &&
            http_handler == NULL) {
            printf("http handler regi fail");
            assert(false);
        }
        if (http_handler) {
            http_handler->Init(svr);
        }
        return 0;
    }
    void* Run(CDataBuf *item, CASyncSvr* svr);
    int Finish(CASyncSvr* svr){
        if (http_handler) {
            http_handler->Finish(svr);
        }
        return 0;
    }


private:
    int iClientNum;
};

void RunHttp(HttpHandler* http_handler,
             HttpRequest* request,
             PbClosure* done) {
    http_handler->OnRec(request, done);
}

void RunRpcMethod(RpcMethod *rpc_method,
                  const ::google::protobuf::MethodDescriptor *method,
                  ::google::protobuf::RpcController* rpc_controller,
                  ::google::protobuf::Message *request,
                  ::google::protobuf::Message *response,
                  PbClosure* done) {
    rpc_method->service_->CallMethod(method,rpc_controller,
                                     request, response, done);
}

void RunCloseHandler(RpcServer* rpc_svr,
                     CASyncSvr* svr,
                     unsigned cli_flow,
                     void* param) {
    assert(NULL != rpc_svr->_close_handler); // 空不可能会调用
    rpc_svr->_close_handler(svr, cli_flow, param);
}

void* CUploadDownWebSvr::Run(CDataBuf *item, CASyncSvr* svr)
{
    if (NULL == item->data || 0 == item->len) { // 处理close 事件
        PbClosure* proc_closure = pbext::NewCallback(&RunCloseHandler, rpc_svr, svr,
                                                     item->flow, rpc_svr->_close_handler_param);
        ::google::protobuf::Closure* cro_closure =
            ::google::protobuf::NewCallback(&RpcMgr::RunWithCoroutine, proc_closure);
        RpcMgr::PutOutSideQueue(cro_closure);
        return NULL;
    }
    CHttpParser* ps = new CHttpParser();

    int ret = ps->parse_head(item->data, item->len);
    if ( ret <= 0 ) {
        LOG(LOG_ALL,"Http decode error return %d\n",ret);
        return NULL;
    }
    unsigned cli_flow = item->flow;

    int content_len = ps->getContentLen();

    string opcode = ps->get_head_field("op");
    string cli_id = ps->get_head_field("cli_id"); // 客户端包唯一标识
    if (http_handler &&
        (opcode.empty() || cli_id.empty())) { // 处理http请求
        HttpRequest* request = new HttpRequest(ps, item->data, item->len,
                                               cli_flow, svr);
        item->data = NULL;
        ::google::protobuf::Closure *done = ::google::protobuf::NewCallback(&HandleHttpDone,
                                                                            request);
        PbClosure* proc_closure = pbext::NewCallback(&RunHttp,
                                                     http_handler,
                                                     request,
                                                     done);
        ::google::protobuf::Closure* cro_closure =
            ::google::protobuf::NewCallback(&RpcMgr::RunWithCoroutine, proc_closure);
        RpcMgr::PutOutSideQueue(cro_closure);
        return NULL;
    }

    map<std::string, RpcMethod*>::iterator it = rpc_method_map_->find(opcode);
    if (it != rpc_method_map_->end()) { // 处理rpc请求
        RpcMethod *rpc_method = it->second;
        const ::google::protobuf::MethodDescriptor *method = rpc_method->method_;
        ::google::protobuf::Message *request = rpc_method->request_->New();
        ::google::protobuf::Message *response = rpc_method->response_->New();
        int headlen = item->len - content_len;
        char* content = item->data + headlen;
        if (!request->ParseFromArray(content, content_len)) {
            delete request;
            delete response;
            delete ps;
            LOG(LOG_ALL,"%s decode error\n", opcode.c_str());
            return NULL;
        }
        ::google::protobuf::RpcController* rpc_controller = new RpcController(svr, cli_flow);
        HandleServiceEntry *entry = new HandleServiceEntry(method, request, response, svr,
                                                           cli_flow, cli_id, rpc_controller);
        ::google::protobuf::Closure *done = ::google::protobuf::NewCallback(&HandleServiceDone,
                                                                            entry);
        PbClosure* proc_closure =
            pbext::NewCallback(&RunRpcMethod,rpc_method,method,rpc_controller,request,response,done);
        ::google::protobuf::Closure* cro_closure =
            ::google::protobuf::NewCallback(&RpcMgr::RunWithCoroutine, proc_closure);
        RpcMgr::PutOutSideQueue(cro_closure);
        delete ps;
        return NULL;
    }

    string str_data;
    string sContent = "404 page not found";
    string resp_str;
    string content_type;
    string add_head;

    content_type = "text/html";
    add_head = "Connection: keep-alive\r\n";
    CHttpResponseMaker::make_string(sContent, resp_str, content_type, add_head);

    //svr->SendBack(cli_flow,(char*)(resp_str.c_str()),resp_str.length(),SEND_AND_CLOSE);
    svr->SendBack(cli_flow,(char*)(resp_str.c_str()),resp_str.length());

    delete ps;
    return NULL;
}

int core_count() {
    int count = sysconf(_SC_NPROCESSORS_CONF);
    if (count < 1) count = 1;
    return count;
}

bool RpcServer::start() {
    LOG_OPEN(LOG_DEBUG,LOG_TYPE_DAILY,"./","arpc_server");
    if (listen_fd_ > 0) {
        // 多进程模式
        StartOneProc(listen_fd_, RPCSOCKNUM, 1, 1, this);
        return true;
    } else {
        int cpu_num = core_count();
        // io用多线程 业务逻辑用单线程协程
        StartMulThread(host_, port_, RPCSOCKNUM, cpu_num, 1, this);
        return true;
    }
}















//////////////http服务默认配置，走http则无需改动////////////////////
/*
is_complete 和 check_recv_one 返回值
ret<0 小于零,返回失败
ret>0 大于零,返回得到的字节数
ret=0 等于零,还要继续接收
*/
int CClientSocketReq::is_complete(char *data,unsigned data_len)
{
    return http_complete_func(data,data_len);
}

int CBackSocketReq::is_complete(char *data,unsigned data_len)
{
    return http_complete_func(data,data_len);
}

xCallbackObj *CMainAsyncSvr::CreateAsyncObj(CDataBuf *item)
{
    return NULL;
}
CTask *CASyncSvr::CCreatePoolTask()
{
    CUploadDownWebSvr *web = new CUploadDownWebSvr();
    return web;
}

