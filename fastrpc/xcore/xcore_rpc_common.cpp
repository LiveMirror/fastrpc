#include "xcore_rpc_common.h"

std::deque< PbClosure* > RpcMgr::outside_queue;
XSemaphore RpcMgr::m_recvsem;
XMutex RpcMgr::m_recvmux;

void RpcMgr::PutOutSideQueue(PbClosure* done) {
    m_recvmux.lock();
    outside_queue.push_back(done);
    m_recvmux.unlock();
    m_recvsem.post();
}

PbClosure* RpcMgr::GetOutSideQueue(unsigned msec) {
    m_recvsem.trywait(msec);
    m_recvmux.lock();
    PbClosure* done = NULL;
    if (!outside_queue.empty()) {
        done = outside_queue.front();
        outside_queue.pop_front();
    }
    m_recvmux.unlock();
    return done;
}

void RpcMgr::RunWithCoroutine(PbClosure* closure) {
    ProcessWithNewCro(closure);
}

void RpcMgr::RunWithCoroutine(Closure<void>* closure) {
    ProcessWithNewCro(closure);
}

void DeleteAfterRun(Closure<void>* closure) {
    closure->Run();
    delete closure;
}

void CroTimer::on_timer(XTimer* pTimer, uint32 id, void* ptr) {
    if (NULL == closure) return;
    --repeat;
    PbClosure* cro_closure = NULL;
    if (repeat <= 0) {
        Closure<void>* del_closure = NewClosure(DeleteAfterRun, closure);
        cro_closure =
            ::google::protobuf::NewCallback(&RpcMgr::RunWithCoroutine, del_closure);
        closure = NULL;
    } else {
        cro_closure =
            ::google::protobuf::NewCallback(&RpcMgr::RunWithCoroutine, closure);
    }
    RpcMgr::PutOutSideQueue(cro_closure);
}

::google::protobuf::Message* PbMgr::CreateMessage(const std::string& typeName) {
    ::google::protobuf::Message* message = NULL;
    const ::google::protobuf::Descriptor* descriptor =
        ::google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    if (descriptor) {
        const ::google::protobuf::Message* prototype =
            ::google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype) {
            message = prototype->New();
        }
    }
    return message;
}

/////QueMgr//////

void QueMgr::PutQueue(Closure<void>* closure) {
    m_mux.lock();
    m_queue.push_back(closure);
    m_mux.unlock();
    m_sem.post();
}

Closure<void>* QueMgr::GetQueue() {
    m_sem.wait();
    m_mux.lock();
    Closure<void>* closure = NULL;
    if (!m_queue.empty()) {
        closure = m_queue.front();
        m_queue.pop_front();
    }
    m_mux.unlock();
    return closure;
}

/////TpMgr/////

std::map<unsigned, ThreadCroInfo> TPMgr::th_cro_map;
XMutex TPMgr::th_mut;

void TPMgr::AddThreadCroInfo() {
    ThreadCroInfo tc_info;
    tc_info.cro_mgr = coroutine_create();
    tc_info.que_mgr = new QueMgr();
    unsigned tid = pthread_self();
    th_mut.lock();
    TPMgr::th_cro_map[tid] = tc_info;
    th_mut.unlock();
}

void TPMgr::DelThreadCroInfo() {
    unsigned tid = pthread_self();
    th_mut.lock();
    std::map<unsigned, ThreadCroInfo>::iterator it = TPMgr::th_cro_map.find(tid);
    if (it != th_cro_map.end()) {
        ThreadCroInfo& tc_info = it->second;
        delete tc_info.cro_mgr;
        delete tc_info.que_mgr;
    }
    th_mut.unlock();
}

ThreadCroInfo* TPMgr::GetThreadCroInfo(unsigned tid) {
    if (0 == tid) tid = pthread_self();
    ThreadCroInfo* ret = NULL;
    th_mut.lock();
    std::map<unsigned, ThreadCroInfo>::iterator it = TPMgr::th_cro_map.find(tid);
    if (it != th_cro_map.end()) {
        ret = &(it->second);
    }
    th_mut.unlock();
    return ret;
}

