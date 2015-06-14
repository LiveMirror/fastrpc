#include <iostream>
#include <assert.h>
#include <sys/time.h>

#include "CSvr.h"
#include <unistd.h>


using namespace std;

list<CDataBuf>* CSearchTheadPool::clientQueue = NULL;
CSem* CSearchTheadPool::_csem = NULL;
CMutex* CSearchTheadPool::_mutex = NULL;

CASyncSvr::CASyncSvr(const char *strip,int port,int max_conn,int threadnum,bool basync,unsigned svr_id)
{
    _svr_ip = strip;
    _svr_port = port;

    _client_d = NULL;
    _main_d = NULL;
    _back_d = NULL;
    _pool = NULL;

    _max_conn = max_conn;
    _basync_svr = basync;
    _svr_id = svr_id;

    _thread_num = threadnum;

    _close_handler_param = NULL;
    _close_handler = NULL;
}

bool CASyncSvr::RegiCloseHandler(ext_handler* close_handler, void* close_handler_param) {
    if (!close_handler) {
        return false;
    }
    _close_handler = close_handler;
    _close_handler_param = close_handler_param;
    return true;
}

int CASyncSvr::Start(int listen_sd)
{
    // ClientD 线程
    _client_d = new CClientNetSvr(this,_max_conn);
    assert(_client_d!=NULL);
    _client_d->Start(listen_sd);
    _clientWorker.start(CClientNetSvr::run,(void*)_client_d);

    if ( IsAsyncSvr() ) {

        _main_d = new CMainAsyncSvr();assert(_main_d!=NULL);
        _mainWorker.start(CMainAsyncSvr::run,(void*)_main_d);

        // 启动BackD，后端svr不需要很多连接socket
        _back_d = new CBackNetSvr(100,200,200);assert(_back_d!=NULL);
        _back_d->Start();
        _backWorker.start(CBackNetSvr::run,(void*)_back_d);
    }
    else {
        _pool = new CSearchTheadPool(this,_thread_num);
        assert(_pool!=NULL);
        _pool->Start();
        LOG(LOG_ALL,"Info:Thread Pool %d Create.Svr Start with Pool mode.\n",_thread_num);
    }

    return 0;

}

int CASyncSvr::Stop()
{
    _clientWorker.stop();
    _clientWorker.join();

    if ( IsAsyncSvr() ) {
        _mainWorker.stop();
        _mainWorker.join();

        _backWorker.stop();
        _backWorker.join();
    }else {
        _pool->Stop();
    }
/*    if(_client_d!=NULL){
        delete _client_d;
    }
    if(_main_d!=NULL){
        delete _main_d;
    }
    if(_back_d!=NULL){
        delete _back_d;
    }
    if(_pool!=NULL){
        delete _pool;
    } */
    return 0;
}

void CASyncSvr::SendBack(unsigned clid_flow,char *data,unsigned len,unsigned _op ) {
    MainD_CliD(this,clid_flow,data,len,_op);
}

CSearchTheadPool::CSearchTheadPool(CASyncSvr* asvr, int threadNum)
{
    m_queue_size = 0;
    p_svr = asvr;
    m_iThreadNum = threadNum;
    m_iStop = 0;
    m_ptask = NULL;
}

int CSearchTheadPool::Start()
{
    for(int i = 0; i < m_iThreadNum;i++)
    {
        pthread_t tid = 0;
        int ret = pthread_create(&tid,NULL,CSearchTheadPool::ThreadProcess,this);
        assert(ret==0);
        m_IdleThreadList.push_back(tid);
        usleep(10);
    }
    return 0;
}

int CSearchTheadPool::Stop()
{
    CSearchTheadPool::SignStopStatus();
    BroadCast();
    if ( m_IdleThreadList.empty() && m_BusyThreadList.empty() ) return 0;

    LOG(LOG_ALL,"\nInfo:Server start stop  %d worker thread.\n",m_iThreadNum);
    unsigned i = 0;

    list<pthread_t>::iterator iter = m_IdleThreadList.begin();
    while( iter != m_IdleThreadList.end() )
    {
        pthread_join(*iter,NULL);
        //LOG(LOG_ALL,"Idle Thread %d Stop\n",int(*iter));
        iter++;
        i++;
    }

    iter = m_BusyThreadList.begin();
    while( iter != m_BusyThreadList.end() )
    {
        pthread_join(*iter,NULL);
        //LOG(LOG_ALL,"Busy Thread %d Stop\n",int(*iter));
        iter++;
        i++;
    }

    m_IdleThreadList.clear();
    m_BusyThreadList.clear();
    LOG(LOG_ALL,"Info:Server stop %u worker thread finish.\n",i);
    return 0;
}


