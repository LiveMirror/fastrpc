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

class RpcMgr {
public:
    static void PutOutSideQueue(PbClosure* done);
    static PbClosure* GetOutSideQueue(unsigned msec);
    static void RunWithCoroutine(PbClosure* closure);
    static void RunWithCoroutine(Closure<void>* closure);
    static void AsynRunWithCoro(Closure<void>* closure);

    static std::deque< PbClosure* > outside_queue;
    static XSemaphore   m_recvsem;
    static XMutex       m_recvmux;
};

#define AsynRun(...) RpcMgr::AsynRunWithCoro(NewClosure(__VA_ARGS__))

class CroTimer : public xcore::XTimer::ICallBack {
public:
    CroTimer(Closure<void>* a_closure, int a_repeat) {
        closure = a_closure;
        repeat = a_repeat;
    }
    virtual ~CroTimer() {
        if (closure) {
                delete closure;
                closure = NULL;
        }
    }
    virtual void on_timer(XTimer* pTimer, uint32 id, void* ptr);
    Closure<void>* closure;
    int repeat;
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

class QueMgr {
public:
    void PutQueue(Closure<void>* closure);
    Closure<void>* GetQueue();
    std::deque<Closure<void>*> m_queue;
    XSemaphore m_sem;
    XMutex m_mux;
};

#define TPRun(...) AddJob(NewClosure(__VA_ARGS__))
#define TPAsynRun(...) AsynAddJob(NewClosure(__VA_ARGS__))

struct ThreadCroInfo {
    CroMgr cro_mgr;
    QueMgr* que_mgr; 
};

ProcessType TPProcess(void *argument);

class TPMgr {
public:
    TPMgr(int a_thread_num);
    ~TPMgr();

    int AddJob(Closure<void>* closure);
    int AsynAddJob(Closure<void>* closure);

public:
    bool m_is_stop;
    int m_thread_num;
#ifdef __WINDOWS__
    std::vector<HANDLE> proc_thread_list;
#endif//__WINDOWS__
#ifdef __GNUC__
    std::vector<pthread_t> proc_thread_list;
#endif//__GNUC__
    unsigned cur_use_tid;

    std::vector<unsigned> tid_list;
    XMutex m_mut_for_start;

public:
    static void AddThreadCroInfo();
    static void DelThreadCroInfo();
    static ThreadCroInfo* GetThreadCroInfo(unsigned tid = 0);
public:
    static std::map<unsigned, ThreadCroInfo> th_cro_map;
    static XMutex th_mut;
};

#endif // end of _XCORE_RPC_COMMON_H_