TPMgr::TPMgr(int a_thread_num) {
    m_is_stop = false;
    m_thread_num = a_thread_num;
    cur_use_tid = 0;
        // start threads
    for (int i = 0; i < m_thread_num; ++i) {
#ifdef __WINDOWS__
        HANDLE hThread;
        unsigned threadID;
        hThread = (HANDLE)_beginthreadex(NULL,0,TPProcess,this,0, &threadID);
        proc_thread_list.push_back(hThread);
#endif//__WINDOWS__
#ifdef __GNUC__
        pthread_t tid = 0;
        pthread_create(&tid,NULL,TPProcess,this);
        proc_thread_list.push_back(tid);
#endif//__GNUC__
    }
    // 暂时这样保证每个线程的协程创建完了再用
    while ((int)tid_list.size() != a_thread_num) {
        usleep(1000);
    }
}

TPMgr::~TPMgr() {
    m_is_stop = true;
    for (int i = 0; i < m_thread_num; ++i) {
#ifdef __WINDOWS__
        HANDLE m_handle = proc_thread_list[i];
        //TerminateThread(m_handle, 0);
        WaitForSingleObject(m_handle, INFINITE);
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
#endif//__WINDOWS__
#ifdef __GNUC__
        pthread_t m_id = proc_thread_list[i];
        //pthread_cancel(m_id);
        pthread_join(m_id, NULL);
#endif//__GNUC__
    }
    proc_thread_list.clear();
}

void ResumeAfterRun(Closure<void>* closure, PbClosure* cro_done) {
    closure->Run();
    if (cro_done) {
        RpcMgr::PutOutSideQueue(cro_done);
    }
}

void TPStartNewCro(ThreadCroInfo* tc_info, Closure<void>* closure) {
    int croid = coroutine_new(tc_info->cro_mgr, closure);
    coroutine_resume(tc_info->cro_mgr, croid);
}


int TPMgr::AddJob(Closure<void>* closure) {
    CroMgr mgr = GetCroMgr();
    int croid = coroutine_running(mgr);
    PbClosure* cro_done = NULL;
    if (-1 != croid) {
        // 执行完后恢复主线程协程
        cro_done = ::google::protobuf::NewCallback(&coroutine_resume, mgr, croid);
    }
    Closure<void>* th_run_closure =
        NewClosure(ResumeAfterRun, closure, cro_done);
    cur_use_tid = ++cur_use_tid % m_thread_num;
    unsigned tid = tid_list[cur_use_tid];
    ThreadCroInfo* tc_info = TPMgr::GetThreadCroInfo(tid);
    assert(NULL != tc_info);
    // 线程池里启动新的协程来执行job
    Closure<void>* run_with_newcro = NewClosure(TPStartNewCro, tc_info, th_run_closure);
    tc_info->que_mgr->PutQueue(run_with_newcro);
    if (-1 != croid) {
        coroutine_yield(mgr); // 放权
    }
    return 0;
}

int TPMgr::AsynAddJob(Closure<void>* closure) {
    cur_use_tid = ++cur_use_tid % m_thread_num;
    unsigned tid = tid_list[cur_use_tid];
    ThreadCroInfo* tc_info = TPMgr::GetThreadCroInfo(tid);
    assert(NULL != tc_info);
    // 线程池里启动新的协程来执行job
    Closure<void>* run_with_newcro = NewClosure(TPStartNewCro, tc_info, closure);
    tc_info->que_mgr->PutQueue(run_with_newcro);
    return 0;
}

ProcessType TPProcess(void *argument) {
    TPMgr::AddThreadCroInfo();

    TPMgr &tp_mgr = (  *((TPMgr*)argument) );
    unsigned tid = pthread_self();
    tp_mgr.m_mut_for_start.lock();
    // 通过统计tid来保证子线程都执行完主线程再继续
    tp_mgr.tid_list.push_back(tid);
    tp_mgr.m_mut_for_start.unlock();

    ThreadCroInfo* tc_info = TPMgr::GetThreadCroInfo();
    while (!tp_mgr.m_is_stop) {
        Closure<void>* closure = tc_info->que_mgr->GetQueue();
        if (closure) closure->Run();
    }
    TPMgr::DelThreadCroInfo();
}




