#include "rpc_server.h"

#include "xcore_atomic.h"

static void HandleServiceDone(HandleServiceEntry *entry) {
    std::string cli_id = entry->cli_id_;
    std::string content;
    entry->response_->SerializeToString(&content);
    std::stringstream ss;
    ss << "POST / HTTP/1.1\r\n"
    //ss << "HTTP/1.1 200 OK\r\n"
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

    int Initial(CASyncSvr* svr){
        RpcServer &rpc_server = (  *((RpcServer*)svr->rpc_param) );
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

void* CUploadDownWebSvr::Run(CDataBuf *item, CASyncSvr* svr)
{
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
        (opcode.empty() || cli_id.empty())) {
        /// http
        HttpRequest* request = new HttpRequest(ps,
                                               item->data,
                                               item->len,
                                               cli_flow,
                                               svr);
        ::google::protobuf::Closure *done = ::google::protobuf::NewCallback(&HandleHttpDone,
                                                                            request);
        item->data = NULL;
        http_handler->OnRec(request, done);
        return NULL;
    }

    /// rpc
    map<std::string, RpcMethod*>::iterator it = rpc_method_map_->find(opcode);
    if (it != rpc_method_map_->end()) {
        RpcMethod *rpc_method = it->second;
        const ::google::protobuf::MethodDescriptor *method = rpc_method->method_;
        ::google::protobuf::Message *request = rpc_method->request_->New();
        ::google::protobuf::Message *response = rpc_method->response_->New();
        ::google::protobuf::RpcController* rpc_controller = new RpcController(svr, cli_flow);
        int headlen = item->len - content_len;
        char* content = item->data + headlen;
        if (!request->ParseFromArray(content, content_len)) {
            delete request;
            delete response;
            delete ps;
            LOG(LOG_ALL,"%s decode error\n", opcode.c_str());
            return NULL;
        }
        HandleServiceEntry *entry = new HandleServiceEntry(method,
                                                           request,
                                                           response,
                                                           svr,
                                                           cli_flow,
                                                           cli_id,
                                                           rpc_controller);
        ::google::protobuf::Closure *done = ::google::protobuf::NewCallback(&HandleServiceDone,
                                                                            entry);
        rpc_method->service_->CallMethod(method,
                                         rpc_controller,
                                         request, response, done);
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

















//////////////http服务默认配置，走http则无需改动////////////////////
/*
is_complete 和 check_recv_one 返回值
ret<0 小于零,返回失败
ret>0 大于零,返回得到的字节数
ret=0 等于零,还要继续接收
注意如果包大于 CLIENT_COMPLETE_MAX_BUFFER 和 BACK_COMPLETE_MAX_BUFFER，需要放弃包。
*/
int CClientSocketReq::is_complete(char *data,unsigned data_len)
{
    return http_complete_func(data,data_len);
}

int CBackSocketReq::is_complete(char *data,unsigned data_len)
{
    return http_complete_func(data,data_len);
}

//业务逻辑默认同步模式
xCallbackObj *CMainAsyncSvr::CreateAsyncObj(CDataBuf *item)
{
    return NULL;
}
CTask *CASyncSvr::CCreatePoolTask()
{
    CUploadDownWebSvr *web = new CUploadDownWebSvr();
    return web;
}

bool RpcServer::start() {
    //IsClientAsyn = false; // 默认true，客户端如果是同步模式，设为false，可减少加锁
    //Multi_Process_or_Thread = "Process"; // 默认Thread，io模块使用多线程还是多进程
    LOG_OPEN(LOG_DEBUG,LOG_TYPE_DAILY,"./","arpc_server");
    int cpu_num = ipcs_common::GetCpuNum();
    app_main(host_, port_, RPCSOCKNUM, cpu_num+1, 2, this);
    return true;
}
