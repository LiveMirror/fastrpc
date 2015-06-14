#ifndef C_COROUTINE_H
#define C_COROUTINE_H

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

#include "closure.h"
#include "xcore_mutex.h"

#include <ucontext.h>
#include <poll.h>
#include <google/protobuf/stubs/common.h>

#define STACK_SIZE (1024*1024*8)
#define DEFAULT_COROUTINE 16

typedef ::google::protobuf::Closure PbClosure;

struct schedule {
    char stack[STACK_SIZE];
    ucontext_t main;
    int nco;
    int cap;
    int running;
    struct coroutine **co;
    bool enable_sys_hook;
};

struct coroutine {
    Closure<void>* func;
    ::google::protobuf::Closure* pbfunc;
    ucontext_t ctx;
    struct schedule * sch;
    ptrdiff_t cap;
    ptrdiff_t size;
    int status;
    char *stack;
};

struct schedule * coroutine_open(void);
void coroutine_close(struct schedule *);

int coroutine_new(struct schedule *, Closure<void>* closure);
int coroutine_new(struct schedule *,
                  ::google::protobuf::Closure* closure);
void coroutine_resume(struct schedule *, int id);
int coroutine_status(struct schedule *, int id);
int coroutine_running(struct schedule *);
void coroutine_yield(struct schedule *);


typedef struct schedule * CroMgr;
extern CroMgr singleton;
CroMgr GetCroMgr();
bool ProcessWithNewCro(Closure<void>* closure);
bool ProcessWithNewCro(::google::protobuf::Closure* closure);
void co_resume_in_suspend(CroMgr mgr, int croid);

void co_enable_hook_sys();
void co_disable_hook_sys();
bool co_is_enable_sys_hook();

void co_log_err(const char* fmt, ...);

struct MyPollFd {
public:
    MyPollFd() {
        croid = -1;
        revents = 0;
    }
    int croid;
    short revents;
};
typedef struct MyPollFd POFD;

class ClosureMap {
public:
    bool Insert(int key, PbClosure* closure) {
        _mux.lock();
        clo_map_[key] = closure;
        _mux.unlock();
        return true;
    }
    PbClosure* Pop(int key) {
        _mux.lock();
        PbClosure* ret = NULL;
        std::map<int, PbClosure*>::iterator it = clo_map_.find(key);
        if (it != clo_map_.end()) {
            ret = it->second;
            clo_map_.erase(it);
        }
        _mux.unlock();
        return ret;
    }
    std::map<int, PbClosure*> clo_map_;
    XMutex _mux;
};

class PFMutMap {
public:
    bool Insert(int key, POFD& a_pfd) {
        _mux.lock();
        pofd_map_[key] = a_pfd;
        _mux.unlock();
        return true;
    }

    POFD Pop(int key) {
        _mux.lock();
        POFD ret;
        std::map<int, POFD>::iterator it = pofd_map_.find(key);
        if (it != pofd_map_.end()) {
            ret = it->second;
            pofd_map_.erase(it);
        }
        _mux.unlock();
        return ret;
    }

    POFD Get(int key) {
        _mux.lock();
        POFD ret;
        std::map<int, POFD>::iterator it = pofd_map_.find(key);
        if (it != pofd_map_.end()) {
            ret = it->second;
        }
        _mux.unlock();
        return ret;
    }

    bool Set(int key, POFD& a_pofd) {
        bool ret = false;
        _mux.lock();
        std::map<int, POFD>::iterator it = pofd_map_.find(key);
        if (it != pofd_map_.end()) {
             it->second = a_pofd;
             ret = true;
        }
        _mux.unlock();
        return ret;
    }

    std::map<int, POFD> pofd_map_;
    XMutex _mux;
};

extern int hook_epollfd;

void* HookEpollLoop(void *argument);

void StartHookEpoll();

class HookMgr {
public:
    HookMgr();
    ~HookMgr();
};

#endif
