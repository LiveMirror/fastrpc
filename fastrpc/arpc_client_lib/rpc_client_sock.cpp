#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include <stdio.h>

#include "rpc_client_sock.h"
#include "CSvr.h"
#include "CConnectPoll.h"
#include "Statistic.h"
#include "http_server_codec.h"
#include "xcore_atomic.h"
#include "xcore_socket.h"
#include "xcore_define.h"


using namespace std;

int GloSockNum = 8;


/*
* <0 写失败，会触发SOCK_CLEAR事件
* =0 数据全部写完毕，并将sock重新放入epoll中进行监听读请求(相当于长连接)
* =1 数据写成功，但没有全部写完，放回epoll等待下次再监听到可写状态

*/

//XAtomic32 jjj = 0;

int rpc_write_client(ependingpool* ep, int sock, void **arg)
{
	unsigned write_succ_size = 0;
	int ret = -1;

	// 可写，直接从缓存在发送
	CClientSocketReq *psock_req = (CClientSocketReq*)(*arg);


    while (true ) {
        psock_req->sendcache.extern_lock();

        if ( psock_req->sendcache.data_len() == 0 ) {
            int w_ret = -1;
            if ( psock_req->keep_live ) {
                ep->reset_item(psock_req->offset, true);
                w_ret = 1;
            }
            psock_req->sendcache.extern_unlock();
            return w_ret;
        }
        ret = noblock_write_buff(sock,psock_req->sendcache,&write_succ_size);
        if ( ret!=0 ) {
            if(ret == -1) {
                LOG(LOG_ALL,"write buf failed.ip is %s and data len is %s\n",sock2peer(sock).c_str(),psock_req->sendcache.data_len());
            }
            if(ret == -2) {
                LOG(LOG_ALL,"client closed detected by write.ip is %s and data len is %s\n",sock2peer(sock).c_str(),psock_req->sendcache.data_len());
            }
            psock_req->sendcache.extern_unlock();
            return -1;
        }
        if ( write_succ_size>0 ) psock_req->sendcache.skip(write_succ_size);

        psock_req->sendcache.extern_unlock();

        if ( ret==0 && errno==EAGAIN && write_succ_size==0 ) {
            break;
        }

    }


    psock_req->sendcache.extern_lock();
    if ( psock_req->sendcache.data_len() == 0 ) {
        int w_ret = -1;
        if ( psock_req->keep_live ) {
            ep->reset_item(psock_req->offset, true);
            w_ret = 1;
        }
        psock_req->sendcache.extern_unlock();
        return w_ret;
    }
    psock_req->sendcache.extern_unlock();
    return 1;
}
/*
* <0 读失败，会触发SOCK_CLEAR事件
* =0 数据全部读取完毕，并将sock放入就绪队列中
* =1 数据读成功，但没有全部读取完毕，需要再次进行监听，sock会被放回epoll中
* =2 放到todo event

*/
int rpc_read_client(ependingpool* ep, int sock, void **arg)
{
	int ret = 0;
	unsigned read_size = 0;
    CClientSocketReq *psock_req = (CClientSocketReq*)(*arg);
    RpcShareParam* sp = (RpcShareParam*)ep->ext_data;
    char* client_recv_buff = sp->client_recv_buff;
    int client_recv_len = sp->client_recv_len;

    unsigned hasread=0;
    //unsigned i = 0;
    //while ( i<10000 ) {
    while ( true ) {
        ret = noblock_read_buff(sock,client_recv_buff,&read_size,client_recv_len);
        if ( ret!=0 ){
            if( ret == -1)
            {
                 LOG(LOG_ALL,"read buf failed.ip is %s and data is %s\n",sock2peer(sock).c_str(),client_recv_buff);
            }
            if( ret == -2)
            {
                 //LOG(LOG_ALL,"client closed detected by read. ip is %s and data is %s\n",sock2peer(sock).c_str(),client_recv_buff);
            }

            return -1;    }
        if ( read_size>0 ) psock_req->recvcache.append(client_recv_buff,(unsigned)read_size);

        if ( read_size>0 ) hasread += read_size;
        if ( ret==0 && errno==EAGAIN && read_size==0 ) {
            //LOG(LOG_ALL,"EAGAIN|sock %d|i|%u|%d|%s|has read|%u\n",sock,i,errno,strerror(errno),hasread);
           // i=2000000;
            //continue;
            break;
        }
        //LOG(LOG_ALL,"INFO|sock %d|i|%u|%d|%s|has read|%u\n",sock,i,errno,strerror(errno),hasread);
      //  i++;
    }


    // 直接返回2，todo event操作
    return 2;
}

