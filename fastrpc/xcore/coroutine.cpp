#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

CroMgr singleton = coroutine_open();
// 目前是每个线程都有协程
// 但是只有一个主线程首先处理client消息，控制定时器、线程池调度
// 该函数就是用来设置该线程的
void SetCoroutineUsedByCurThread() {
    singleton->threadid = pthread_self();
}

CroMgr GetCroMgr() {
    return singleton;
}

static PFMutMap pf_map;

struct coroutine *
_co_new(struct schedule *S , Closure<void>* closure) {
    struct coroutine * co = (struct coroutine *)malloc(sizeof(*co));
    co->func = closure;
    co->pbfunc = NULL;
    co->sch = S;
    co->cap = 0;
    co->size = 0;
    co->status = COROUTINE_READY;
    co->stack = NULL;
    co->enable_sys_hook = false;
    return co;
}

struct coroutine *
_co_new(struct schedule *S , ::google::protobuf::Closure* closure) {
    struct coroutine * co = (struct coroutine *)malloc(sizeof(*co));
    co->func = NULL;
    co->pbfunc = closure;
    co->sch = S;
    co->cap = 0;
    co->size = 0;
    co->status = COROUTINE_READY;
    co->stack = NULL;
    co->enable_sys_hook = false;
    return co;
}

void
_co_delete(struct coroutine *co) {
    free(co->stack);
    free(co);
}

struct schedule *
coroutine_open(void) {
    struct schedule *S = new schedule();
    S->running = -1;
    S->threadid = -1;
    return S;
}

struct schedule * coroutine_create(void) {
    struct schedule * new_s = coroutine_open();
    new_s->threadid = pthread_self();
    return new_s;
}

void
coroutine_close(struct schedule *S) {
    size_t i;
    for (i=0;i<S->co_dic.size();i++) {
        struct coroutine * co = S->co_dic[i];
        if (co) {
            _co_delete(co);
        }
    }
    S->co_dic.clear();
    delete S;
}

static int keyid = 0;

int
NextNewId(int& keyid) {
        if (++keyid>2000000000) keyid = 1;
        return keyid;
}

int
coroutine_new(struct schedule *S, Closure<void>* closure) {
    struct coroutine *co = _co_new(S, closure);
    while (true) {
            int i = NextNewId(keyid);
            CoMap::iterator it = S->co_dic.find(i);
            if (it != S->co_dic.end()) continue;
            S->co_dic.insert(pair<int,struct coroutine *>(i,co));
            return i;
    }
}

int
coroutine_new(struct schedule *S, ::google::protobuf::Closure* closure) {
    struct coroutine *co = _co_new(S, closure);
    while (true) {
            int i = NextNewId(keyid);
            CoMap::iterator it = S->co_dic.find(i);
            if (it != S->co_dic.end()) continue;
            S->co_dic.insert(pair<int,struct coroutine *>(i,co));
            return i;
    }
}

static void
mainfunc(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    struct schedule *S = (struct schedule *)ptr;
    int id = S->running;
    CoMap::iterator it = S->co_dic.find(id);
    if (it != S->co_dic.end()) {
            struct coroutine *C = it->second;
            if (C->func) {
                    C->func->Run();
            } else if (C->pbfunc) {
                    C->pbfunc->Run();
            }
            _co_delete(C);
            S->co_dic.erase(it);
    }
    S->running = -1;
}

void
coroutine_resume(struct schedule * S, int id) {
    assert(S->running == -1);
    assert(id >=0);
    CoMap::iterator it = S->co_dic.find(id);
    if (it == S->co_dic.end()) return;
    struct coroutine *C = it->second;
    if (C == NULL) return;
    int status = C->status;
    uintptr_t ptr = NULL;
    switch(status) {
    case COROUTINE_READY:
        getcontext(&C->ctx);
        C->ctx.uc_stack.ss_sp = S->stack;
        C->ctx.uc_stack.ss_size = STACK_SIZE;
        C->ctx.uc_link = &S->main;
        S->running = id;
        C->status = COROUTINE_RUNNING;
        ptr = (uintptr_t)S;
        makecontext(&C->ctx, (void (*)(void)) mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        swapcontext(&S->main, &C->ctx);
        break;
    case COROUTINE_SUSPEND:
        memcpy(S->stack + STACK_SIZE - C->size, C->stack, C->size);
        S->running = id;
        C->status = COROUTINE_RUNNING;
        swapcontext(&S->main, &C->ctx);
        break;
    default:
        assert(0);
    }
}

static void
_save_stack(struct coroutine *C, char *top) {
    char dummy = 0;
    assert(top - &dummy <= STACK_SIZE);
    if (C->cap < top - &dummy) {
        free(C->stack);
        C->cap = top-&dummy;
        C->stack = (char *)malloc(C->cap);
    }
    C->size = top - &dummy;
    memcpy(C->stack, &dummy, C->size);
}

void
coroutine_yield(struct schedule * S) {
    int id = S->running;
    assert(id >= 0);
    struct coroutine * C = S->co_dic[id];
    assert((char *)&C > S->stack);
    _save_stack(C,S->stack + STACK_SIZE);
    C->status = COROUTINE_SUSPEND;
    S->running = -1;
    swapcontext(&C->ctx , &S->main);
}

int
coroutine_status(struct schedule * S, int id) {
    assert(id>=0);
    CoMap::iterator it = S->co_dic.find(id);
    if (it == S->co_dic.end()) {
        return COROUTINE_DEAD;
    }
    return it->second->status;
}

int
coroutine_running(struct schedule * S) {
    if ((unsigned)pthread_self() != S->threadid) return -1;
    return S->running;
}


bool ProcessWithNewCro(Closure<void>* closure) {
    CroMgr mgr = GetCroMgr();
    if (-1 != coroutine_running(mgr)) {
        closure->Run();
        return false;
    } else {
        int croid = coroutine_new(mgr, closure);
        coroutine_resume(mgr, croid);
    }
    return true;
}

bool ProcessWithNewCro(::google::protobuf::Closure* closure) {
    CroMgr mgr = GetCroMgr();
    if (-1 != coroutine_running(mgr)) {
        closure->Run();
        return false;
    } else {
        int croid = coroutine_new(mgr, closure);
        coroutine_resume(mgr, croid);
    }
    return true;
}

void co_resume_in_suspend(CroMgr mgr, int croid) {
    if (mgr && COROUTINE_SUSPEND == coroutine_status(mgr, croid)) {
        coroutine_resume(mgr, croid);
    }
}

void co_disable_hook_sys() {
    CroMgr cro_mgr = GetCroMgr();
    int id = coroutine_running(cro_mgr);
    assert(id >= 0);
    CoMap::iterator it = cro_mgr->co_dic.find(id);
    assert(it != cro_mgr->co_dic.end());
    struct coroutine * C = it->second;
    C->enable_sys_hook = false;
}

// 只有当前在协程中才会hook
bool co_is_enable_sys_hook() {
    CroMgr cro_mgr = GetCroMgr();
    if (!cro_mgr) return false;
    int id = coroutine_running(cro_mgr);
    if (-1 == id) return false;
    CoMap::iterator it = cro_mgr->co_dic.find(id);
    assert(it != cro_mgr->co_dic.end());
    struct coroutine * C = it->second;
    return C->enable_sys_hook;
}

void co_log_err(const char* fmt, ...) {}


