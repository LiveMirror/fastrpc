#ifndef _SEARCH_CLIENT_NET_SVR_H_
#define _SEARCH_CLIENT_NET_SVR_H_

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <string>
#include <map>
#include <vector>
#include <list>

#include "Define.h"
#include "ependingpool.h"

using namespace std;


/**
* @author feimat@feimat.com
*
*
* <pre>
* 前端接入异步单线程模块
*
* </pre>
**/

class CClientSocketReq
{
public:
    bool keep_live;
    CRWCache sendcache;
    CRWCache recvcache;

    /*
    重载函数，判断网络数据接收是否完整

    is_complete 和 check_recv_one 返回值
    ret<0 小于零,返回失败
    ret>0 大于零,返回得到的字节数
    ret=0 等于零,还要继续接收
    注意如果包大于 CLIENT_COMPLETE_MAX_BUFFER 和 BACK_COMPLETE_MAX_BUFFER，需要放弃包。
    */
    virtual int is_complete(char *data,unsigned data_len);

    // 检查是否收到一个完整的包，然后把他放到buff里面，len是长度
    int check_recv_one(char *buff,unsigned &len)
    {
        len = 0;
        int ret = is_complete(recvcache.data(),recvcache.data_len());
        if ( ret>0 ) // 收到一个完整的包
        {
            //memcpy(buff,recvcache.data(),ret);
            //recvcache.skip(ret);

            //if ( recvcache.data()==NULL ) LOG(LOG_ALL,"NULLxxxxxxx ret %d,%u\n",ret,recvcache.data_len());
            //LOG(LOG_ALL,"xxxxxxx ret %d,%u\n",ret,recvcache.data_len());

            len = ret;
            return ret;
        }
        return ret;
    }

    CClientSocketReq();
    virtual ~CClientSocketReq();

    unsigned flow;
    int offset;
    // 统计
    timeval tBegin;
    timeval tEnd;
};

//int todo_client(struct ependingpool::ependingpool_task_t *v);
//int read_client(int sock, void **arg);
//int write_client(int sock, void **arg);
//int accept_client(int lis, void **arg);
//int ini_client(int sock, void **arg);
//int clear_client(int sock, void **arg);
//int  server_hup(int sock,void **arg);
//int read_time_out(int sock,void **arg);
//int write_time_out(int sock,void **arg);
//int listen_time_out(int sock,void **arg);
//int sock_close(int sock,void **arg);
//int sock_error(int sock,void **arg);


/*
    协议数据结构
    【nshead】【动态内容】没有ReqHeader
*/
typedef struct _ShareParam {
    _ShareParam (int max_len) {
        client_recv_len = max_len;
        //client_recv_buff = new char[max_len];
        client_recv_buff = (char*)malloc(max_len);
        cliF2O = new CFlow2Offset();
        flow = 0;
    }
    ~_ShareParam () {
        if (client_recv_buff) {
            delete []client_recv_buff;
            delete cliF2O;
        }
    }
    char* client_recv_buff;
    unsigned client_recv_len;
    CFlow2Offset* cliF2O;
    unsigned flow;
    CASyncSvr* p_svr;
} ShareParam;

class CClientNetSvr
{
public:
    static void *run(void *instance);

    // buf_len 接收缓冲区的长度
    CClientNetSvr(CASyncSvr* asvr,const int &_max,int _sock_num=10000,int _queue_len=10000);
    ~CClientNetSvr();

    void Start(int listen_sd);
    void MainLoop();


    unsigned max_conn;
    CMetaQueue<CDataBuf> mPending;

    ependingpool pool;
    int pool_sock_num;
    int pool_queue_len;
    int pool_listen_fd;

    ShareParam* sp;
    CASyncSvr* p_svr;

};





/*

=2 操作成功， 触发SOCK_CLEAR事件，但不关闭句柄，可以把句柄移出ependingpool


_block_size是升级已经申请的空间

connect 是 status_send_connecting


dcc_req_send:  // 一般我们只用到这个，然后直接就是 status_send_connecting

(ret != -EWOULDBLOCK) && (ret != -EINPROGRESS) 非阻塞connect，如果返回这个，下次就可读可写

epoll有两种模式,Edge Triggered(简称ET) 和 Level Triggered(简称LT).在采用这两种模式时要注意的是,如果采用ET模式,那么仅当状态发生变化时才会通知,而采用LT模式类似于原来的select/poll操作,只要还有没有处理的事件就会一直通知.

边界触发的效率高，但程序实现上要小心，漏掉没处理的不会得到提醒了。
而电平触发的程序实现上方便，效率要低些，似乎libevent和lighttpd都用的它。


EPOLLIN  可读，包括socket正常关闭
EPOLLOUT 可写
EPOLLERR 发生错误
EPOLLHUP socket短了
EPOLLONEAHOT 只监听一次事件，如想监听，必须再次epoll_ctl



keep_alive close(fd)，并从epoll_ctl(del)
reset_item(int handle, bool keep_alive)
pool_epoll_offset_mod(handle, EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLONESHOT);

clear_item
不close（fd），只是pool_epoll_offset_del(handle);


加入epoll，都是默认参数
accept_sock
insert_item_sock



【CCD】

1. 【epoll event】 send 发送完毕，改为 IN,未发送完，不做任何处理
2. 【epoll event】 recv 不做任何处理
3. 【item data】 send完成，不做任何处理，未完成，改为in、out



1.  【item data】已经连接，send发送完，不做任何处理，未发送完，修改为in/out
不存在连接，发送完，改为IN,未发送完，改为in/out


2.  【epoll event】 发送完，改为IN，未发送完，不做任何处理
3.  【epoll event】 recv 不做任何处理



insert_item 会修改为 last_active，并设置为 sock_status = READY
reset_item 如果保持长连接 true ，会last_active，并设置为 sock_status = READY
write_reset_item 会last_active，并设置为 sock_status = WRITE_BUSY

do_read_event
if (READ_BUSY != m_ay_sock[offset].sock_status) {

m_ay_sock[offset].last_active = time(NULL);

m_ay_sock[offset].sock_status = READ_BUSY;

}

do_write_event
if (WRITE_BUSY != m_ay_sock[offset].sock_status) {

m_ay_sock[offset].last_active = time(NULL);

m_ay_sock[offset].sock_status = WRITE_BUSY;

}

放到就绪队列， 会修改为 last_active
m_ay_sock[offset].sock_status = BUSY;
*/

#endif
