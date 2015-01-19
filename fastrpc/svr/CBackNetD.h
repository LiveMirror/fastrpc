#ifndef _SEARCH_BACK_NET_SVR_H_
#define _SEARCH_BACK_NET_SVR_H_

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
* @author feimat@baidu.com
*
* 
* <pre>
* 连接后台的网络异步接受模块
*
* </pre>
**/






class CBackSocketReq
{
public:
    CRWCache sendcache;
    CRWCache recvcache;

    /*
    重载函数，判断网络数据接收是否完整

    is_complete 和 check_recv_one 返回值
    ret<0 小于零,返回失败
    ret>0 大于零,返回得到的字节数
    ret=0 等于零,还要继续接收
    // 注意如果包大于 CLIENT_COMPLETE_MAX_BUFFER 和 BACK_COMPLETE_MAX_BUFFER，需要放弃包。
    */
    virtual int is_complete(char *data,unsigned data_len);

    // 检查是否收到一个完整的包，然后把他放到buff里面，len是长度
    int check_recv_one(char *buff,unsigned &len)
    {
        int ret = is_complete(recvcache.data(),recvcache.data_len());
        if ( ret>0 ) // 收到一个完整的包
        {
            memcpy(buff,recvcache.data(),ret);
            recvcache.skip(ret);
            len = ret;
            return ret;
        }
        return ret;
    }


    CBackSocketReq(){}
    virtual ~CBackSocketReq(){}

};


int read_back_svr(int sock, void **arg);
int write_back_svr(int sock, void **arg);
int init_back_svr(int sock, void **arg);
int clear_back_svr(int sock, void **arg);



/*
    协议数据结构
    【nshead】【动态内容】没有ReqHeader
*/
class CBackNetSvr
{
public:
    static void *run(void *instance);

    // buf_len 接收缓冲区的长度
    CBackNetSvr(const int &_max,int _sock_num=10000,int _queue_len=10000);
    ~CBackNetSvr(){}

    void Start();
    void MainLoop();
  

    unsigned max_conn;
    CMetaQueue<CDataBuf> mPending;

	CIp2Handle ip2back_flow;

    ependingpool pool;
    int pool_sock_num;
    int pool_queue_len;

};


#endif
