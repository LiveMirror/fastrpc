#ifndef _RPC_SOCK_EPOLL_
#define _RPC_SOCK_EPOLL_

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
#include "rpc_client.h"

using namespace std;


class RpcClient;

typedef struct _RpcShareParam {
    _RpcShareParam (int max_len) {
        client_recv_len = max_len;
        client_recv_buff = new char[max_len];
        cliF2O = new CFlow2Offset();
        flow = 0;
    }
    ~_RpcShareParam () {
        if (client_recv_buff) {
            delete []client_recv_buff;
            delete cliF2O;
        }
    }
    char* client_recv_buff;
    unsigned client_recv_len;
    CFlow2Offset* cliF2O;
    int flow;
    RpcClient* p_svr;
} RpcShareParam;

class RpcClientSock
{
public:
    static void *run(void *instance);

    // buf_len 接收缓冲区的长度
    RpcClientSock(RpcClient* asvr,const int &_max,int _sock_num=10000,int _queue_len=10000);
    ~RpcClientSock();

    void Start();
    void MainLoop();

    void PushData(std::string data);
    void InsertOneSock();


    unsigned max_conn;
    CMetaQueue<CDataBuf> mPending;

    ependingpool pool;
    int pool_sock_num;
    int pool_queue_len;
    int pool_listen_fd;

    RpcShareParam* sp;
    RpcClient* p_svr;

    std::vector<int> offset_list;
    pthread_mutex_t _offmux;
    int random;
    CMetaThread _mainWorker;
};





#endif
