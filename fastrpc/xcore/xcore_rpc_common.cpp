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
    --repeat;
    PbClosure* cro_closure = NULL;
    if (repeat <= 0) {
        Closure<void>* del_closure = NewPermanentClosure(DeleteAfterRun, closure);
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

TPMgr::TPMgr(int a_thread_num) {
    m_is_stop = false;
    m_thread_num = a_thread_num;
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
}

TPMgr::~TPMgr() {
    m_is_stop = true;
    for (int i = 0; i < m_thread_num; ++i) {
        m_proc_sem.post();
    }
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

void TPMgr::PutProcQueue(Closure<void>* closure) {
    m_proc_mux.lock();
    proc_queue.push_back(closure);
    m_proc_mux.unlock();
    m_proc_sem.post();
}

Closure<void>* TPMgr::GetProcQueue() {
    m_proc_sem.wait();
    m_proc_mux.lock();
    Closure<void>* closure = NULL;
    if (!proc_queue.empty()) {
        closure = proc_queue.front();
        proc_queue.pop_front();
    }
    m_proc_mux.unlock();
    return closure;
}

void ResumeAfterRun(Closure<void>* closure, PbClosure* cro_done) {
    closure->Run();
    if (cro_done) {
        RpcMgr::PutOutSideQueue(cro_done);
    }
}

int TPMgr::AddJob(Closure<void>* closure) {
    CroMgr mgr = GetCroMgr();
    int croid = coroutine_running(mgr);
    PbClosure* cro_done = NULL;
    if (-1 != croid) {
        cro_done = ::google::protobuf::NewCallback(&coroutine_resume, mgr, croid);
    }
    Closure<void>* th_run_closure =
        NewClosure(ResumeAfterRun, closure, cro_done);
    PutProcQueue(th_run_closure);
    if (-1 != croid) {
        coroutine_yield(mgr); // 放权
    }
    return 0;
}

ProcessType TPProcess(void *argument) {
    TPMgr &tp_mgr = (  *((TPMgr*)argument) );
    while (!tp_mgr.m_is_stop) {
        Closure<void>* closure = tp_mgr.GetProcQueue();
        if (closure) {
            closure->Run();
        }
    }
}




