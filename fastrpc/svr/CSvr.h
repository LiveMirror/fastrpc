#ifndef _SEARCH_THREAD_SVR_H_
#define _SEARCH_THREAD_SVR_H_

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <semaphore.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <list>


#include "Define.h"


/**
*
*
* <pre>
* 异步编程svr，包括单线程异步svr，线程池同步svr
* 异步编程包含3个模块
* ClientD 负责和客户端的网络交互
* MainD   做异步逻辑的工作
* BackD   负责和后台的网络交互
*
* 同步编程svr,包含2个模块
* ClientD 负责和客户端的网络交互，他把数据收完后再交给Pool
* Pool    同步线程处理动作



* 例子，具体请看 CompleteFun.h 的说明：

* async_echo_svr 异步模式实现，client发送数据过来，然后直接返回
* async_web_svr  异步模式的web上传下载服务器实现
* pool_web_svr   同步模式的web上传下载服务器实现
* </pre>
**/



using namespace std;


class CTask;
class CSearchTheadPool;
class CClientNetSvr;
class CBackNetSvr;
class CMainAsyncSvr;

/*
    异步svr
	实现异步模式需要的步骤：
	1. 实现 int CClientSocketReq::is_complete(char *data,unsigned data_len)
	2. 实现 int CBackSocketReq::is_complete(char *data,unsigned data_len)
	3. 实现 xCallbackObj *CMainAsyncSvr::CreateAsyncObj(CDataBuf *item)
	请看 async_echo_svr 例子 和 async_web_svr 例子

	同步模式和异步模式的切换，非常简单，async_web_svr 和 pool_web_svr 差不多一样。

*/
typedef int ext_handler(CASyncSvr* svr, unsigned cli_flow, void* param);

class CSem {
public:
    CSem() {
        sem_init(&_sem, 0 ,0);
    }

    ~CSem() {
        sem_destroy(&_sem);
    }

    void Wait() {
        sem_wait(&_sem);
    }

    void Notify() {
        sem_post(&_sem);
    }

private:
    sem_t _sem;
};


class CASyncSvr {
public:
    CASyncSvr(const char *strip,int port,int max_conn,int threadnum,bool basync,unsigned svr_id);
    virtual ~CASyncSvr() {}

    int Start(int listen_sd);
    int Stop();
    bool IsAsyncSvr() { return _basync_svr; }

	// 不一定需要，但可以用于初始化业务变量
    virtual void Initail() { }
    virtual void Finish() {}

    // 同步模式，创建业务Task
    virtual CTask* CCreatePoolTask();

    void SendBack(unsigned clid_flow,char *data,unsigned len,unsigned _op = SEND_NORMAL);

    bool RegiCloseHandler(ext_handler* close_handler, void* close_handler_param);

public:
    string _svr_ip;
    int _svr_port;
    int _max_conn;
    int _thread_num;
    bool _basync_svr;
    unsigned _svr_id;

    CMetaThread _clientWorker;
    CClientNetSvr *_client_d;

    CMetaThread _mainWorker;
    CMainAsyncSvr *_main_d;

    CMetaThread _backWorker;
    CBackNetSvr *_back_d;

    CSearchTheadPool *_pool;
    void* rpc_param;
    ext_handler* _close_handler;
    void* _close_handler_param;
};

/*
	同步模式
	开发者只需要继承，并实现Run函数，Initial和Finish用于创建和释放私有变量
	创建一个svr的步骤：
	1. 实现 CClientSocketReq::is_complete
	2. CTask *CASyncSvr::CCreatePoolTask()
	3. 继承类 CTask，并实现Run函数
	请看 pool_web_svr，其实现一个web svr实现上传下载
*/



class CTask
{
public:
	virtual int Initial(CASyncSvr* svr) = 0;
	virtual void* Run(CDataBuf *item, CASyncSvr* svr) = 0;
    virtual int Finish(CASyncSvr* svr) = 0;

    CTask(){}
    virtual ~CTask(){}
};

class CSearchTheadPool
{
public:

    CSearchTheadPool(CASyncSvr* asvr,int threadNum);
    ~CSearchTheadPool() {
    }

    unsigned GetPendingQueue() {return m_queue_size;}
    int ThreadInfo(string &info);
    // 主线程调用的函数
    int Start();
    int Stop();

    CDataBuf PopDataBuf();
    int PushDataBuf(CASyncSvr* p_svr,unsigned flow,char *data,unsigned len, unsigned ip, unsigned short port);

    static void *ThreadProcess(void *argument);
    int SignStopStatus();
    bool PoolEnable();
    CASyncSvr* p_svr;
    int m_iStop;

    static list<CDataBuf>* clientQueue;

    static CSem* _csem;
    //CMutex _sem_mutex;
    static CMutex* _mutex;

protected:


    int BroadCast();

    int MoveToIdle(pthread_t tid);
    int MoveToBusy(pthread_t tid);

private:

	int m_iThreadNum;
    int m_queue_size;

	list<pthread_t> m_IdleThreadList;
	list<pthread_t> m_BusyThreadList;

};



// kill -USR1 18864
void InitDaemon();
void SignalAll();


extern int app_main(std::string& ip, unsigned short port, int sock_num,
                    int io_thread_num, int worker_num, void* rpc_param=NULL);

extern bool IsClientAsyn;
extern std::string Multi_Process_or_Thread;

#endif