int rpc_todo_client(ependingpool* ep, struct ependingpool::ependingpool_task_t *v)
{
    int sock = v->sock;
    void *arg = v->arg;


    CClientSocketReq *psock_req = (CClientSocketReq *)(arg);
    RpcShareParam* sp = (RpcShareParam*)ep->ext_data;
    char* client_recv_buff = sp->client_recv_buff;
    unsigned one_len = 0;
    int ret = 0;

    RpcClient* p_svr = sp->p_svr;

    do {
        ret = psock_req->check_recv_one(client_recv_buff,one_len);
        if ( ret>0 ) {
            unsigned long ip = 0;
            sock2peer(sock,ip);

            std::string one_http;
            one_http.assign(psock_req->recvcache.data(), one_len);
            p_svr->PutReadQueue(one_http);

            psock_req->recvcache.skip(one_len);

            if ( psock_req->recvcache.data_len()==0 ) ret = 0;
        }
    } while( ret>0 );

    // 返回0，监听 OUT，写操作
    // 返回1，监听 IN
    // 返回2，关闭连接clear_item
    // 返回3，什么也不做
    if ( ret >= 0 ) {
		// 这个包还没接收完，需要继续读
        return 3;
	}
	// 读失败，触发SOCK_CLEAR事件
	return -1;
}

int rpc_ini_client(ependingpool* ep, int sock, void **arg)
{
	CClientSocketReq *sock_req = new CClientSocketReq();
	if (sock_req == NULL)
	{
		return -1;
	}
	//将指针赋给sock捆绑的指针
	*arg = sock_req;
	return 0;
}

int rpc_clear_client(ependingpool* ep, int sock, void **arg)
{
	CClientSocketReq *req = (CClientSocketReq*)(*arg);
    RpcShareParam* sp = (RpcShareParam*)ep->ext_data;
	if ( req!=NULL ){
        sp->cliF2O->Del(sock);
		delete req;
		req = NULL;

        string strip = sock2peer(sock);
        //LOG(LOG_ALL,"CLI|%s close|sock:%d\n",strip.c_str(),sock);
	}
	return 0;
}


int rpc_accept_client(ependingpool* ep, int lis, void **arg)
{
    int work_sock;
    work_sock = do_accept(lis);

    if ( work_sock == -1 ) {
        //LOG(LOG_ALL,"do_accept(%d) call failed.error[%d] info is %s.", work_sock,errno, strerror(errno));
        return -1;
    }


    //这里需要设置非堵塞模式， 否则读写的时候会被hand住
    if (do_setsocktonoblock(work_sock)) {
        close(work_sock);

        LOG(LOG_ALL,"do_setsocktonoblock failed.sock[%d] error[%d] info is %s.",work_sock,errno,strerror(errno));
        return -1;
    }
    return work_sock;

}

int rpc_server_hup(ependingpool* ep, int sock,void **arg){
     LOG(LOG_ALL,"sock error: %d server socket error detected by epollhup.\n", sock);
     return 0;
}

int rpc_sock_error(ependingpool* ep, int sock,void **arg){
    LOG(LOG_ALL,"sock error: %d server socket error detected by epollerror.\n", sock);
    return 0;
}

int rpc_sock_close(ependingpool* ep, int sock,void **arg){
    //LOG(LOG_ALL,"sock will be closed.\n", sock);
    return 0;
}

RpcClientSock::RpcClientSock(RpcClient* asvr,const int &_max,int _sock_num/* =10000 */,int _queue_len/* =10000 */)

{
    p_svr = asvr;
    max_conn = _max;

    pool_sock_num = _sock_num;
    pool_queue_len = _queue_len;

    pool.set_epoll_timeo(-1);
    //设置连接超时时间(秒), 默认为1s
    pool.set_conn_timeo(60*30);
    //设置读超时时间(秒), 默认为1s
    pool.set_read_timeo(60*30);
    //设置写超时时间(秒), 默认为1s
    pool.set_write_timeo(60*30);

    //设置可存储socket的数量
    pool.set_sock_num(max_conn);
    //设置已就绪队列的长度
    pool.set_queue_len(pool_sock_num);
    sp = new RpcShareParam(CLIENT_COMPLETE_MAX_BUFFER);
    sp->p_svr = p_svr;
    pool.ext_data = sp;
    pthread_mutex_init(&_offmux,NULL);
}

RpcClientSock::~RpcClientSock() {
    pthread_mutex_destroy(&_offmux);
    delete sp;
}

int rpc_read_time_out(ependingpool* ep, int sock, void **arg) {
    LOG(LOG_ALL,"sock error: %d read time out.\n", sock);
    return 0;
}

int rpc_write_time_out(ependingpool* ep, int sock, void **arg) {
    LOG(LOG_ALL,"sock error: %d write time out.\n", sock);
    return 0;
}

int rpc_listen_time_out(ependingpool* ep, int sock, void **arg) {
    LOG(LOG_ALL,"sock error: %d listen time out.\n", sock);
    return 0;
}