void *CSearchTheadPool::ThreadProcess(void *argument)
{
	pthread_t tid = pthread_self();
    LOG(LOG_ALL,"Info:IndexD CSearchTheadPool thread create,pid=%d,tid=%u\n",getpid(),tid);

	CSearchTheadPool &pool = (  *((CSearchTheadPool*)argument) );

	CTask *ptask = pool.p_svr->CCreatePoolTask();
	assert(ptask!=NULL);

	int ret = ptask->Initial(pool.p_svr);
	if ( ret!=0 ) {
		LOG(LOG_ALL,"Error:Initial Task return %d\n",ret);
		exit(0);
	}
    pool.m_ptask = ptask;

	while ( pool.PoolEnable() )
	{
		CDataBuf in_buf = pool.PopDataBuf();
		ptask->Run(&in_buf, in_buf.svr);
        in_buf.Free();
	}
    ptask->Finish(pool.p_svr);
	delete ptask;
	return (void*)0;
}

int CSearchTheadPool::PushDataBuf(CASyncSvr* p_svr,unsigned flow,char *data,unsigned len, unsigned ip, unsigned short port)
{
    CDataBuf buf;
    buf.destinfo._ip = ip;
    buf.destinfo._port = port;
    buf.Copy(data,len,flow);
    buf.svr = p_svr;

    //不走pool线程了 直接扔给业务端 modify at 2015-06-05
    //
    if (!m_ptask) {
        printf("has not been started yet\n");
        return -1;
    }

    m_ptask->Run(&buf, buf.svr);
    buf.Free();

    //{
    //    MutexGuard g(*CSearchTheadPool::_mutex);
    //    CSearchTheadPool::clientQueue->push_back(buf);
    //    m_queue_size++;
    //}
    //CSearchTheadPool::_csem->Notify();
    return 0;
}

CDataBuf CSearchTheadPool::PopDataBuf()
{
    CDataBuf buf;
    CSearchTheadPool::_csem->Wait();

    MutexGuard g(*CSearchTheadPool::_mutex);
    if ( !CSearchTheadPool::clientQueue->empty() )
    {
        buf = CSearchTheadPool::clientQueue->front();
        CSearchTheadPool::clientQueue->pop_front();
        m_queue_size--;
    }
    return buf;
}

int CSearchTheadPool::BroadCast()
{
    for(int i=0; i<m_iThreadNum*8; i++) CSearchTheadPool::_csem->Notify();
    return 0;
}



int CSearchTheadPool::MoveToIdle(pthread_t tid)
{
	MutexGuard g(*CSearchTheadPool::_mutex);

	list<pthread_t>::iterator busyIter = m_BusyThreadList.begin();
	while( busyIter != m_BusyThreadList.end() )
	{
		if( tid == (*busyIter) )
		{
			break;
		}
		busyIter++;
	}
	m_BusyThreadList.erase(busyIter);
	m_IdleThreadList.push_back(tid);
	return 0;
}

int CSearchTheadPool::MoveToBusy(pthread_t tid)
{
	MutexGuard g(*CSearchTheadPool::_mutex);

	list<pthread_t>::iterator idleIter = m_IdleThreadList.begin();
	while( idleIter != m_IdleThreadList.end() )
	{
		if( tid == (*idleIter) )
		{
			break;
		}
		idleIter++;
	}
	m_IdleThreadList.erase(idleIter);
	m_BusyThreadList.push_back(tid);
	return 0;
}

int CSearchTheadPool::SignStopStatus()
{
    m_iStop = 1;
    return 0;
}

bool CSearchTheadPool::PoolEnable() {
    return (m_iStop==0);
}

int CSearchTheadPool::ThreadInfo(string &info)
{
    // io层消息队列取消打印也没意义 modify at 2015-06-05
	//ostringstream oss;
	//oss.str("");
	//oss << string("OK\r\n");

    //{
    //    MutexGuard g(*CSearchTheadPool::_mutex);
    //    oss << m_BusyThreadList.size() << string(" Busy Thread.\n");
    //    oss << m_IdleThreadList.size() << string(" Idle Thread.\n");
    //}

    //oss << GetPendingQueue() << string(" Task waiting to pending .\n");
	//info = oss.str();

	return 0;
}


void sig_handle(int sig_val)
{
    return;
}

void InitDaemon()
{
    pid_t pid;

    if ((pid = fork() ) != 0 )
    {
        exit( 0);
    }

    setsid();

    signal( SIGINT,  SIG_IGN);
    signal( SIGHUP,  SIG_IGN);
    signal( SIGPIPE, SIG_IGN);
    signal( SIGTTOU, SIG_IGN);
    signal( SIGTTIN, SIG_IGN);
    signal( SIGCHLD, SIG_IGN);
    signal( SIGTERM, SIG_IGN);

    struct sigaction sig;

    sig.sa_handler = SIG_IGN;
    sig.sa_flags = 0;
    sigemptyset( &sig.sa_mask);
    sigaction( SIGHUP,&sig,NULL);

    if ((pid = fork() ) != 0 )
    {
        exit(0);
    }

    umask(0);
    setpgrp();
}

