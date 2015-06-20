#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

CroMgr singleton = coroutine_open();
// 暂时一个进程就支持一个协程
// 以后再考虑多线程，不过意义不大
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
    struct schedule *S = (struct schedule *)malloc(sizeof(*S));
    S->nco = 0;
    S->cap = DEFAULT_COROUTINE;
    S->running = -1;
    S->co = (struct coroutine **)malloc(sizeof(struct coroutine *) * S->cap);
    memset(S->co, 0, sizeof(struct coroutine *) * S->cap);
    S->threadid = -1;
    return S;
}

void
coroutine_close(struct schedule *S) {
    int i;
    for (i=0;i<S->cap;i++) {
        struct coroutine * co = S->co[i];
        if (co) {
            _co_delete(co);
        }
    }
    free(S->co);
    S->co = NULL;
    free(S);
}

int
coroutine_new(struct schedule *S, Closure<void>* closure) {
    struct coroutine *co = _co_new(S, closure);
    if (S->nco >= S->cap) {
        int id = S->cap;
        S->co = (struct coroutine **)realloc(S->co, S->cap * 2 * sizeof(struct coroutine *));
        memset(S->co + S->cap , 0 , sizeof(struct coroutine *) * S->cap);
        S->co[S->cap] = co;
        S->cap *= 2;
        ++S->nco;
        return id;
    } else {
        int i;
        for (i=0;i<S->cap;i++) {
            int id = (i+S->nco) % S->cap;
            if (S->co[id] == NULL) {
                S->co[id] = co;
                ++S->nco;
                return id;
            }
        }
    }
    assert(0);
    return -1;
}

int
coroutine_new(struct schedule *S, ::google::protobuf::Closure* closure) {
    struct coroutine *co = _co_new(S, closure);
    if (S->nco >= S->cap) {
        int id = S->cap;
        S->co = (struct coroutine **)realloc(S->co, S->cap * 2 * sizeof(struct coroutine *));
        memset(S->co + S->cap , 0 , sizeof(struct coroutine *) * S->cap);
        S->co[S->cap] = co;
        S->cap *= 2;
        ++S->nco;
        return id;
    } else {
        int i;
        for (i=0;i<S->cap;i++) {
            int id = (i+S->nco) % S->cap;
            if (S->co[id] == NULL) {
                S->co[id] = co;
                ++S->nco;
                return id;
            }
        }
    }
    assert(0);
    return -1;
}

static void
mainfunc(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    struct schedule *S = (struct schedule *)ptr;
    int id = S->running;
    struct coroutine *C = S->co[id];
    if (C->func) {
        C->func->Run();
    } else if (C->pbfunc) {
        C->pbfunc->Run();
    }
    _co_delete(C);
    S->co[id] = NULL;
    --S->nco;
    S->running = -1;
}

void
coroutine_resume(struct schedule * S, int id) {
    assert(S->running == -1);
    assert(id >=0 && id < S->cap);
    struct coroutine *C = S->co[id];
    if (C == NULL)
        return;
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
    struct coroutine * C = S->co[id];
    assert((char *)&C > S->stack);
    _save_stack(C,S->stack + STACK_SIZE);
    C->status = COROUTINE_SUSPEND;
    S->running = -1;
    swapcontext(&C->ctx , &S->main);
}

int
coroutine_status(struct schedule * S, int id) {
    assert(id>=0 && id < S->cap);
    if (S->co[id] == NULL) {
        return COROUTINE_DEAD;
    }
    return S->co[id]->status;
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
    struct coroutine * C = cro_mgr->co[id];
    C->enable_sys_hook = false;
}

// 只有当前在协程中才会hook
bool co_is_enable_sys_hook() {
    CroMgr cro_mgr = GetCroMgr();
    int id = coroutine_running(cro_mgr);
    if (-1 == id) return false;
    struct coroutine * C = cro_mgr->co[id];
    return C->enable_sys_hook;
}

void co_log_err(const char* fmt, ...) {}