void RpcClientSock::Start()
{

    int ret;

    //针对不同的事件使用相应的回调函数
    ret = pool.set_event_callback(ependingpool::SOCK_ACCEPT, rpc_accept_client);   //printf("set ret%d\n",ret);


    //初始化与sock捆绑的数据
    pool.set_event_callback(ependingpool::SOCK_INIT, rpc_ini_client);
    //释放与sock捆绑的数据
    pool.set_event_callback(ependingpool::SOCK_CLEAR, rpc_clear_client);
    //使用非堵塞读写方式，模拟异步读写
    pool.set_event_callback(ependingpool::SOCK_READ, rpc_read_client);
    pool.set_event_callback(ependingpool::SOCK_WRITE, rpc_write_client);
    pool.set_todo_event_ex(rpc_todo_client, NULL);
   // set timeout callback,by liuyulian; 2014-04-08
    pool.set_event_callback(ependingpool::SOCK_READTIMEOUT, rpc_read_time_out);
    pool.set_event_callback(ependingpool::SOCK_WRITETIMEOUT, rpc_write_time_out);
    pool.set_event_callback(ependingpool::SOCK_LISTENTIMEOUT, rpc_listen_time_out);
    pool.set_event_callback(ependingpool::SOCK_HUP,rpc_server_hup);
    pool.set_event_callback(ependingpool::SOCK_CLOSE,rpc_sock_close);
    pool.set_event_callback(ependingpool::SOCK_ERROR,rpc_sock_error);

    random = 0;

    _mainWorker.start(RpcClientSock::run,(void*)this);
    return;
}

void RpcClientSock::InsertOneSock() {
    pthread_mutex_lock(&_offmux);
    int sock_size = sp->cliF2O->GetSize();
    if (sock_size < GloSockNum) {
        XSocket xsock;
        xsock.open(SOCK_STREAM);
        xsock.set_nonblock(true);
        bool bret = xsock.connect(XSockAddr(sp->p_svr->host_, sp->p_svr->port_));
        if (!bret || !xsock.can_send(1)) {
            printf("rpc server  connect fail\n");
            exit(1);
        }
        int aoff = 0;
        int sock = (int)xsock.m_sock;
        xsock.detach();
        int ret = pool.insert_item_sock(sock,aoff,NULL,0);
        if (ret < 0) {
            printf("insert sock to epollpool fail\n");
            exit(1);
        }
        sp->cliF2O->Add(sock, aoff);
    }
    pthread_mutex_unlock(&_offmux);
}

void RpcClientSock::MainLoop()
{
    for (int i = 0; i < GloSockNum; ++i) {
        InsertOneSock();
    }

    while (pool.is_run()) {
        pool.check_item();
    }// while
    return;
}

void RpcClientSock::PushData(std::string data) {
    int sock_size = sp->cliF2O->GetSize();
    if (sock_size < GloSockNum) {
        InsertOneSock();
    }

    int offset = -1;
    unsigned cd_flow = -1;
    int try_c = 0;
    while (++try_c < 100) {
        int insert_sock = sp->cliF2O->GetNextSock(random);
        cd_flow = insert_sock;
        offset = sp->cliF2O->GetOffset(cd_flow);
        if ( offset!=-1 ) {
            break;
        }
        printf("offset %d has been close\n", offset);
    }
    if (offset == -1) {
        printf("sock has been close\n");
        exit(1);
    }

    if (!data.empty()) {
        void *arg;
        int ret = pool.fetch_handle_arg(offset,&arg);
        if ( ret != 0 )
        {
            printf("fetch handle null\n");
            exit(1);
            LOG(LOG_ALL,"Error:pool fetch_handle_arg ret %d,cdflow %u,offset %d\n",ret,cd_flow,offset);
        }
        else
        {
            CClientSocketReq *psock_req = (CClientSocketReq*)(arg);
            if ( psock_req!=NULL )
            {
                psock_req->offset = offset;
                psock_req->sendcache.extern_lock();
                psock_req->sendcache.append_nolock(data.c_str(),data.length());
                if ( psock_req->sendcache.data_len() == 0 ) {
                    printf("append null\n");
                    exit(1);
                }

                ret = pool.write_reset_item(offset);
                if ( ret!= 0 )
                {
                    printf("write reset item fail\n");
                    exit(1);
                    LOG(LOG_ALL,"Error:pool write_reset_item ret %d,cd_flow %u,offset %d\n",ret,cd_flow,offset);
                }
                psock_req->keep_live = true;
                psock_req->sendcache.extern_unlock();
            }
            else
            {
                printf("psock req null\n");
                exit(1);
                LOG(LOG_ALL,"Error:cd_flow %u,offset %d fetch_handle_arg NULL\n",cd_flow,offset);
            }
        }
    }
    else {
        printf("push data empty\n");
        exit(1);
    }

}

void *RpcClientSock::run(void *instance)
{
    RpcClientSock *client_svr = (RpcClientSock*)instance;
    pthread_t tid = pthread_self();
    LOG(LOG_ALL,"Info:IndexD RpcClientSock detach thread create,pid=%d,tid=%u\n",getpid(),tid);

    client_svr->MainLoop();

    return NULL;
}