void SignalAll()
{
    if ( signal(SIGUSR1, sig_handle) == SIG_ERR ) {
        printf("signal SIGUSR1 %s\n",strerror(errno));
        exit(0);
    }

    /*if ( signal(SIGILL, sig_handle) == SIG_ERR ) {
    printf("signal SIGILL %s\n",strerror(errno));
    exit(0);
    }

    if ( signal(SIGABRT, sig_handle) == SIG_ERR ) {
    printf("signal SIGABRT %s\n",strerror(errno));
    exit(0);
    }

    if ( signal(SIGBUS, sig_handle) == SIG_ERR ) {
    printf("signal SIGBUS %s\n",strerror(errno));
    exit(0);
    }

    if ( signal(SIGFPE, sig_handle) == SIG_ERR ) {
    printf("signal SIGFPE %s\n",strerror(errno));
    exit(0);
    }
    if ( signal(SIGSEGV, sig_handle) == SIG_ERR ) {
    printf("signal SIGSEGV %s\n",strerror(errno));
    exit(0);
    }

    */
}
typedef struct _thread_param {
    string ip;
    unsigned short port;
    int sock_num;
    int worker_num;
    int listen_sd;
    void* rpc_param;
} Thread_Param;

void* process(void *argument) {
    Thread_Param& tp = (  *((Thread_Param*)argument) );
    unsigned svr_id = 0;
    if (Multi_Process_or_Thread == "Thread") {
        svr_id = pthread_self();
    } else {
        svr_id = getpid();
    }
    printf("svr_id:%u\n", svr_id);
    CASyncSvr* mysvr = new CASyncSvr(tp.ip.c_str(),
                                     tp.port,
                                     tp.sock_num,
                                     tp.worker_num,
                                     false,
                                     svr_id);
    if (tp.rpc_param != NULL) {
        mysvr->rpc_param = tp.rpc_param;
    }
    if ( mysvr==NULL ) {
        LOG(LOG_ALL,"Error:CASyncSvr is NULL.\n");
        exit(0);
    }
    mysvr->Start(tp.listen_sd);

    while(true)
    {
        if ( mysvr->IsAsyncSvr() )
        {
            printf("ClientD queue:%u,MainD queue:%u,BackD queue:%u\n",
                   mysvr->_client_d->mPending.size(), mysvr->_main_d->mPending.size(), mysvr->_back_d->mPending.size());

        }else if ( mysvr->_pool->PoolEnable() )
        {
            string info;
            mysvr->_pool->ThreadInfo(info);
            //printf("svr_id[%u]\n%s\n",svr_id,info.c_str());
        }
        sleep(600);
    }

    mysvr->Stop();
    LOG(LOG_ALL,"server stop.\n");
}
static std::string sip;
static unsigned sport;

int StartWithMultProc(std::string ip, unsigned short port, StartFunction* fun, int proc_num) {
    sip = ip;
    sport = port;
    int listen_sd;
    if ((listen_sd = do_tcplisten(ip.c_str(), port, 10240)) == -1)
    {
        printf("create listening soket error! port:%d,error[%d] info is %s\n", port,errno,strerror(errno));
        exit(-1);
    }
    do_setsocktonoblock(listen_sd);
    //多进程
    if (proc_num <= 0) {
        proc_num = sysconf(_SC_NPROCESSORS_CONF);
        if (proc_num < 1) proc_num = 1;
    }
    for (int i = 0; i < proc_num - 1; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            printf("error in for");
            exit(1);
        } else if (pid == 0) {
            fun(listen_sd);
            return 0;
        }
    }
    fun(listen_sd);
    return 0;
}

int StartOneProc(int listen_sd, int sock_num,
        int io_thread_num, int worker_num, void* rpc_param) {
    CSearchTheadPool::clientQueue = new list<CDataBuf>();
    CSearchTheadPool::_csem = new CSem();
    CSearchTheadPool::_mutex = new CMutex();

    Thread_Param tp;
    tp.ip = sip;
    tp.port = sport;
    tp.sock_num = sock_num;
    tp.worker_num = worker_num;
    tp.listen_sd = listen_sd;
    tp.rpc_param = rpc_param;

    process(&tp);
    return 0;
}

int StartMulThread(std::string& ip, unsigned short port, int sock_num,
             int io_thread_num, int worker_num, void* rpc_param) {
    //InitDaemon();
    //SignalAll();

    //bind PORT端口
    CSearchTheadPool::clientQueue = new list<CDataBuf>();
    CSearchTheadPool::_csem = new CSem();
    CSearchTheadPool::_mutex = new CMutex();
    int listen_sd;
    if ((listen_sd = do_tcplisten(ip.c_str(), port, 10240)) == -1)
    {
        printf("create listening soket error! port:%d,error[%d] info is %s\n", port,errno,strerror(errno));
        exit(-1);
    }
    do_setsocktonoblock(listen_sd);

    Thread_Param tp;
    tp.ip = ip;
    tp.port = port;
    tp.sock_num = sock_num;
    tp.worker_num = worker_num;
    tp.listen_sd = listen_sd;
    tp.rpc_param = rpc_param;

    //多线程
    for(int i = 0; i < io_thread_num - 1; ++i) {
        pthread_t tid = 0;
        int ret = pthread_create(&tid,NULL,process,&tp);
        assert(ret==0);
    }

    process(&tp);

    return 0;
}

bool IsClientAsyn = true;
std::string Multi_Process_or_Thread = "Thread";
