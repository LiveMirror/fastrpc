// 2013-05-22
// common define for rpc
#ifndef _XCORE_RPC_COMMON_H_
#define _XCORE_RPC_COMMON_H_

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
#include <deque>

#include "xcore_define.h"
#include "xcore_mutex.h"
#include "xcore_semaphore.h"
#include "xcore_timer.h"
#include "coroutine.h"

typedef ::google::protobuf::Closure PbClosure;

class RpcMgr {
public:
    static void PutOutSideQueue(PbClosure* done);
    static PbClosure* GetOutSideQueue(unsigned msec);
    static void RunWithCoroutine(PbClosure* closure);
    static void RunWithCoroutine(Closure<void>* closure);

    static std::deque< PbClosure* > outside_queue;
    static XSemaphore   m_recvsem;
    static XMutex       m_recvmux;
};

class CroTimer : public xcore::XTimer::ICallBack {
public:
    CroTimer(Closure<void>* a_closure, int a_repeat) {
        closure = a_closure;
        repeat = a_repeat;
    }
    virtual ~CroTimer() {
        if (closure)
            delete closure;
    }
    virtual void on_timer(XTimer* pTimer, uint32 id, void* ptr);
    int repeat;
    Closure<void>* closure;
};

class TimerMgr {
public:
    TimerMgr() {
        timer.start();
    }
    ~TimerMgr() {
        timer.stop();
    }
    unsigned AddJob(unsigned timeout, Closure<void>* closure, int repeat = 1) {
        CroTimer* ct = new CroTimer(closure, repeat);
        return timer.schedule(ct, (uint32)timeout, repeat);
    }
    unsigned DelJob(unsigned uid) {
        return timer.cancel(uid);
    }
    xcore::XTimer timer;
};

class PbMgr {
public:
    static ::google::protobuf::Message* CreateMessage(const std::string& typeName);
};

#ifdef __WINDOWS__
#define ProcessType unsigned __stdcall
#endif//__WINDOWS__
#ifdef __GNUC__
#define ProcessType void*
#endif//__GNUC__

ProcessType TPProcess(void *argument);

class TPMgr {
public:
    TPMgr(int a_thread_num);
    ~TPMgr();

    void PutProcQueue(Closure<void>* closure);
    Closure<void>* GetProcQueue();

    int AddJob(Closure<void>* closure);

public:
    bool m_is_stop;
    int m_thread_num;
    std::deque<Closure<void>*> proc_queue;
#ifdef __WINDOWS__
    std::vector<HANDLE> proc_thread_list;
#endif//__WINDOWS__
#ifdef __GNUC__
    std::vector<pthread_t> proc_thread_list;
#endif//__GNUC__
    XSemaphore m_proc_sem;
    XMutex m_proc_mux;
};

#endif // end of _XCORE_RPC_COMMON_H_

