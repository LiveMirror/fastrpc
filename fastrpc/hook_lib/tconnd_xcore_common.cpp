#include "tconnd_xcore_common.h"
#include "tconnd_xcore_byte_order.h"

std::deque< Closure<void>* > XcoreMgr::outside_queue;
XSemaphore XcoreMgr::m_recvsem;
XMutex XcoreMgr::m_recvmux;

void XcoreMgr::PutOutSideQueue(Closure<void>* done) {
    m_recvmux.lock();
    outside_queue.push_back(done);
    m_recvmux.unlock();
    m_recvsem.post();
}

Closure<void>* XcoreMgr::GetOutSideQueue(unsigned msec) {
    m_recvsem.trywait(msec);
    m_recvmux.lock();
    Closure<void>* done = NULL;
    if (!outside_queue.empty()) {
        done = outside_queue.front();
        outside_queue.pop_front();
    }
    m_recvmux.unlock();
    return done;
}

int XcoreMgr::Update(unsigned msec) {
    Closure<void>* done = XcoreMgr::GetOutSideQueue(msec);
    if (NULL == done) {
        return 0;
    }
    done->Run();
    return 1;
}

void XcoreMgr::RunWithCoroutine(Closure<void>* closure) {
    ProcessWithNewCro(closure);
}

void DeleteAfterRun(Closure<void>* closure) {
    closure->Run();
    delete closure;
}

void CroTimer::on_timer(XTimer* pTimer, uint32 id, void* ptr) {
    --repeat;
    Closure<void>* cro_closure = NULL;
    if (repeat <= 0) {
        Closure<void>* del_closure = NewPermanentClosure(DeleteAfterRun, closure);
        cro_closure = NewClosure(XcoreMgr::RunWithCoroutine, del_closure);
        closure = NULL;
    } else {
        cro_closure = NewClosure(XcoreMgr::RunWithCoroutine, closure);
    }
    XcoreMgr::PutOutSideQueue(cro_closure);
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

CroMgr singleton = coroutine_open();

CroMgr GetCroMgr() {
    return singleton;
}

static PFMutMap pf_map;

struct coroutine *
_co_new(struct schedule *S , Closure<void>* closure) {
    struct coroutine * co = (struct coroutine *)malloc(sizeof(*co));
    co->func = closure;
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

struct schedule * coroutine_create(void) {
    struct schedule * new_s = coroutine_open();
    new_s->threadid = pthread_self();
    return new_s;
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

static void
mainfunc(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    struct schedule *S = (struct schedule *)ptr;
    int id = S->running;
    struct coroutine *C = S->co[id];
    if (C->func) {
        C->func->Run();
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

// Âè™ÊúâÂΩìÂâçÂú®ÂçèÁ®ã‰∏≠Êâç‰ºöhook
bool co_is_enable_sys_hook() {
    CroMgr cro_mgr = GetCroMgr();
    if (!cro_mgr) return false;
    int id = coroutine_running(cro_mgr);
    if (-1 == id) return false;
    struct coroutine * C = cro_mgr->co[id];
    return C->enable_sys_hook;
}

void co_log_err(const char* fmt, ...) {}


/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// this is not original code
// modify by feimat

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <set>
#include <map>

#include <dlfcn.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <errno.h>
#include <time.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include <time.h>

#include "tconnd_xcore_common.h"

typedef long long ll64_t;
using std::set;
using std::map;

struct rpchook_t
{
	int user_flag;
	struct sockaddr_in dest; //maybe sockaddr_un;
	int domain; //AF_LOCAL , AF_INET

	struct timeval read_timeout;
	struct timeval write_timeout;
};
static inline pid_t GetPid()
{
	char **p = (char**)pthread_self();
	return p ? *(pid_t*)(p + 18) : getpid();
}
#define MAX_HOOK_SOCK_NUM 20000
static rpchook_t *g_rpchook_socket_fd[MAX_HOOK_SOCK_NUM] = { 0 };
static PFMutMap wait_pf_map;
static ClosureMap clo_map;
static TimerMgr hook_timer_mgr;
int hook_epollfd = -1;

bool HookMgr::is_stop = false;
pthread_t HookMgr::epoll_thread = 0;

unsigned ALL_EPOLL_EV = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR;

void EpollDel(int sockfd, unsigned events) {
    struct epoll_event ev;
    ev.data.u32 = 0UL;
    ev.events = events;
    if (epoll_ctl(hook_epollfd, EPOLL_CTL_DEL, sockfd, &ev) < 0) {
        //printf("epoll_ctl del socket:%d fail\n", sockfd);
    }
}

void EpollAdd(int sockfd,unsigned ev_uid, unsigned events) {
    struct epoll_event ev;
    ev.data.u32 = ev_uid;
    ev.events = events | EPOLLONESHOT;
    while (-1 == hook_epollfd) {
        // ÂàöÂêØÂä®ËøõÁ®ãÊó∂Â∞±Ê≥®ÂÜåÁöÑË¶ÅÁ≠â‰∏Ä‰ºö
        usleep(10);
    }
    if (epoll_ctl(hook_epollfd, EPOLL_CTL_ADD, sockfd, &ev) < 0) {
        //printf("epoll_ctl add socket:%d fail\n", sockfd);
    }
}

static uint32_t PollEvent2Epoll( short events ) {
    uint32_t e = 0;
    if( events & POLLIN ) 	e |= EPOLLIN;
    if( events & POLLOUT )  e |= EPOLLOUT;
    if( events & POLLHUP ) 	e |= EPOLLHUP;
    if( events & POLLERR )	e |= EPOLLERR;
    return e;
}
static short EpollEvent2Poll( uint32_t events ) {
    short e = 0;
    if( events & EPOLLIN ) 	e |= POLLIN;
    if( events & EPOLLOUT ) e |= POLLOUT;
    if( events & EPOLLHUP ) e |= POLLHUP;
    if( events & EPOLLERR ) e |= POLLERR;
    return e;
}

void* HookEpollLoop(void *argument) {
    epoll_event events[ MAX_HOOK_SOCK_NUM ];
    hook_epollfd = epoll_create( MAX_HOOK_SOCK_NUM );
    ASSERT( hook_epollfd != -1 );
    while( !HookMgr::is_stop ) {
        std::map<unsigned, POFD> sock_set;
        int number = epoll_wait( hook_epollfd, events, MAX_HOOK_SOCK_NUM, 1 );
        if (number <= 0) continue;
        //ASSERT( ( number < 0 ) && ( errno != EINTR ) );
        for ( int i = 0; i < number; i++ ) {
            unsigned ev_uid = events[i].data.u32;
            POFD pofd = wait_pf_map.Get(ev_uid);
            if (0 == pofd.cuid) {
                continue;
            }
            if( events[i].events & EPOLLHUP ) {
                pofd.revents |= POLLHUP;
            } else if( events[i].events & EPOLLERR ) {
                pofd.revents |= POLLERR;
            } else if( events[i].events & EPOLLIN ) {
                pofd.revents |= POLLIN;
            } else if( events[i].events & EPOLLOUT ) {
                pofd.revents |= POLLOUT;
            } else {
                printf("not support event\n");
            }
            if (wait_pf_map.Set(ev_uid, pofd) &&
                0 != pofd.revents) {
                sock_set[ev_uid] = pofd;
            }
        }

        std::map<unsigned, POFD>::iterator it = sock_set.begin();
        for(; it != sock_set.end(); ++it) {
            POFD pofd = it->second;
            if (pofd.revents == 0) {
                printf("notify ev is null\n");
                abort();
            }
            if (0 == pofd.cuid) {
                printf("notify croid is  -1\n");
                abort();
            }
            Closure<void>* clo = clo_map.Pop(pofd.cuid);
            if (clo) XcoreMgr::PutOutSideQueue(clo);
        }

    }
    return NULL;
}

void StartHookEpoll() {
    pthread_create(&HookMgr::epoll_thread,NULL,HookEpollLoop,NULL);
}

HookMgr::HookMgr() {
    HookMgr::is_stop = true;
}

void HookMgr::Start() {
    HookMgr::is_stop = false;
    StartHookEpoll();
}

void HookMgr::CloseEpoll() {
    if (!HookMgr::is_stop) {
        HookMgr::is_stop = true;
        pthread_join(HookMgr::epoll_thread, NULL);
    }
}

HookMgr::~HookMgr(){
    HookMgr::CloseEpoll();
}

static HookMgr hook_mgr;

// ÊöÇÊó∂‰∏Ä‰∏™ËøõÁ®ãÂ∞±ÊîØÊåÅ‰∏Ä‰∏™ÂçèÁ®ã
// ‰ª•ÂêéÂÜçËÄÉËôëÂ§öÁ∫øÁ®ãÔºå‰∏çËøáÊÑè‰πâ‰∏çÂ§ß
void SetCoroutineUsedByCurThread() {
    hook_timer_mgr.Start();    
    singleton->threadid = pthread_self();
    HookMgr::Start();
}

void CloseUseCoroutine() {
    HookMgr::CloseEpoll();
    hook_timer_mgr.Close();
}

// ‰ª•‰∏ãÊòØÈí©Â≠êÂÆûÁé∞

typedef int (*socket_pfn_t)(int domain, int type, int protocol);
typedef int (*connect_pfn_t)(int socket, const struct sockaddr *address, socklen_t address_len);
typedef int (*close_pfn_t)(int fd);

typedef ssize_t (*read_pfn_t)(int fildes, void *buf, size_t nbyte);
typedef ssize_t (*write_pfn_t)(int fildes, const void *buf, size_t nbyte);

typedef ssize_t (*sendto_pfn_t)(int socket, const void *message, size_t length,
	                 int flags, const struct sockaddr *dest_addr,
					               socklen_t dest_len);

typedef ssize_t (*recvfrom_pfn_t)(int socket, void *buffer, size_t length,
	                 int flags, struct sockaddr *address,
					               socklen_t *address_len);

typedef size_t (*send_pfn_t)(int socket, const void *buffer, size_t length, int flags);
typedef ssize_t (*recv_pfn_t)(int socket, void *buffer, size_t length, int flags);

typedef int (*poll_pfn_t)(struct pollfd fds[], nfds_t nfds, int timeout);
typedef int (*setsockopt_pfn_t)(int socket, int level, int option_name,
			                 const void *option_value, socklen_t option_len);

typedef int (*fcntl_pfn_t)(int fildes, int cmd, ...);
typedef struct tm *(*localtime_r_pfn_t)( const time_t *timep, struct tm *result );

typedef void *(*pthread_getspecific_pfn_t)(pthread_key_t key);
typedef int (*pthread_setspecific_pfn_t)(pthread_key_t key, const void *value);

typedef int (*pthread_rwlock_rdlock_pfn_t)(pthread_rwlock_t *rwlock);
typedef int (*pthread_rwlock_wrlock_pfn_t)(pthread_rwlock_t *rwlock);
typedef int (*pthread_rwlock_unlock_pfn_t)(pthread_rwlock_t *rwlock);

static socket_pfn_t g_sys_socket_func 	= (socket_pfn_t)dlsym(RTLD_NEXT,"socket");
static connect_pfn_t g_sys_connect_func = (connect_pfn_t)dlsym(RTLD_NEXT,"connect");
static close_pfn_t g_sys_close_func 	= (close_pfn_t)dlsym(RTLD_NEXT,"close");

static read_pfn_t g_sys_read_func 		= (read_pfn_t)dlsym(RTLD_NEXT,"read");
static write_pfn_t g_sys_write_func 	= (write_pfn_t)dlsym(RTLD_NEXT,"write");

static sendto_pfn_t g_sys_sendto_func 	= (sendto_pfn_t)dlsym(RTLD_NEXT,"sendto");
static recvfrom_pfn_t g_sys_recvfrom_func = (recvfrom_pfn_t)dlsym(RTLD_NEXT,"recvfrom");

static send_pfn_t g_sys_send_func 		= (send_pfn_t)dlsym(RTLD_NEXT,"send");
static recv_pfn_t g_sys_recv_func 		= (recv_pfn_t)dlsym(RTLD_NEXT,"recv");

static poll_pfn_t g_sys_poll_func 		= (poll_pfn_t)dlsym(RTLD_NEXT,"poll");

static setsockopt_pfn_t g_sys_setsockopt_func
										= (setsockopt_pfn_t)dlsym(RTLD_NEXT,"setsockopt");
static fcntl_pfn_t g_sys_fcntl_func 	= (fcntl_pfn_t)dlsym(RTLD_NEXT,"fcntl");

/*
static pthread_getspecific_pfn_t g_sys_pthread_getspecific_func
			= (pthread_getspecific_pfn_t)dlsym(RTLD_NEXT,"pthread_getspecific");

static pthread_setspecific_pfn_t g_sys_pthread_setspecific_func
			= (pthread_setspecific_pfn_t)dlsym(RTLD_NEXT,"pthread_setspecific");

static pthread_rwlock_rdlock_pfn_t g_sys_pthread_rwlock_rdlock_func
			= (pthread_rwlock_rdlock_pfn_t)dlsym(RTLD_NEXT,"pthread_rwlock_rdlock");

static pthread_rwlock_wrlock_pfn_t g_sys_pthread_rwlock_wrlock_func
			= (pthread_rwlock_wrlock_pfn_t)dlsym(RTLD_NEXT,"pthread_rwlock_wrlock");

static pthread_rwlock_unlock_pfn_t g_sys_pthread_rwlock_unlock_func
			= (pthread_rwlock_unlock_pfn_t)dlsym(RTLD_NEXT,"pthread_rwlock_unlock");
*/



static inline unsigned long long get_tick_count()
{
	uint32_t lo, hi;
	__asm__ __volatile__ (
			"rdtscp" : "=a"(lo), "=d"(hi)
			);
	return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

struct rpchook_connagent_head_t
{
    unsigned char    bVersion;
    struct in_addr   iIP;
    unsigned short   hPort;
    unsigned int     iBodyLen;
    unsigned int     iOssAttrID;
    unsigned char    bIsRespNotExist;
	unsigned char    sReserved[6];
}__attribute__((packed));


#define HOOK_SYS_FUNC(name) if( !g_sys_##name##_func ) { g_sys_##name##_func = (name##_pfn_t)dlsym(RTLD_NEXT,#name); }

static inline ll64_t diff_ms(struct timeval &begin,struct timeval &end)
{
	ll64_t u = (end.tv_sec - begin.tv_sec) ;
	u *= 1000 * 10;
	u += ( end.tv_usec - begin.tv_usec ) / (  100 );
	return u;
}



static inline rpchook_t * get_by_fd( int fd )
{
	if( fd > -1 && fd < (int)sizeof(g_rpchook_socket_fd) / (int)sizeof(g_rpchook_socket_fd[0]) )
	{
		return g_rpchook_socket_fd[ fd ];
	}
	return NULL;
}
static inline void free_by_fd( int fd )
{
	if( fd > -1 && fd < (int)sizeof(g_rpchook_socket_fd) / (int)sizeof(g_rpchook_socket_fd[0]) )
	{
		rpchook_t *lp = g_rpchook_socket_fd[ fd ];
		if( lp )
		{
			g_rpchook_socket_fd[ fd ] = NULL;
			free(lp);
		}
	}
	return;

}
static inline rpchook_t * alloc_by_fd( int fd )
{
    free_by_fd(fd);
	if( fd > -1 && fd < (int)sizeof(g_rpchook_socket_fd) / (int)sizeof(g_rpchook_socket_fd[0]) )
	{
		rpchook_t *lp = (rpchook_t*)calloc( 1,sizeof(rpchook_t) );
		lp->read_timeout.tv_sec = 30;
		lp->write_timeout.tv_sec = 30;
		g_rpchook_socket_fd[ fd ] = lp;
		return lp;
	}
	return NULL;
}
int socket(int domain, int type, int protocol)
{
	HOOK_SYS_FUNC( socket );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_socket_func( domain,type,protocol );
	}
	int fd = g_sys_socket_func(domain,type,protocol);
	if( fd < 0 )
	{
		return fd;
	}

	rpchook_t *lp = alloc_by_fd( fd );
	lp->domain = domain;

	fcntl( fd, F_SETFL, g_sys_fcntl_func(fd, F_GETFL,0 ) );

	return fd;
}

int co_accept( int fd, struct sockaddr *addr, socklen_t *len )
{
	int cli = accept( fd,addr,len );
	if( cli < 0 )
	{
		return cli;
	}
	//rpchook_t *lp = alloc_by_fd( cli );
	return cli;
}

int connect(int fd, const struct sockaddr *address, socklen_t address_len)
{
	HOOK_SYS_FUNC( connect );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_connect_func(fd,address,address_len);
	}

	int ret = g_sys_connect_func(fd,address,address_len);

	if( address_len == sizeof(sockaddr_un))
	{
		const struct sockaddr_un *p = (const struct sockaddr_un *)address;
		if( strstr( p->sun_path,"connagent_unix_domain_socket") ) ///tmp/connagent_unix_domain_socket
		{
		}
	}
	rpchook_t *lp = get_by_fd( fd );
	if( lp )
	{
		if( sizeof(lp->dest) >= address_len )
		{
			memcpy( &(lp->dest),address,(int)address_len );
		}
	}
	return ret;
}
int close(int fd)
{
	HOOK_SYS_FUNC( close );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_close_func( fd );
	}

	free_by_fd( fd );
	int ret = g_sys_close_func(fd);

	return ret;
}
ssize_t read( int fd, void *buf, size_t nbyte )
{
	HOOK_SYS_FUNC( read );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_read_func( fd,buf,nbyte );
	}
	rpchook_t *lp = get_by_fd( fd );

	if( !lp || ( O_NONBLOCK & lp->user_flag ) )
	{
		ssize_t ret = g_sys_read_func( fd,buf,nbyte );
		return ret;
	}
	int timeout = ( lp->read_timeout.tv_sec * 1000 )
				+ ( lp->read_timeout.tv_usec / 1000 );

	struct pollfd pf = { 0 };
	pf.fd = fd;
	pf.events = ( POLLIN | POLLERR | POLLHUP );

	int pollret = poll( &pf,1,timeout );

	ssize_t readret = g_sys_read_func( fd,(char*)buf ,nbyte );

	if( readret < 0 )
	{
		co_log_err("CO_ERR: read fd %d ret %ld errno %d poll ret %d timeout %d",
					fd,readret,errno,pollret,timeout);
	}

	return readret;

}
ssize_t write( int fd, const void *buf, size_t nbyte )
{
	HOOK_SYS_FUNC( write );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_write_func( fd,buf,nbyte );
	}
	rpchook_t *lp = get_by_fd( fd );

	if( !lp || ( O_NONBLOCK & lp->user_flag ) )
	{
		ssize_t ret = g_sys_write_func( fd,buf,nbyte );
		return ret;
	}
	size_t wrotelen = 0;
	int timeout = ( lp->write_timeout.tv_sec * 1000 )
				+ ( lp->write_timeout.tv_usec / 1000 );

	ssize_t writeret = g_sys_write_func( fd,(const char*)buf + wrotelen,nbyte - wrotelen );

	if( writeret > 0 )
	{
		wrotelen += writeret;
	}
	while( wrotelen < nbyte )
	{

		struct pollfd pf = { 0 };
		pf.fd = fd;
		pf.events = ( POLLOUT | POLLERR | POLLHUP );
		poll( &pf,1,timeout );

		writeret = g_sys_write_func( fd,(const char*)buf + wrotelen,nbyte - wrotelen );

		if( writeret <= 0 )
		{
			break;
		}
		wrotelen += writeret ;
	}
	return wrotelen;
}

ssize_t sendto(int socket, const void *message, size_t length,
	                 int flags, const struct sockaddr *dest_addr,
					               socklen_t dest_len)
{
	/*
		1.no enable sys call ? sys
		2.( !lp || lp is non block ) ? sys
		3.try
		4.wait
		5.try
	*/
	HOOK_SYS_FUNC( sendto );
	if( !co_is_enable_sys_hook() )
	{
		return g_sys_sendto_func( socket,message,length,flags,dest_addr,dest_len );
	}

	rpchook_t *lp = get_by_fd( socket );
	if( !lp || ( O_NONBLOCK & lp->user_flag ) )
	{
		return g_sys_sendto_func( socket,message,length,flags,dest_addr,dest_len );
	}

	ssize_t ret = g_sys_sendto_func( socket,message,length,flags,dest_addr,dest_len );
	if( ret < 0 && EAGAIN == errno )
	{
		int timeout = ( lp->write_timeout.tv_sec * 1000 )
					+ ( lp->write_timeout.tv_usec / 1000 );


		struct pollfd pf = { 0 };
		pf.fd = socket;
		pf.events = ( POLLOUT | POLLERR | POLLHUP );
		poll( &pf,1,timeout );

		ret = g_sys_sendto_func( socket,message,length,flags,dest_addr,dest_len );

	}
	return ret;
}

ssize_t recvfrom(int socket, void *buffer, size_t length,
	                 int flags, struct sockaddr *address,
					               socklen_t *address_len)
{
	HOOK_SYS_FUNC( recvfrom );
	if( !co_is_enable_sys_hook() )
	{
		return g_sys_recvfrom_func( socket,buffer,length,flags,address,address_len );
	}

	rpchook_t *lp = get_by_fd( socket );
	if( !lp || ( O_NONBLOCK & lp->user_flag ) )
	{
		return g_sys_recvfrom_func( socket,buffer,length,flags,address,address_len );
	}

	int timeout = ( lp->read_timeout.tv_sec * 1000 )
				+ ( lp->read_timeout.tv_usec / 1000 );


	struct pollfd pf = { 0 };
	pf.fd = socket;
	pf.events = ( POLLIN | POLLERR | POLLHUP );
	poll( &pf,1,timeout );

	ssize_t ret = g_sys_recvfrom_func( socket,buffer,length,flags,address,address_len );
	return ret;
}

ssize_t send(int socket, const void *buffer, size_t length, int flags)
{
	HOOK_SYS_FUNC( send );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_send_func( socket,buffer,length,flags );
	}
	rpchook_t *lp = get_by_fd( socket );

	if( !lp || ( O_NONBLOCK & lp->user_flag ) )
	{
		return g_sys_send_func( socket,buffer,length,flags );
	}
	size_t wrotelen = 0;
	int timeout = ( lp->write_timeout.tv_sec * 1000 )
				+ ( lp->write_timeout.tv_usec / 1000 );

	ssize_t writeret = g_sys_send_func( socket,buffer,length,flags );

	if( writeret > 0 )
	{
		wrotelen += writeret;
	}
	while( wrotelen < length )
	{

		struct pollfd pf = { 0 };
		pf.fd = socket;
		pf.events = ( POLLOUT | POLLERR | POLLHUP );
		poll( &pf,1,timeout );

		writeret = g_sys_send_func( socket,(const char*)buffer + wrotelen,length - wrotelen,flags );

		if( writeret <= 0 )
		{
			break;
		}
		wrotelen += writeret ;
	}

	return wrotelen;
}

ssize_t recv( int socket, void *buffer, size_t length, int flags )
{
	HOOK_SYS_FUNC( recv );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_recv_func( socket,buffer,length,flags );
	}
	rpchook_t *lp = get_by_fd( socket );

	if( !lp || ( O_NONBLOCK & lp->user_flag ) )
	{
		return g_sys_recv_func( socket,buffer,length,flags );
	}
	int timeout = ( lp->read_timeout.tv_sec * 1000 )
				+ ( lp->read_timeout.tv_usec / 1000 );

	struct pollfd pf = { 0 };
	pf.fd = socket;
	pf.events = ( POLLIN | POLLERR | POLLHUP );

	int pollret = poll( &pf,1,timeout );

	ssize_t readret = g_sys_recv_func( socket,buffer,length,flags );

	if( readret < 0 )
	{
		co_log_err("CO_ERR: read fd %d ret %ld errno %d poll ret %d timeout %d",
					socket,readret,errno,pollret,timeout);
	}

	return readret;

}

static XMutex cu_mutex;
static unsigned cuid = 0;

unsigned NextNewId(unsigned& cliflow) {
    unsigned new_id;
    cu_mutex.lock();
    cliflow = ((cliflow == 0 || cliflow == 0xffffffffUL ) ? 1 : cliflow+1);
    new_id = cliflow;
    cu_mutex.unlock();
    return new_id;
}

void TimeOut_Job(unsigned u_croid) {
    Closure<void>* clo = clo_map.Pop(u_croid);
    if (clo) XcoreMgr::PutOutSideQueue(clo);
}

int co_poll(struct pollfd fds[], nfds_t nfds, int timeout) {
    unsigned* id_arr = new unsigned[nfds];
    unsigned u_croid = NextNewId(cuid);
    for (nfds_t i = 0; i < nfds; ++i) {
        id_arr[i] = NextNewId(cuid);
    }
    CroMgr mgr = GetCroMgr();
    int croid = coroutine_running(mgr);
    if (croid == -1) {
        printf("can not be -1\n");
        abort();
    }
    Closure<void>* callback = NewClosure(coroutine_resume, mgr, croid);
    clo_map.Insert(u_croid, callback);
    // Â≠òÊîæ‰∫ã‰ª∂ÁªìÊûú,epoll_waitËøîÂõûÁöÑ‰∫ã‰ª∂‰ºöÂ≠òÂà∞ËøôÈáå
    for (nfds_t i = 0; i < nfds; ++i) {
        unsigned ev_uid = id_arr[i];
        POFD  pofd;
        pofd.cuid = u_croid;
        wait_pf_map.Insert(ev_uid, pofd);
    }
    // Ê≥®ÂÜåÁªôÂè¶Â§ñ‰∏Ä‰∏™Á∫øÁ®ãÁöÑepoll(timeout=-1)ÔºåÊú¨Á∫øÁ®ã‰∏çÁî®ËΩÆËØ¢
    std::map<int, unsigned> cuid_dic;
    for (nfds_t i = 0; i < nfds; ++i) {
        struct pollfd& sub_pollfd = fds[i];
        unsigned ev_uid = id_arr[i];
        unsigned epoll_evs = PollEvent2Epoll(sub_pollfd.events);
        EpollAdd(sub_pollfd.fd, ev_uid, epoll_evs);
    }
    // Ë∂ÖÊó∂Â§ÑÁêÜ
    unsigned timer_id = -1;
    if (timeout >= 0) {
        Closure<void>* timeout_job =
            NewPermanentClosure(TimeOut_Job, u_croid);
        timer_id = hook_timer_mgr.AddJob(timeout, timeout_job, 1);
    }
    coroutine_yield(mgr);
    if (timeout >= 0) {
        hook_timer_mgr.DelJob(timer_id);
    }
    for (nfds_t i = 0; i < nfds; ++i) {
        struct pollfd& sub_pollfd = fds[i];
        EpollDel(sub_pollfd.fd, PollEvent2Epoll(sub_pollfd.events));
    }
    // Êî∂Ââ≤‰∫ã‰ª∂ÊûúÂÆû‰∫Ü
    int ev_fd_num = 0;
    for (nfds_t i = 0; i < nfds; ++i) {
        struct pollfd& sub_pollfd = fds[i];
        unsigned ev_uid = id_arr[i];
        POFD pofd = wait_pf_map.Pop(ev_uid);
        if (0 != pofd.revents) {
            ++ev_fd_num;
            sub_pollfd.revents = EpollEvent2Poll(pofd.revents);
        }
    }
    //if (0 == ev_fd_num) {
    //    printf("poll time out\n");
    //}
    delete []id_arr;
    return ev_fd_num;
}

int poll(struct pollfd fds[], nfds_t nfds, int timeout)
{

    HOOK_SYS_FUNC( poll );

    if( !co_is_enable_sys_hook() )
    {
        return g_sys_poll_func( fds,nfds,timeout );
    }

    return co_poll( fds,nfds,timeout );
}

int setsockopt(int fd, int level, int option_name,
			                 const void *option_value, socklen_t option_len)
{
	HOOK_SYS_FUNC( setsockopt );

	if( !co_is_enable_sys_hook() )
	{
		return g_sys_setsockopt_func( fd,level,option_name,option_value,option_len );
	}
	rpchook_t *lp = get_by_fd( fd );

	if( lp && SOL_SOCKET == level )
	{
		struct timeval *val = (struct timeval*)option_value;
		if( SO_RCVTIMEO == option_name  )
		{
			memcpy( &lp->read_timeout,val,sizeof(*val) );
		}
		else if( SO_SNDTIMEO == option_name )
		{
			memcpy( &lp->write_timeout,val,sizeof(*val) );
		}
	}
	return g_sys_setsockopt_func( fd,level,option_name,option_value,option_len );
}


int fcntl(int fildes, int cmd, ...)
{
	HOOK_SYS_FUNC( fcntl );

	if( fildes < 0 )
	{
		return __LINE__;
	}

	va_list arg_list;
	va_start( arg_list,cmd );

	int ret = -1;
	rpchook_t *lp = get_by_fd( fildes );
	switch( cmd )
	{
		case F_DUPFD:
		{
			int param = va_arg(arg_list,int);
			ret = g_sys_fcntl_func( fildes,cmd,param );
			break;
		}
		case F_GETFD:
		{
			ret = g_sys_fcntl_func( fildes,cmd );
			break;
		}
		case F_SETFD:
		{
			int param = va_arg(arg_list,int);
			ret = g_sys_fcntl_func( fildes,cmd,param );
			break;
		}
		case F_GETFL:
		{
			ret = g_sys_fcntl_func( fildes,cmd );
			break;
		}
		case F_SETFL:
		{
			int param = va_arg(arg_list,int);
			int flag = param;
			if( co_is_enable_sys_hook() && lp )
			{
				flag |= O_NONBLOCK;
			}
			ret = g_sys_fcntl_func( fildes,cmd,flag );
			if( 0 == ret && lp )
			{
				lp->user_flag = param;
			}
			break;
		}
		case F_GETOWN:
		{
			ret = g_sys_fcntl_func( fildes,cmd );
			break;
		}
		case F_SETOWN:
		{
			int param = va_arg(arg_list,int);
			ret = g_sys_fcntl_func( fildes,cmd,param );
			break;
		}
		case F_GETLK:
		{
			struct flock *param = va_arg(arg_list,struct flock *);
			ret = g_sys_fcntl_func( fildes,cmd,param );
			break;
		}
		case F_SETLK:
		{
			struct flock *param = va_arg(arg_list,struct flock *);
			ret = g_sys_fcntl_func( fildes,cmd,param );
			break;
		}
		case F_SETLKW:
		{
			struct flock *param = va_arg(arg_list,struct flock *);
			ret = g_sys_fcntl_func( fildes,cmd,param );
			break;
		}
	}

	va_end( arg_list );

	return ret;
}

void co_enable_hook_sys()
{
    CroMgr cro_mgr = GetCroMgr();
    int id = coroutine_running(cro_mgr);
    ASSERT(id >= 0);
    struct coroutine * C = cro_mgr->co[id];
    C->enable_sys_hook = true;
}


// 2009-07-27
// xcore_atomic.cpp
// 


namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XAtomic32
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

XAtomic32::XAtomic32(int32 i)
	: m_counter(i)
{
	ASSERT(sizeof(int32) == sizeof(LONG));
}

XAtomic32::XAtomic32(const XAtomic32& from)
	: m_counter(from.m_counter)
{
	ASSERT(sizeof(int32) == sizeof(LONG));
}

XAtomic32::~XAtomic32()
{
	// empty
}

int32 XAtomic32::get_value() const
{
	return InterlockedCompareExchange((volatile LONG*)&m_counter, 0, 0);
}

int32 XAtomic32::set_value(int32 i)
{
	return InterlockedExchange((LONG*)&m_counter, i);
}

int32 XAtomic32::test_zero_inc()
{
	for ( ; ; )
	{
		LONG tmp = get_value();
		if (tmp <= 0) return tmp;
		if (InterlockedCompareExchange((LONG*)&m_counter, tmp + 1, tmp) == tmp) return tmp + 1;
	}
	return 0;
}

XAtomic32::operator int32()
{
	return get_value();
}

XAtomic32& XAtomic32::operator= (int32 i)
{
	this->set_value(i);
	return *this;
}

XAtomic32& XAtomic32::operator= (const XAtomic32& from)
{
	if (this != &from)
	{
		this->set_value(from.m_counter);
	}
	return *this;
}

int32 XAtomic32::operator+= (int32 i)
{
	return InterlockedExchangeAdd((LONG*)&m_counter, i) + i;
}

int32 XAtomic32::operator-= (int32 i)
{
	return InterlockedExchangeAdd((LONG*)&m_counter, -i) - i;
}

int32 XAtomic32::operator++ (int)
{
	return  InterlockedIncrement((LONG*)&m_counter) - 1;
}

int32 XAtomic32::operator-- (int)
{
	return InterlockedDecrement((LONG*)&m_counter) + 1;
}

int32 XAtomic32::operator++ ()
{
	return InterlockedIncrement((LONG*)&m_counter);
}

int32 XAtomic32::operator-- ()
{
	return InterlockedDecrement((LONG*)&m_counter);
}

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

// ‘⁄linux2.6.18÷Æ«∞£¨∂‡≤…”√“‘œ¬∑Ω∑® µœ÷‘≠◊”≤Ÿ◊˜£¨’‚÷÷◊ˆ∑®”ÎCPU¿‡–Õœ‡πÿ
//static int32 add_return(int32& counter, int32 i)
//{
//	/* Modern 486+ processor */
//	__asm__ __volatile__(
//		"lock ; xaddl %0, %1"
//		:"+r" (i), "+m" (counter)
//		: : "memory");
//	return i; // ∑µªÿ‘≠”–÷µ
//}
//
// ‘⁄linux2.6.18÷Æ∫Û£¨…æ≥˝¡À<asm/atomic.h>∫Õ<asm/bitops.h>Õ∑Œƒº˛£¨±‡“Î∆˜Ã·π©ƒ⁄Ω®(built-in)‘≠◊”≤Ÿ◊˜∫Ø ˝°£
// –Ë“™‘⁄gcc±‡“Î—°œÓ÷–÷∏√˜CPU¿‡–Õ°£»Ágcc -marth=i686 -o hello hello.c

XAtomic32::XAtomic32(int32 i)
	: m_counter(i)
{
	ASSERT(sizeof(int32) == sizeof(int32_t));
}

XAtomic32::XAtomic32(const XAtomic32& from)
	: m_counter(from.m_counter)
{
	ASSERT(sizeof(int32) == sizeof(int32_t));
}

XAtomic32::~XAtomic32()
{
	// empty
}

int32 XAtomic32::get_value() const
{
	return __sync_val_compare_and_swap((volatile int*)&m_counter, 0, 0);
}

int32 XAtomic32::set_value(int32 i)
{
	// set m_counter = i and return old value of m_counter
	return __sync_lock_test_and_set(&m_counter, i);
}

int32 XAtomic32::test_zero_inc()
{
	// if (m_counter != 0) ++m_counter; return m_counter;

	for( ; ; )
	{
		int32 tmp = get_value();
		if(tmp <= 0) return tmp;
		if (__sync_val_compare_and_swap(&m_counter, tmp, tmp + 1) == tmp) return tmp + 1;
	}
	return 0;
}

XAtomic32::operator int32()
{
	return get_value();
}

XAtomic32& XAtomic32::operator= (int32 i)
{
	this->set_value(i);
	return *this;
}

XAtomic32& XAtomic32::operator= (const XAtomic32& from)
{
	if (this != &from)
	{
		this->set_value(from.get_value());
	}
	return *this;
}

int32 XAtomic32::operator+= (int32 i)
{
	return __sync_add_and_fetch(&m_counter, i);
}

int32 XAtomic32::operator-= (int32 i)
{
	return __sync_sub_and_fetch(&m_counter, i);
}

int32 XAtomic32::operator++ (int)
{
	return __sync_fetch_and_add(&m_counter, 1);
}

int32 XAtomic32::operator-- (int)
{
	return __sync_fetch_and_sub(&m_counter, 1);
}

int32 XAtomic32::operator++ ()
{
	return __sync_add_and_fetch(&m_counter, 1);
}

int32 XAtomic32::operator-- ()
{
	return __sync_sub_and_fetch(&m_counter, 1);
}

#endif//__GNUC__


///////////////////////////////////////////////////////////////////////////////
// class XAtomic64
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

XAtomic64::XAtomic64(int64 i)
	: m_counter(0)
{
#if (_WIN32_WINNT >= 0x0502)
	m_counter = i;
#else
	*(volatile LONG*)&m_counter = (LONG)i;
#endif

	ASSERT(sizeof(int64) == sizeof(LONGLONG));
}

XAtomic64::XAtomic64(const XAtomic64& from)
	: m_counter(0)
{
#if (_WIN32_WINNT >= 0x0502)
	m_counter = from.m_counter;
#else
	*(volatile LONG*)&m_counter = (LONG)from.m_counter;
#endif

	ASSERT(sizeof(int64) == sizeof(LONGLONG));
}

XAtomic64::~XAtomic64()
{
	// empty
}

int64 XAtomic64::get_value() const
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedCompareExchange64((volatile LONGLONG*)&m_counter, 0, 0);
#else
	return (int64)InterlockedCompareExchange((volatile LONG*)&m_counter, 0, 0);
#endif
}

int64 XAtomic64::set_value(int64 i)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedExchange64((volatile LONGLONG*)&m_counter, i);
#else
	return (int64)InterlockedExchange((volatile LONG*)&m_counter, (LONG)i);
#endif
}

int64 XAtomic64::test_zero_inc()
{
#if (_WIN32_WINNT >= 0x0502)
	for ( ; ; )
	{
		LONGLONG tmp = get_value();
		if (tmp <= 0) return tmp;
		if (InterlockedCompareExchange64((volatile LONGLONG*)&m_counter, tmp + 1, tmp) == tmp) return tmp + 1;
	}
	return 0;
#else
	for ( ; ; )
	{
		LONG tmp = (LONG)get_value();
		if (tmp <= 0) return tmp;
		if (InterlockedCompareExchange((volatile LONG*)&m_counter, tmp + 1, tmp) == tmp) return (int64)tmp + 1;
	}
	return 0;
#endif
}

XAtomic64::operator int64()
{
	return get_value();
}

XAtomic64& XAtomic64::operator= (int64 i)
{
	this->set_value(i);
	return *this;
}

XAtomic64& XAtomic64::operator= (const XAtomic64& from)
{
	if (this != &from)
	{
		this->set_value(from.m_counter);
	}
	return *this;
}

int64 XAtomic64::operator+= (int64 i)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedExchangeAdd64((volatile LONGLONG*)&m_counter, i) + i;
#else
	return (int64)InterlockedExchangeAdd((volatile LONG*)&m_counter, i) + i;
#endif
}

int64 XAtomic64::operator-= (int64 i)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedExchangeAdd64((volatile LONGLONG*)&m_counter, -i) - i;
#else
	return (int64)InterlockedExchangeAdd((volatile LONG*)&m_counter, -i) - i;
#endif
}

int64 XAtomic64::operator++ (int)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedIncrement64((volatile LONGLONG*)&m_counter) - 1;
#else
	return (int64)InterlockedIncrement((volatile LONG*)&m_counter) - 1;
#endif
}

int64 XAtomic64::operator-- (int)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedDecrement64((volatile LONGLONG*)&m_counter) + 1;
#else
	return (int64)InterlockedDecrement((volatile LONG*)&m_counter) + 1;
#endif
}

int64 XAtomic64::operator++ ()
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedIncrement64((volatile LONGLONG*)&m_counter);
#else
	return (int64)InterlockedIncrement((volatile LONG*)&m_counter);
#endif
}

int64 XAtomic64::operator-- ()
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedDecrement64((volatile LONGLONG*)&m_counter);
#else
	return (int64)InterlockedDecrement((volatile LONG*)&m_counter);
#endif
}

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

XAtomic64::XAtomic64(int64 i)
	: m_counter(i)
{
	ASSERT(sizeof(int64) == sizeof(int64_t));
}

XAtomic64::XAtomic64(const XAtomic64& from)
	: m_counter(from.m_counter)
{
	ASSERT(sizeof(int64) == sizeof(int64_t));
}

XAtomic64::~XAtomic64()
{
	// empty
}

int64 XAtomic64::get_value() const
{
	return __sync_val_compare_and_swap((volatile int64*)&m_counter, 0, 0);
}

int64 XAtomic64::set_value(int64 i)
{
	// set m_counter = i and return old value of m_counter
	return __sync_lock_test_and_set(&m_counter, i);
}

int64 XAtomic64::test_zero_inc()
{
	// if (m_counter != 0) ++m_counter; return m_counter;

	for( ; ; )
	{
		int64 tmp = get_value();
		if(tmp <= 0) return tmp;
		if (__sync_val_compare_and_swap(&m_counter, tmp, tmp + 1) == tmp) return tmp + 1;
	}
	return 0;
}

XAtomic64::operator int64()
{
	return get_value();
}

XAtomic64& XAtomic64::operator= (int64 i)
{
	this->set_value(i);
	return *this;
}

XAtomic64& XAtomic64::operator= (const XAtomic64& from)
{
	if (this != &from)
	{
		this->set_value(from.get_value());
	}
	return *this;
}

int64 XAtomic64::operator+= (int64 i)
{
	return __sync_add_and_fetch(&m_counter, i);
}

int64 XAtomic64::operator-= (int64 i)
{
	return __sync_sub_and_fetch(&m_counter, i);
}

int64 XAtomic64::operator++ (int)
{
	return __sync_fetch_and_add (&m_counter, 1);
}

int64 XAtomic64::operator-- (int)
{
	return __sync_fetch_and_sub(&m_counter, 1);
}

int64 XAtomic64::operator++ ()
{
	return __sync_add_and_fetch(&m_counter, 1);
}

int64 XAtomic64::operator-- ()
{
	return __sync_sub_and_fetch(&m_counter, 1);
}

#endif//__GNUC__

}//namespace xcore

////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

// 2011-07-13
// xcore_clock.cpp
//


#include <string.h>
#include <new>

using std::nothrow;

//#pragma comment(lib, "winmm.lib")

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XClock
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

static uint32 __getNumberOfProcessors()
{
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	return SysInfo.dwNumberOfProcessors;
}

static LARGE_INTEGER XClock_frequency;
static BOOL XClock_result = QueryPerformanceFrequency(&XClock_frequency);
static uint32 XClock_numberOfProcessors = __getNumberOfProcessors();

// ’‚Ã◊ª˙÷∆‘⁄∂‡∫ÀÃıº˛œ¬ª·≥ˆŒ Ã‚£¨ø…≤ŒøºSetThreadAffinityMas,timeGetTime¡Ω∏ˆ∫Ø ˝
class XClock::XClockImpl
{
public:
	XClockImpl()
	{
		VERIFY(QueryPerformanceCounter(&m_start));
		memcpy(&m_last, &m_start, sizeof(m_last));
		m_start2 = (uint64)timeGetTime();
		m_last2 = m_start2;
	}

	XClockImpl(const XClockImpl& from)
	{
		memcpy(&m_start, &from.m_start, sizeof(m_start));
		memcpy(&m_last, &from.m_last, sizeof(m_last));
		m_start2 = from.m_start2;
		m_last2 = from.m_last2;
	}

	XClockImpl& operator= (const XClockImpl& from)
	{
		if (this != &from)
		{
			memcpy(&m_start, &from.m_start, sizeof(m_start));
			memcpy(&m_last, &from.m_last, sizeof(m_last));
			m_start2 = from.m_start2;
			m_last2 = from.m_last2;
		}
		return *this;
	}

	~XClockImpl()
	{
		// empty
	}

	void reset()
	{
		if (XClock_numberOfProcessors <= 1)
		{
			VERIFY(QueryPerformanceCounter(&m_start));
			memcpy(&m_last, &m_start, sizeof(m_last));
		}
		else
		{
			m_start2 = (uint64)timeGetTime();
			m_last2 = m_start2;
		}
	}

	XTimeSpan peek()
	{
		if (XClock_numberOfProcessors <= 1)
		{
			ASSERT(XClock_result && (XClock_frequency.QuadPart > 0));

			LARGE_INTEGER now;
			VERIFY(QueryPerformanceCounter(&now));
			ASSERT(now.QuadPart >= m_start.QuadPart);

			int64 usec = (int64)((now.QuadPart - m_start.QuadPart) * 1000000 / XClock_frequency.QuadPart);
			return XTimeSpan(usec / 1000000, usec % 1000000);
		}
		else
		{
			uint64 now = (uint64)timeGetTime();
			if (now < m_start2) now = m_start2 + 0X100000000; // timeGetTimeº∆ ±“Á≥ˆ¡À

			uint64 msec = now - m_start2;
			return XTimeSpan(msec / 1000, (msec % 1000) * 1000);
		}
	}

	XTimeSpan diff_last()
	{
		if (XClock_numberOfProcessors <= 1)
		{
			ASSERT(XClock_result && (XClock_frequency.QuadPart > 0));

			LARGE_INTEGER now;
			VERIFY(QueryPerformanceCounter(&now));
			ASSERT(now.QuadPart >= m_last.QuadPart);

			int64 usec = (int64)((now.QuadPart - m_last.QuadPart) * 1000000 / XClock_frequency.QuadPart);
			memcpy(&m_last, &now, sizeof(now));
			return XTimeSpan(usec / 1000000, usec % 1000000);
		}
		else
		{
			uint64 now = (uint64)timeGetTime();
			if (now < m_start2) now = m_start2 + 0X100000000; // timeGetTimeº∆ ±“Á≥ˆ¡À

			uint64 msec = now - m_last2;
			m_last2 = now;
			return XTimeSpan(msec / 1000, (msec % 1000) * 1000);
		}
	}

private:
	LARGE_INTEGER  m_start;
	LARGE_INTEGER  m_last;
	uint64 m_start2;
	uint64 m_last2;
};

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

class XClock::XClockImpl
{
public:
	XClockImpl()
	{
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &m_start));
		memcpy(&m_last, &m_start, sizeof(m_last));
	}

	XClockImpl(const XClockImpl& from)
	{
        ASSERT(0x0 != &from.m_start);
		memcpy(&m_start, &from.m_start, sizeof(m_start));
		memcpy(&m_last, &from.m_last, sizeof(m_last));
	}

	XClockImpl& operator= (const XClockImpl& from)
	{
		if (this != &from)
		{
            ASSERT(0x0 != &from.m_start);
			memcpy(&m_start, &from.m_start, sizeof(m_start));
			memcpy(&m_last, &from.m_last, sizeof(m_last));
		}
		return *this;
	}

	~XClockImpl()
	{
		// empty
	}

	void reset()
	{
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &m_start));
		memcpy(&m_last, &m_start, sizeof(m_last));
	}

	XTimeSpan peek()
	{
		timespec now;
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &now));
		ASSERT(now.tv_sec >= m_start.tv_sec);

		int64 nsec = now.tv_sec - m_start.tv_sec;
		nsec = nsec * 1000000000 + now.tv_nsec - m_start.tv_nsec;
		return XTimeSpan(nsec / 1000000000, (nsec % 1000000000) / 1000);
	}

	XTimeSpan diff_last()
	{
		timespec now;
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &now));
		ASSERT(now.tv_sec >= m_last.tv_sec);

		int64 nsec = now.tv_sec - m_last.tv_sec;
		nsec = nsec * 1000000000 + now.tv_nsec - m_last.tv_nsec;
		memcpy(&m_last, &now, sizeof(now));
		return XTimeSpan(nsec / 1000000000, (nsec % 1000000000) / 1000);
	}

private:
	timespec  m_start;
	timespec  m_last;
};

#endif//__GNUC__

///////////////////////////////////////////////////////////////////////////////
XClock::XClock()
	: m_impl(new(nothrow) XClockImpl)
{
	// empty
}

XClock::XClock(const XClock& from)
	: m_impl(new(nothrow) XClockImpl(*from.m_impl))
{
	// empty
}

XClock& XClock::operator=(const XClock& from)
{
	if (this != &from)
	{
		*m_impl = *from.m_impl;
	}
	return *this;
}

XClock::~XClock()
{
}

void XClock::reset()
{
	m_impl->reset();
}

XTimeSpan XClock::peek()
{
	return m_impl->peek();
}

XTimeSpan XClock::diff_last()
{
	return m_impl->diff_last();
}

///////////////////////////////////////////////////////////////////////////////
static XMutex cmut;
XClock* static_clock = NULL;

XTimeSpan running_time(void)
{
    if (NULL == static_clock) {
        usleep(1000);
        cmut.lock();
        if (NULL == static_clock) static_clock = new XClock();
        cmut.unlock();
    }
    return static_clock->peek();
}

} // namespace xcore


// 2008-02-23
// xcore_critical.cpp
// 

#include <new>

using std::nothrow;

namespace xcore {

XCritical XCritical::StaticCritical;

///////////////////////////////////////////////////////////////////////////////
// class XCritical
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

class XCritical::XCriticalImpl
{
public:
	XCriticalImpl()
	{
		InitializeCriticalSection(&m_critical);
	}

	~XCriticalImpl()
	{
		DeleteCriticalSection(&m_critical);
	}

	void lock()
	{
		EnterCriticalSection(&m_critical);
	}

	bool trylock()
	{
		return !!TryEnterCriticalSection(&m_critical);
	}

	void unlock()
	{
		LeaveCriticalSection(&m_critical);
	}

private:
	CRITICAL_SECTION  m_critical;
};

#endif //__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

class XCritical::XCriticalImpl
{
public:
	XCriticalImpl()
	{
		pthread_mutexattr_t attr;
		VERIFY(!pthread_mutexattr_init(&attr));
		VERIFY(!pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)); // PTHREAD_MUTEX_RECURSIVE_NP
		VERIFY(!pthread_mutex_init(&m_mutex, &attr));
		VERIFY(!pthread_mutexattr_destroy(&attr));
	}

	~XCriticalImpl()
	{
		int ret = pthread_mutex_destroy(&m_mutex);
		ASSERT(!ret && "the mutex is currently locked.");
		ret = 0;
	}

	void lock()
	{
		int ret = pthread_mutex_lock(&m_mutex);
		ASSERT(!ret && "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
		ret = 0;
	}

	bool trylock()
	{
		int ret = pthread_mutex_trylock(&m_mutex);
		if (0 == ret) return true;
		if (errno == EAGAIN) return false;
		ASSERT(!"The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
		ret = 0;
        return true;
	}

	void unlock()
	{
		int ret = pthread_mutex_unlock(&m_mutex);
		ASSERT(!ret && "The current thread does not own the mutex.");
		ret = 0;
	}

private:
	pthread_mutex_t  m_mutex;
};

#endif //__GNUC__

///////////////////////////////////////////////////////////////////////////////
XCritical::XCritical()
	: m_impl(new(nothrow) XCriticalImpl)
{
	// empty
}

XCritical::~XCritical()
{
}

void XCritical::lock()
{
	m_impl->lock();
}

bool XCritical::trylock()
{
	return m_impl->trylock();
}

void XCritical::unlock()
{
	m_impl->unlock();
}

} // namespace xcore


// 2008-02-23
// xcore_event.cpp
// 


namespace xcore {

XEvent XEvent::AutoStaticEvent;
XEvent XEvent::ManualStaticEvent(true);

///////////////////////////////////////////////////////////////////////////////
// class XEvent
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

class XEvent::XEventImpl
{
public:
	XEventImpl(bool manual)
		: m_manual(manual)
	{
		VERIFY(m_handle = (void *)CreateEvent(NULL, (BOOL)m_manual, FALSE, NULL));
	}

	~XEventImpl()
	{
		ASSERT(m_handle);
	 	VERIFY(CloseHandle(m_handle));
		m_handle = NULL;
	}

	void set()
	{
		ASSERT(m_handle);
		VERIFY(SetEvent(m_handle));
	}

	void reset()
	{
		ASSERT(m_handle);
		ASSERT(m_manual && "this is a auto event, can't manual reset.");
		VERIFY(ResetEvent(m_handle));
	}

	void wait()
	{
		ASSERT(m_handle);
		VERIFY(WAIT_OBJECT_0 == WaitForSingleObject(m_handle, INFINITE));
	}

	bool trywait(int32 msec)
	{
		if (msec < 0)
		{
			wait();
			return true;
		}

		ASSERT(m_handle);
		return (WAIT_OBJECT_0 == WaitForSingleObject(m_handle, msec));
	}

private:
	HANDLE  m_handle;
	bool    m_manual;  // manual or auto reset
};

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

class XEvent::XEventImpl
{
public:
	XEventImpl(bool manual)
		: m_flag(false)
		, m_manual(manual)
	{
		VERIFY(!pthread_mutex_init(&m_mutex, NULL));  // PTHREAD_MUTEX_FAST_NP
		VERIFY(!pthread_cond_init(&m_cond, NULL));
	}

	~XEventImpl()
	{
		int ret = pthread_cond_destroy(&m_cond);
		ASSERT(!ret && "some threads are currently waiting on 'cond'.");
		ret = pthread_mutex_destroy(&m_mutex);
		ASSERT(!ret && "the mutex is currently locked.");
	}

	void set()
	{
		VERIFY(!pthread_mutex_lock(&m_mutex));
		m_flag = true;
		if (m_manual)
			VERIFY(!pthread_cond_broadcast(&m_cond));
		else
			VERIFY(!pthread_cond_signal(&m_cond));
		VERIFY(!pthread_mutex_unlock(&m_mutex));
	}

	void reset()
	{
		ASSERT(m_manual && "this is a auto event, can't manual reset.");
		VERIFY(!pthread_mutex_lock(&m_mutex));
		m_flag = false;
		VERIFY(!pthread_mutex_unlock(&m_mutex));
	}

	void wait()
	{
		do 
		{
			VERIFY(!pthread_mutex_lock(&m_mutex));
			if (!m_flag)
			{
				VERIFY(!pthread_cond_wait(&m_cond, &m_mutex));
			}
			if (m_flag)
			{
				if (!m_manual) m_flag = false;
				VERIFY(!pthread_mutex_unlock(&m_mutex));
				break;
			}
			else
			{
				VERIFY(!pthread_mutex_unlock(&m_mutex));
				continue;
			}
		} while (true);
	}

	bool trywait(int32 msec)
	{
		if (msec < 0)
		{
			wait();
			return true;
		}

		bool flag = false;
		VERIFY(!pthread_mutex_lock(&m_mutex));
		flag = m_flag;
		if (!flag && msec > 0)
		{
			struct timeval tv = {};
			gettimeofday(&tv, NULL);

			tv.tv_sec  += msec / 1000;
			tv.tv_usec += (msec % 1000 * 1000);
			tv.tv_sec  += tv.tv_usec / 1000000;
			tv.tv_usec %= 1000000;

			struct timespec abstime = {};
			abstime.tv_sec  = tv.tv_sec;
			abstime.tv_nsec = tv.tv_usec * 1000;

			int ret = pthread_cond_timedwait(&m_cond, &m_mutex, &abstime);
			ASSERT(ret == 0 || ret == ETIMEDOUT);
            ret = 0;
			flag = m_flag;
		}
		if (!m_manual) m_flag = false;
		VERIFY(!pthread_mutex_unlock(&m_mutex));

		return flag;
	}

private:
	pthread_cond_t	 m_cond;
	pthread_mutex_t	 m_mutex;
	volatile bool    m_flag;
	bool             m_manual;  // manual or auto reset
};

#endif//__GNUC__

///////////////////////////////////////////////////////////////////////////////

XEvent::XEvent(bool manual)
	: m_impl(new(std::nothrow) XEventImpl(manual))
{
	// empty
}

XEvent::~XEvent()
{
}

void XEvent::set()
{
	return m_impl->set();
}

void XEvent::reset()
{
	return m_impl->reset();
}

void XEvent::wait()
{
	return m_impl->wait();
}

bool XEvent::trywait(int32 msec)
{
	return m_impl->trywait(msec);
}

} // namespace xcore


// 2008-02-29
// xcore_memory.cpp
// 

#include <new>
#include <string.h>

using std::nothrow;


namespace xcore {

#define MEMORY_BLOCK_MARK *((unsigned int*)"GNIQ")
#define MEMORY_BUFFER_RESERVE_SIZE   8

static uint32 g_uRose[] = 
{
	64,
	1   * 8 + 64,
	2   * 8 + 64,
	4   * 8 + 64,
	8   * 8 + 64,
	16  * 8 + 64,
	32  * 8 + 64,
	64  * 8 + 64,
	128 * 8 + 64, /*1  * 1024*/
	256 * 8 + 64, /*2  * 1024*/
	512 * 8 + 64, /*4  * 1024*/
	1024* 8 + 64, /*8  * 1024*/
	2048* 8 + 64, /*16 * 1024*/
	4096* 8 + 64, /*32 * 1024*/
	8192* 8 + 64  /*64 * 1024*/
};

struct XMemoryBlock;
struct XBlockLinkHead;

#pragma pack(1)
struct XBlockLinkHead
{
	XMemoryBlock *		m_pFreeLink;			// ◊‘”…øÈ¡¥
	uint32				m_nFreeCount;			// ◊‘”…øÈ ˝
	XCritical           m_lock;                 // À¯
};

struct XMemoryBlock
{
	XMemoryBlock *			m_pNext;			// øÈ¡¥
	XBlockLinkHead *		m_pHead;			// ¡¥Õ∑
	#ifdef __DEBUG__
	uint32					m_mark;				// ºÏ—È◊÷
	#endif//__DEBUG__
	char					m_data[0];			// ”––ß ˝æ›
};
#pragma pack()

////////////////////////////////////////////////////////////////////////////////
// XMemory
////////////////////////////////////////////////////////////////////////////////
class XMemory
{
public:
	XMemory()
	{
		for (size_t i = 0; i < COUNT_OF_ARRAY(m_MainLink); i++)
		{
			m_MainLink[i].m_pFreeLink = NULL;
			m_MainLink[i].m_nFreeCount = 0;
		}
	}

	~XMemory()
	{
		XMemoryBlock *pBlock = NULL;
		XMemoryBlock *pBlock2 = NULL;

		for (size_t i = 0; i < COUNT_OF_ARRAY(g_uRose); i++)
		{
			XLockGuard<XCritical> lock_(m_MainLink[i].m_lock);
			pBlock = m_MainLink[i].m_pFreeLink;
			while (pBlock)
			{
				pBlock2 = pBlock;
				pBlock = pBlock->m_pNext;
				delete[] pBlock2;
			}
		}
	}

	void* alloc(uint32 uSize)
	{
		XBlockLinkHead* pHead = NULL;
		XMemoryBlock *pBlock = NULL;
		uSize += sizeof(XMemoryBlock);

		uint32 count = COUNT_OF_ARRAY(g_uRose);
		if (uSize > g_uRose[count - 1])
		{
			// ≥¨¥ÛøÈ”…≤Ÿ◊˜œµÕ≥∑÷≈‰
			pBlock = (XMemoryBlock *)::new(nothrow) uint8[uSize];
		}
		else
		{
			// ≤È’“∫œ  µƒøÈ¡¥
			uint32 index = 0;
			for (index = 0; index < count; index++)
			{
				if (uSize <= g_uRose[index]) break;
			}
			pHead = &m_MainLink[index];

			XLockGuard<XCritical> lock(pHead->m_lock);
			if (pHead->m_pFreeLink == NULL)
			{
				pBlock = (XMemoryBlock *)new(nothrow) uint8[g_uRose[index]];
			}
			else
			{
				pBlock = pHead->m_pFreeLink;
				pHead->m_pFreeLink = pBlock->m_pNext;
				if (pHead->m_nFreeCount > 0) pHead->m_nFreeCount--;
			}
		}

		ASSERT(pBlock);
		ASSERT((pBlock->m_mark = MEMORY_BLOCK_MARK, true));
		pBlock->m_pHead = pHead;
		pBlock->m_pNext = NULL;

		return (void *)pBlock->m_data;
	}

	void free(void *pData)
	{
		if (pData == NULL) return;
		ASSERT(((uint32)((char *)pData - (char *)NULL) > (uint32)sizeof(XMemoryBlock)) && "Invalid Memory Block!");
		XMemoryBlock* pBlock = (XMemoryBlock *)((char *)pData - sizeof(XMemoryBlock));
		ASSERT((pBlock->m_mark == MEMORY_BLOCK_MARK) && "Corrupt Memory Block!");

		XBlockLinkHead* pHead = pBlock->m_pHead;
		if ((pHead == NULL) || (pHead->m_nFreeCount > 10000))
		{
			// 1. ≤Ÿ◊˜œµÕ≥π‹¿ÌµƒøÈ
			// 2. ◊‘”…¡¥…œµƒ◊‘”…øÈπ˝∂‡
			delete[] pBlock;
		}
		else
		{
			XLockGuard<XCritical> lock_(pHead->m_lock);
			pBlock->m_pNext = (XMemoryBlock *)pHead->m_pFreeLink;
			pHead->m_pFreeLink = pBlock;
			pHead->m_nFreeCount++;
		}
		return;
	}

private:
	XBlockLinkHead m_MainLink[COUNT_OF_ARRAY(g_uRose)];
};

static XMemory g_Memory_instance;

////////////////////////////////////////////////////////////////////////////////
// class XBuffer
////////////////////////////////////////////////////////////////////////////////
class XBuffer : public IXBuffer
{
public:
	struct XBuffer_Deleter
	{
		void operator()(XBuffer* pBuffer) const
		{
			if (pBuffer == NULL) return;
			pBuffer->~XBuffer();
			g_Memory_instance.free((void *)pBuffer);
		}
	};

public:
	XBuffer(uint32 uSize, void *pvData)
	  : m_orignSize(uSize)
	  , m_orignData((char*)pvData)
	  , m_size(uSize)
	  , m_pData((char*)pvData)
	{
		ASSERT(pvData);
	}

	virtual uint32 size() const
	{
		return m_size;
	}

	virtual void* data() const
	{
		return m_pData;
	}

	virtual void resize(uint32 newSize)
	{
		ASSERT(newSize <= capacity());
		m_size = newSize;
	}

	virtual uint32 size_pre_reserve() const
	{
		return m_pData - (m_orignData - MEMORY_BUFFER_RESERVE_SIZE);
	}

	virtual void resize_pre_reserve(uint32 newSize)
	{
		uint32 reserveSize = size_pre_reserve();
		ASSERT(newSize <= reserveSize + m_size);
		m_pData = (char*)m_orignData - MEMORY_BUFFER_RESERVE_SIZE + newSize;
		m_size = reserveSize + m_size - newSize;
	}

	virtual uint32 capacity() const
	{
		return (m_orignData + m_orignSize) - m_pData;
	}

	virtual void restore()
	{
		m_size = m_orignSize;
		m_pData = (char *)m_orignData;
	}

	virtual shared_ptr<IXBuffer> clone()
	{
		shared_ptr<XBuffer> ptrBuffer = dynamic_pointer_cast<XBuffer, IXBuffer>(create_buffer(m_orignSize));
		ASSERT(ptrBuffer.get() != NULL);
		ptrBuffer->resize_pre_reserve(this->size_pre_reserve());
		ptrBuffer->resize(this->size());
		memcpy((char *)ptrBuffer->m_orignData - MEMORY_BUFFER_RESERVE_SIZE,
			   m_orignData - MEMORY_BUFFER_RESERVE_SIZE,
			   m_orignSize + MEMORY_BUFFER_RESERVE_SIZE);
		return shared_ptr<IXBuffer>(ptrBuffer);
	}

private:
	const uint32	m_orignSize;
	const char*     m_orignData;
	uint32			m_size;
	char*           m_pData;
};

///////////////////////////////////////////////////////////////////////
// function create_buffer
///////////////////////////////////////////////////////////////////////
shared_ptr<IXBuffer> create_buffer(uint32 uSize)
{
	void* pvBuffer = g_Memory_instance.alloc(uSize + sizeof(XBuffer) + MEMORY_BUFFER_RESERVE_SIZE);
	ASSERT(pvBuffer);

	char* pvData = (char *)pvBuffer + sizeof(XBuffer) + MEMORY_BUFFER_RESERVE_SIZE;
	XBuffer* pBuffer = new(pvBuffer)XBuffer(uSize, pvData);  // placement operator new
	ASSERT(pBuffer);

	return shared_ptr<IXBuffer>(pBuffer, XBuffer::XBuffer_Deleter());
}

void* memory_alloc(uint32 uSize)
{
	return g_Memory_instance.alloc(uSize);
}

void memory_free(void* pData)
{
	return g_Memory_instance.free(pData);
}

} // namespace xcore


// 2008-02-23
// xcore_mutex.cpp
// 


#include <new>

using std::nothrow;

namespace xcore {

XMutex XMutex::StaticMutex;

///////////////////////////////////////////////////////////////////////////////
// class XMutex
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

class XMutex::XMutexImpl
{
public:
	XMutexImpl()
	{
		//ASSERT((m_owner = 0, true));
		VERIFY(m_handle = CreateMutex(NULL, FALSE, NULL));
		m_owner = 0;
	}

	~XMutexImpl()
	{
		ASSERT(m_handle);
		if (m_owner) unlock();
		VERIFY(CloseHandle(m_handle));
		m_handle = NULL;
		//ASSERT(m_owner == 0 && "the mutex is currently locked.");
		//ASSERT((m_owner = 0, true));
	}

	void lock()
	{
		ASSERT(m_handle);
		VERIFY(WAIT_OBJECT_0 == WaitForSingleObject(m_handle, INFINITE));
		m_owner = 1;
		//ASSERT(m_owner == 0 && "the mutex is already locked by the calling thread.");
		//ASSERT((m_owner = xcore::thread_id(), true));
	}

	bool trylock()
	{
		ASSERT(m_handle);
		if(WAIT_OBJECT_0 != WaitForSingleObject(m_handle, 0))
		{
			return false;
		}
		else
		{
			//ASSERT(m_owner == 0 && "the mutex is already locked by the calling thread.");
			//ASSERT((m_owner = xcore::thread_id(), true));
			return true;
		}
	}

	void unlock()
	{
		ASSERT(m_handle);
		VERIFY(ReleaseMutex(m_handle));
		m_owner = 0;
		//ASSERT(m_owner == xcore::thread_id() && "the calling thread does not own the mutex.");
		//ASSERT((m_owner = 0, true));
	}

private:
	HANDLE   m_handle;
	#ifdef __WINDOWS__
	uint32   m_owner;
	#endif//__WINDOWS__
};

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

class XMutex::XMutexImpl
{
public:
	XMutexImpl()
	{
		//pthread_mutexattr_t attr;
		//VERIFY(!pthread_mutexattr_init(&attr));
		//VERIFY(!pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK));  // PTHREAD_MUTEX_ERRORCHECK_NP
		//VERIFY(!pthread_mutex_init(&m_mutex, &attr));
		//VERIFY(!pthread_mutexattr_destroy(&attr));
        pthread_mutex_init(&m_mutex,NULL);
	}

	~XMutexImpl()
	{
		int ret = pthread_mutex_destroy(&m_mutex);
		ASSERT(!ret && "the mutex is currently locked.");
		ret = 0;
	}

	void lock()
	{
		int ret = pthread_mutex_lock(&m_mutex);
		ASSERT(!ret && "the mutex is already locked by the calling thread.");
		ret = 0;
	}

	bool trylock()
	{
		return (0 == pthread_mutex_trylock(&m_mutex));
	}

	void unlock()
	{
		int ret = pthread_mutex_unlock(&m_mutex);
		ASSERT(!ret && "the calling thread does not own the mutex.");
		ret = 0;
	}

private:
	pthread_mutex_t  m_mutex;
};

#endif//__GNUC__

///////////////////////////////////////////////////////////////////////////////

XMutex::XMutex()
	: m_impl(new(nothrow) XMutexImpl)
{
	// empty
}

XMutex::~XMutex()
{
	delete m_impl;
}

void XMutex::lock()
{
	m_impl->lock();
}

bool XMutex::trylock()
{
	return m_impl->trylock();
}

void XMutex::unlock()
{
	m_impl->unlock();
}

} // namespace xcore


// 2008-02-27
//

#include <new>

using std::nothrow;

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XSemaphore
////////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

class XSemaphore::XSemaphoreImpl
{
public:
	XSemaphoreImpl(uint32 init_count)
	{
		VERIFY(m_handle = CreateSemaphore(NULL, init_count, 0X7FFFFFFF, NULL));
	}

	~XSemaphoreImpl()
	{
		ASSERT(m_handle);
		VERIFY(CloseHandle(m_handle));
		m_handle = NULL;
	}

	void wait()
	{
		ASSERT(m_handle);
		VERIFY(WAIT_OBJECT_0 == WaitForSingleObject(m_handle, INFINITE));
	}

	bool trywait(uint32 msec)
	{
		ASSERT(m_handle);
		return (WAIT_OBJECT_0 == WaitForSingleObject(m_handle, msec));
	}

	bool post(uint32 count)
	{
		long old_count = 0;
		ASSERT(m_handle);
		return !!ReleaseSemaphore(m_handle, count, (LPLONG)&old_count);
	}

	uint32 get_value()
	{
		long old_count = 0;
		ASSERT(m_handle);
		VERIFY(ReleaseSemaphore(m_handle, 0, (LPLONG)&old_count));
		return (uint32)old_count;
	}

private:
	HANDLE  m_handle;
};

#endif//__WINDOWS__

////////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

class XSemaphore::XSemaphoreImpl
{
public:
	XSemaphoreImpl(uint32 init_count)
	{
		if (init_count > SEM_VALUE_MAX)
		{
			init_count = SEM_VALUE_MAX;
		}
		VERIFY(!sem_init(&m_sem, 0, init_count));
	}

	~XSemaphoreImpl()
	{
		int ret = sem_destroy(&m_sem);
		ASSERT(!ret && "There are currently processes blocked on the semaphore.");
		ret = 0;
	}

	void wait()
	{
		int ret = 0;
		do
		{
			ret = sem_wait(&m_sem);
		} while ((0 != ret) && (errno == EINTR));
		ASSERT(!ret && "sem_wait() error.");
	}

	bool trywait(uint32 msec)
	{
		if (msec >= 0)
		//{
		//	return (0 == sem_trywait(&m_sem));
		//}
		//else
		{
			struct timeval tv = {};
			gettimeofday(&tv, NULL);

			tv.tv_sec  += msec / 1000;
			tv.tv_usec += (msec % 1000 * 1000);
			tv.tv_sec  += tv.tv_usec / 1000000;
			tv.tv_usec %= 1000000;

			struct timespec abstime = {};
			abstime.tv_sec  = tv.tv_sec;
			abstime.tv_nsec = tv.tv_usec * 1000;

			int ret = 0;
			do
			{
				ret = sem_timedwait(&m_sem, &abstime);
			} while ((0 != ret) && (errno == EINTR));

			return (0 == ret);
		}
		return false;
	}

	bool post(uint32 count)
	{
		while (count--)
		{
			if (0 != sem_post(&m_sem))
			{
				// the semaphore value would exceed SEM_VALUE_MAX
				return false;
			}
		}
		return true;
	}

	uint32 get_value()
	{
		int count = 0;
		if (0 == sem_getvalue(&m_sem, &count))
		{
			return (uint32)count;
		}
		ASSERT(false && "The sem argument does not refer to a valid semaphore.");
		return 0;
	}

private:
	sem_t  m_sem;
};

#endif//__GNUC__

////////////////////////////////////////////////////////////////////////////////

XSemaphore::XSemaphore(uint32 init_count)
	: m_impl(new(nothrow) XSemaphoreImpl(init_count))
{
	// empty
}

XSemaphore::~XSemaphore()
{
}

void XSemaphore::wait()
{
	m_impl->wait();
}

bool XSemaphore::trywait(uint32 msec)
{
	return m_impl->trywait(msec);
}

bool XSemaphore::post(uint32 count)
{
	return m_impl->post(count);
}

uint32 XSemaphore::get_value()
{
	return m_impl->get_value();
}

} // namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

//#ifdef _XCORE_NEED_TEST
//
//#include "xcore_test.h"
//
//namespace xcore
//{
//
//bool xcore_test_semaphore()
//{
//	return true;
//}
//
//}//namespace xcore

//#endif//_XCORE_NEED_TEST
// 20010-09-29
// xcore_sock_addr.h
// 

//#pragma comment( lib, "ws2_32" )

#include <string.h>

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class __XSockGuarder
////////////////////////////////////////////////////////////////////////////////
class __XSockGuarder
{
public:
	__XSockGuarder()
	{
		#ifdef __WINDOWS__
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		int nRetCode = WSAStartup(wVersionRequested, &wsaData);
		ASSERT((!nRetCode) && "WSAStartup failed!");
		#endif//__WINDOWS__

		#ifdef __GNUC__
		signal(SIGPIPE, SIG_IGN);
		#endif//__GNUC__
	}

	~__XSockGuarder()
	{
		#ifdef __WINDOWS__
		WSACleanup();
		#endif//__WINDOWS__
	}
};
static __XSockGuarder  __g_sock_guarder;


////////////////////////////////////////////////////////////////////////////////
// class XSockAddr
////////////////////////////////////////////////////////////////////////////////

const XSockAddr XSockAddr::AnyAddr;

const XSockAddr XSockAddr::NoneAddr(INADDR_NONE, 0);

XSockAddr::XSockAddr(void)
{
	reset();
}

XSockAddr::XSockAddr(const XSockAddr& addr)
{
	memcpy(this, &addr, sizeof(addr));
}

XSockAddr::XSockAddr(const sockaddr_in& addr)
{
	reset();
	m_inaddr.sin_addr.s_addr = addr.sin_addr.s_addr;
	m_inaddr.sin_port = addr.sin_port;
}

XSockAddr::XSockAddr(const sockaddr& addr)
{
	reset();
	const sockaddr_in& inaddr = (const sockaddr_in&)addr;
	m_inaddr.sin_addr.s_addr = inaddr.sin_addr.s_addr;
	m_inaddr.sin_port = inaddr.sin_port;
}

XSockAddr::XSockAddr(const string& raw_addr)
{
	reset();

    string addr;
    size_t pos = raw_addr.find("http://");
    if (pos != string::npos)
    {
        size_t end_pos = raw_addr.find('/', pos+7);
        if (string::npos == end_pos)
            end_pos = raw_addr.find('?', pos+7);
        if (end_pos != string::npos)
            addr = raw_addr.substr(pos+7, end_pos-pos-7);
        else
            addr = raw_addr.substr(pos+7);
    } else
    {
        addr = raw_addr;
    }

	string host = addr;
	string port = "80";
	pos = addr.find(':');
	if (pos != string::npos)
	{
		host = addr.substr(0, addr.find(':'));
		port = addr.substr(addr.find(':') + 1);
	}

	set_host(host);
	set_port(XStrUtil::try_to_uint_def(port, 0));
}

XSockAddr::XSockAddr(const string& host, uint16 port)
{
	reset();
	set_host(host);
	set_port(port);
}

XSockAddr::XSockAddr(uint32 ip, uint16 port)
{
	reset();
	set_ipaddr(ip);
	set_port(port);
}

XSockAddr::~XSockAddr(void)
{
	reset();
}

XSockAddr& XSockAddr::operator = (const XSockAddr& addr)
{
	if (this != &addr)
	{
		memcpy(&m_inaddr, &addr.m_inaddr, sizeof(m_inaddr));
	}
	return *this;
}

XSockAddr& XSockAddr::operator = (const sockaddr_in& addr)
{
	m_inaddr.sin_addr.s_addr = addr.sin_addr.s_addr;
	m_inaddr.sin_port = addr.sin_port;
	return *this;
}

XSockAddr& XSockAddr::operator = (const sockaddr& addr)
{
	const sockaddr_in& inaddr = (const sockaddr_in&)addr;
	m_inaddr.sin_addr.s_addr = inaddr.sin_addr.s_addr;
	m_inaddr.sin_port = inaddr.sin_port;
	return *this;
}

XSockAddr::operator const sockaddr_in *() const
{
	return &m_inaddr;
}

XSockAddr::operator const sockaddr *() const
{
	return (const sockaddr *)&m_inaddr;
}

XSockAddr::operator sockaddr_in() const
{
	return m_inaddr;
}

XSockAddr::operator sockaddr() const
{
	return *(sockaddr *)&m_inaddr;
}

void XSockAddr::set_port(uint16 port)
{
    m_port = port;
	m_inaddr.sin_port = XByteOrder::local2net(port);
	//m_inaddr.sin_port = htons(port);
}

void XSockAddr::set_ipaddr(uint32 ip)
{
	m_inaddr.sin_addr.s_addr = XByteOrder::local2net(ip);
	//m_inaddr.sin_addr.s_addr = htonl(ip);
}

bool XSockAddr::set_ipaddr(const string& ip)
{
	vector<string> vItems;
	if (4 != XStrUtil::split(ip, vItems, "\r\n\t ."))
	{
		return false;
	}

	uint32 tmp = 0;
	uint32 num = 0;
	for (int i = 0; i < 4; i++)
	{
		if (!XStrUtil::to_uint(vItems[i], tmp) || tmp >= 256)
		{
			return false;
		}
		num <<= 8;
		num |= tmp;
	}
	set_ipaddr(num);
	return true;
}

bool XSockAddr::set_host(const string& host)
{
    m_host = host;
	if (host.empty())
	{
		// ip set "0.0.0.0"
		m_inaddr.sin_addr.s_addr = INADDR_ANY;
		return true;
	}
	if (set_ipaddr(host)) return true;

	XLockGuard<XCritical> lock(XCritical::StaticCritical); // ±£÷§∂‡œﬂ≥Ã∞≤»´

	struct hostent *pHost = gethostbyname(host.c_str());
	if (pHost && pHost->h_addr)
	{
		m_inaddr.sin_addr = *(in_addr *)pHost->h_addr;
		return true;
	}
	else
	{
		m_inaddr.sin_addr.s_addr = INADDR_NONE;
		return false;
	}
}

uint16 XSockAddr::get_port() const
{
	return XByteOrder::net2local((uint16)m_inaddr.sin_port);
	//return ntohs(m_inaddr.sin_port);
}

uint32 XSockAddr::get_ipaddr() const
{
	return XByteOrder::net2local((uint32)m_inaddr.sin_addr.s_addr);
	//return ntohl(m_inaddr.sin_addr.s_addr);
}

string XSockAddr::get_hostname() const
{
	XLockGuard<XCritical> lock(XCritical::StaticCritical); // ±£÷§∂‡œﬂ≥Ã∞≤»´
	
	if (this->m_inaddr.sin_addr.s_addr == INADDR_ANY)
	{
		return local_net_name();
	}

	struct hostent *pHost = gethostbyaddr((char *)&m_inaddr.sin_addr, 4, PF_INET);
	if (pHost && pHost->h_name)
	{
		return pHost->h_name;
	}
	else
	{
		return get_hostaddr();
	}
}

string XSockAddr::get_hostaddr() const
{
	char buf[32];
	uint32 ip_ = get_ipaddr();
	sprintf(buf, "%u.%u.%u.%u", (ip_ >> 24) & 0XFF, (ip_ >> 16) & 0XFF, (ip_ >> 8) & 0XFF, ip_ & 0XFF);
	return buf;
}

string XSockAddr::to_str() const
{
	char buf[32];
	uint32 ip_ = get_ipaddr();
	uint16 port_ = get_port();
	sprintf(buf, "%u.%u.%u.%u:%u", (ip_ >> 24) & 0XFF, (ip_ >> 16) & 0XFF, (ip_ >> 8) & 0XFF, ip_ & 0XFF, port_);
	return buf;
}

void XSockAddr::reset()
{
	memset(&m_inaddr, 0, sizeof(m_inaddr));
	m_inaddr.sin_family = PF_INET;
	return;
}

bool XSockAddr::is_any() const
{
	return (m_inaddr.sin_addr.s_addr == INADDR_ANY);
}

bool XSockAddr::is_none() const
{
	return (m_inaddr.sin_addr.s_addr == INADDR_NONE);
}

bool XSockAddr::is_loopback() const
{
	 return ((get_ipaddr() & 0XFF000001) == 0X7F000001);
}

bool XSockAddr::is_multicast() const
{
	uint32 ip = get_ipaddr();
	return (ip >= 0xE0000000) &&  // 224.0.0.0
		   (ip <= 0xEFFFFFFF); // 239.255.255.255
}

string XSockAddr::local_net_name()
{
	static string name_;

	if (name_.empty())
	{
		char buf[1024] = {};
		struct hostent *pHost = NULL;
		XLockGuard<XCritical> lock(XCritical::StaticCritical); // ±£÷§∂‡œﬂ≥Ã∞≤»´
		if (SOCKET_ERROR != gethostname(buf, 1023))
			pHost = gethostbyname(buf);
		else
			pHost = gethostbyname("");
		
		if (pHost && pHost->h_name)
		{
			name_ = pHost->h_name;
		}
	}
	return name_;
}

XSockAddr XSockAddr::local_mainaddr()
{
	static XSockAddr addr_;
	XLockGuard<XCritical> lock(XCritical::StaticCritical); // ±£÷§∂‡œﬂ≥Ã∞≤»´
	if (!addr_.is_any()) return addr_;

	#ifdef __WINDOWS__
	char buf[1024] = {};
	struct hostent *pHost = NULL;
	if (SOCKET_ERROR != gethostname(buf, 1023))
		pHost = gethostbyname(buf);
	else
		pHost = gethostbyname("");
	if (pHost && pHost->h_addr)
	{
		addr_.m_inaddr.sin_addr = *(in_addr *)pHost->h_addr;
	}
	return addr_;
	#endif//__WINDOWS__

	#ifdef __GNUC__
	int             fd;
	struct ifreq    buf[16];
	struct ifconf   ifc;

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) <= 0) return addr_;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) == -1)
	{
		close(fd);
		return addr_;
	}

	int interface = ifc.ifc_len / sizeof(struct ifreq);
	for (int i = 0; i < interface; i++)
	{
		if (ioctl(fd, SIOCGIFFLAGS, (char *)&buf[i]) == -1) continue;
		if (buf[i].ifr_flags & IFF_LOOPBACK) continue;
		if (!(buf[i].ifr_flags & IFF_UP)) continue;
		if (ioctl(fd, SIOCGIFADDR, (char *)&buf[i]) == 0)
		{
			addr_.m_inaddr.sin_addr = ((struct sockaddr_in *)(&buf[i].ifr_addr))->sin_addr;
			break;
		}
	}
	close(fd);
	return addr_;
	#endif//__GNUC__
}

bool XSockAddr::local_addrs(vector<XSockAddr>& addrs)
{
	static vector<XSockAddr> addrs_;
	addrs.clear();
	XLockGuard<XCritical> lock(XCritical::StaticCritical); // ±£÷§∂‡œﬂ≥Ã∞≤»´
	if (addrs_.size() > 0)
	{
		addrs.insert(addrs.begin(), addrs_.begin(), addrs_.end());
		return true;
	}

	#ifdef __WINDOWS__
	struct hostent *pHost = gethostbyname("");
	if (pHost == NULL) return false;
	for (int i = 0; i < 16; i++)
	{
		char* inaddr = pHost->h_addr_list[i];
		if (inaddr == NULL) break;

		XSockAddr addr;
		addr.m_inaddr.sin_addr = *(in_addr *)inaddr;
		if (addr.is_any()) continue;
		addrs_.push_back(addr);
	}

	addrs.insert(addrs.begin(), addrs_.begin(), addrs_.end());
	return true;
	#endif//__WINDOWS__

	#ifdef __GNUC__
	int             fd;
	struct ifreq    buf[16];
	struct ifconf   ifc;

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) <= 0) return false;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) == -1)
	{
		close(fd);
		return false;
	}

	int interface = ifc.ifc_len / sizeof(struct ifreq);
	for (int i = 0; i < interface; i++)
	{
		if (ioctl(fd, SIOCGIFFLAGS, (char *)&buf[i]) == -1) continue;
		//if (buf[i].ifr_flags & IFF_LOOPBACK) continue;
		if (!(buf[i].ifr_flags & IFF_UP)) continue;
		if (ioctl(fd, SIOCGIFADDR, (char *)&buf[i]) == 0)
		{
			XSockAddr addr;
			addr.m_inaddr.sin_addr = ((struct sockaddr_in *)(&buf[i].ifr_addr))->sin_addr;
			if (addr.is_any()) continue;
			addrs_.push_back(addr);
		}
	}
	
	addrs.insert(addrs.begin(), addrs_.begin(), addrs_.end());
	close(fd);
	return true;
	#endif//__GNUC__
}

bool operator < (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() < addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() < addr2.get_port())))
		return true;
	else
		return false;
}

bool operator <= (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() < addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() <= addr2.get_port())))
		return true;
	else
		return false;
}

bool operator >  (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() > addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() > addr2.get_port())))
		return true;
	else
		return false;
}

bool operator >= (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() > addr2.get_ipaddr()) ||
		((addr1.get_ipaddr() == addr2.get_ipaddr()) &&
		(addr1.get_port() >= addr2.get_port())))
		return true;
	else
		return false;
}

bool operator == (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() == addr2.get_ipaddr()) && 
		(addr1.get_port() == addr2.get_port()))
		return true;
	else
		return false;
}

bool operator != (const XSockAddr& addr1, const XSockAddr& addr2)
{
	if ((addr1.get_ipaddr() != addr2.get_ipaddr()) ||
		(addr1.get_port() != addr2.get_port()))
		return true;
	else
		return false;
}


}//namespace xcore
// 2008-12-03
// xcore_socket.cpp
//

#include <errno.h>
#include <string.h>

namespace xcore {

class IdleSock
{
public:
	IdleSock()
	{
		m_sock.open();
	}

	~IdleSock()
	{
		m_sock.close();
	}

	void attach(SOCKET sock)
	{
		m_sock.attach(sock);
	}

private:
	XSocket m_sock;
};
static IdleSock  g_idleSock;

///////////////////////////////////////////////////////////////////////
// class XSocket
///////////////////////////////////////////////////////////////////////

void XSocket::attach(SOCKET sock)
{
	if (m_sock != sock)
	{
		this->close();
		m_sock = sock;
	}
}

SOCKET XSocket::detach()
{
	SOCKET sock = m_sock;
	m_sock = XCORE_INVALID_SOCKET;
	return sock;
}

bool XSocket::is_open() const
{
	return (m_sock != XCORE_INVALID_SOCKET);
}


///////////////////////////////////////////////////////////////////////
// base interfaces
///////////////////////////////////////////////////////////////////////
bool XSocket::open(int type)
{
	this->close();
	m_sock = ::socket(PF_INET, type, 0);
	return (XCORE_INVALID_SOCKET != m_sock);
}

bool XSocket::bind(const XSockAddr& addr)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	ASSERT(local_addr() == XSockAddr::AnyAddr  && "Socket cann't bind two address.");
	return (0 == ::bind(m_sock, addr, sizeof(sockaddr)));
}

int XSocket::connect(const XSockAddr& addr, int timeout_ms)
{
    if (m_sock == XCORE_INVALID_SOCKET) return false;

    int ret = ::connect(m_sock, addr, sizeof(sockaddr));
    bool first = true;
    if (ret < 0)
    {
        do {
            if (!first) {
                if (!can_send(timeout_ms)) {
                    return -2;
                }
                ret = ::connect(m_sock, addr, sizeof(sockaddr));
                if (0 == ret) return true;
            } else {
                first = false;
            }
            if (_is_can_restore())
            {
                continue;
            }
            //else if (_is_would_block())
            else if ((EAGAIN == errno) ||
                    (EWOULDBLOCK == errno) ||
                    (EINPROGRESS == errno))
            {
                continue;
            }
            else if ((EALREADY == errno) ||
                    (EISCONN == errno))
            {
                return 1;
            }
            printf("connect errno:%d\n", errno);
            return -1;
        } while (true);
    }
    return 1;
}

bool XSocket::listen(const XSockAddr& addr, int backlog)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	if (backlog < 0) backlog = 0X7FFFFFFF;
	if (!set_reuse_addr(true)) return false;
	if (!this->bind(addr)) return false;
	if (0 != ::listen(m_sock, backlog)) return false;
	return true;
}

bool XSocket::accept(XSocket &sock, XSockAddr* remote_addr)
{
	ASSERT(this != &sock);
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	SOCKET s = XCORE_INVALID_SOCKET;
	sock.attach(XCORE_INVALID_SOCKET);

	do
	{
		struct sockaddr addr = {};
		socklen_t sockLen = sizeof(sockaddr);
		s = ::accept(m_sock, (sockaddr *)&addr, &sockLen);
		if (remote_addr) *remote_addr = addr;
		if (s == XCORE_INVALID_SOCKET)
		{
			if (_is_can_restore())
			{
				continue;
			}
			else if (_is_would_block())
			{
				break;
			}
			else if (_is_emfile())
			{
				g_idleSock.attach(XCORE_INVALID_SOCKET);
				g_idleSock.attach(::accept(m_sock, NULL, NULL));
				g_idleSock.attach(XCORE_INVALID_SOCKET);
				g_idleSock.attach(::socket(AF_INET, SOCK_STREAM, 0));
				break;
			}
			return false;
		}
	} while (false);

	sock.attach(s);
	return true;
}

bool XSocket::abort()
{
	return close(0);
}

XSockAddr XSocket::local_addr() const
{
	if (m_sock == XCORE_INVALID_SOCKET) return XSockAddr::AnyAddr;

	struct sockaddr saddr = {};
	socklen_t namelen = (socklen_t)sizeof(sockaddr);
	if (0 != getsockname(m_sock, &saddr, &namelen)) return XSockAddr::AnyAddr;
	return saddr;
}

XSockAddr XSocket::remote_addr() const
{
	if (m_sock == XCORE_INVALID_SOCKET) return XSockAddr::AnyAddr;

	struct sockaddr saddr = {};
	socklen_t namelen = (socklen_t)sizeof(sockaddr);
	if (0 != getpeername(m_sock, &saddr, &namelen)) return XSockAddr::AnyAddr;
	return saddr;
}

///////////////////////////////////////////////////////////////////////
// option interfaces
///////////////////////////////////////////////////////////////////////
bool XSocket::set_reuse_addr(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int nVal = (int)bl;
	int ret = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&nVal, sizeof(int));
	return (0 == ret);
}

bool XSocket::set_keep_alive(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int nVal = (int)bl;
	int ret = setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&nVal, sizeof(int));
	return (0 == ret);
}

bool XSocket::set_tcp_nodelay(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int nVal = (int)bl;
	int ret = setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&nVal, sizeof(int));
	return (0 == ret);
}

bool XSocket::set_linger(uint16 delay_sec, bool bl)
{
	struct linger lingerStruct = {(uint16)bl, delay_sec};
	int ret = setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));
	return (0 == ret);
}

bool XSocket::set_nonblock(bool bl)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	#ifdef __WINDOWS__
	unsigned long ulOption = (unsigned long)bl;
	int ret = ioctlsocket(m_sock, FIONBIO, (unsigned long *)&ulOption);
	#endif//__WINDOWS__

	#ifdef __GNUC__
	int ret = fcntl(m_sock, F_GETFL, 0);
	if (ret == -1) return false;
	if (bl)
		ret = fcntl(m_sock, F_SETFL, ret | O_NONBLOCK);
	else
		ret = fcntl(m_sock, F_SETFL, ret & (~O_NONBLOCK));
	#endif//__GNUC__

	return (0 == ret);
}

bool XSocket::set_send_bufsize(uint32 nSize)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int ret = setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (const char*)&nSize, sizeof(int));
	return (0 == ret);
}

bool XSocket::get_send_bufsize(uint32& nSize) const
{
	nSize = 0;
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	socklen_t optlen = sizeof(int);
	int ret = getsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char *)&nSize, &optlen);
	return (0 == ret);
}

bool XSocket::set_recv_bufsize(uint32 nSize)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	int ret = setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (const char*)&nSize, sizeof(int));
	return (0 == ret);
}

bool XSocket::get_recv_bufsize(uint32& nSize) const
{
	nSize = 0;
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	socklen_t optlen = sizeof(int);
	int ret = getsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char *)&nSize, &optlen);
	return (0 == ret);
}

///////////////////////////////////////////////////////////////////////
// date interfaces
///////////////////////////////////////////////////////////////////////
int XSocket::send(const void* buf, int len)
{
	if ((m_sock == XCORE_INVALID_SOCKET)) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;

	do
	{
		int ret = ::send(m_sock, (const char *)buf, len, 0);
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		return ret;
	} while (false);

	return 0;
}

int XSocket::recv(void* buf, int len)
{
	if ((m_sock == XCORE_INVALID_SOCKET)) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;

	do
	{
		int ret = ::recv(m_sock, (char *)buf, len, 0);
		if (ret == 0) return -1; // closed by remote host
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		return ret;
	} while (false);

	return 0;
}

int XSocket::sendto(const void* buf, int len, const XSockAddr& addr)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;

	do
	{
		int ret = ::sendto(m_sock, (const char *)buf, len, 0, addr, sizeof(sockaddr));
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		return ret;
	} while (false);

	return 0;
}

int XSocket::recvfrom(void* buf, int len, XSockAddr& addr)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || (len <= 0)) return 0;
	addr.reset();

	do
	{
		struct sockaddr saddr;
		socklen_t fromlen = (socklen_t)sizeof(sockaddr);
		int ret = ::recvfrom(m_sock, (char *)buf, len, 0, &saddr, &fromlen);
		if (ret == 0) return -1; // shutdown by remote host
		if (ret < 0)
		{
			if (_is_can_restore())
			{
				continue;
			}
			if (_is_would_block())
			{
				return 0;
			}
			return -1;
		}
		addr = saddr;
		return ret;
	} while (false);

	return 0;

}

int XSocket::send_n(const void* buf, int len, int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || len <= 0) return 0;

	int sendsize = 0;
	do
	{
		if (!can_send(timeout_ms)) {
            //printf("send time out\n");
            return sendsize;
        }
		int ret = this->send((const char *)buf + sendsize, len - sendsize);
		if (ret < 0) return -1;
		sendsize += ret;
	} while(sendsize < len);

	ASSERT(sendsize == len);
	return sendsize;
}

/*find str form begin*/
char* find_str_begin(const char* src,unsigned srclen,const char* matchstr,unsigned matchstrlen) {
    if(src == NULL || matchstr == NULL || srclen <= 0 || matchstrlen <= 0) {
        return NULL;
    }
    unsigned k,j;
    char* p_str = (char*)src;
    for(unsigned i = 0; i < srclen; i++) {
        j = i;
        k = 0;
        while(j < srclen && (tolower(p_str[j]) == tolower(matchstr[k]))) {
            j++;
            k++;
            if( k == matchstrlen) {
                return (p_str + j - matchstrlen);
            }
        }
    }
    return NULL;
}

/*
   0       ªπ–Ë“™ºÃ–¯Ω” ’
   -1       ß∞‹
   */
int is_http_complete(const char* p_data, unsigned int data_len) {
    if(p_data == NULL || data_len < 4 || data_len<= 0) {
        return 0;
    }
    /*first match http head end*/
    int head_len = 0;
    char *p_head_end = find_str_begin(p_data,data_len,"\r\n\r\n",4);
    if(p_head_end == NULL) {
        p_head_end = find_str_begin(p_data,data_len,"\n\n",2);
        if ( p_head_end == NULL ) return 0;
        else head_len = p_head_end + 2 - p_data;
    } else {
        head_len = p_head_end + 4 - p_data;
    }
    //get Content-Length:
    char* pcontent_length = find_str_begin(p_data,data_len,"Content-Length:",15);
    if(pcontent_length != NULL) {
        int content_length = atoi(pcontent_length + 15);
        if( (int)(data_len) < content_length + head_len) {
            return 0;
        } else {
            return content_length + head_len;
        }
    } else {
        // try find chunkend
        char* chunk_end = find_str_begin(p_data,data_len,"\r\n0\r\n\r\n",7);
        if (chunk_end) {
            return (chunk_end + 7 - p_data);
        }
        // √ª Content-Length £¨ƒ«æÕ «÷ª”–Õ∑≤ø¡À
        return head_len;
    }
    return -1;
}

int XSocket::recv_http(char*& buf, int& len,
                       char*& last_left, int& left_len,
                       char*& http_head, int timeout_ms) {
    if (m_sock == XCORE_INVALID_SOCKET) return -1;
    if ((buf == NULL) || len <= 0) return 0;
    int recvsize = 0;
    if (left_len > 0) {
        do{
            int ret = is_http_complete(last_left, left_len);
            if (ret > 0) {
                http_head = last_left;
                last_left = last_left + ret;
                left_len = left_len - ret;
                return ret;
            }
        } while(false);
        if (left_len > 0) {
            memmove(buf, last_left, left_len);
            recvsize = left_len;
        }
    }
    //int cont_len = -1;
    //char *head_end = NULL;
    //char *content_len_end = NULL;
    do {
        if (!can_recv(timeout_ms)) {
            //printf("recv time out\n");
            return -2;
        }
        int ret = this->recv((char *)buf + recvsize, len - recvsize);
        if (ret < 0) {
            http_head = NULL;
            left_len = 0;
            return -1;
        }
        recvsize += ret;
        do{
            int ret = is_http_complete(buf, recvsize);
            if (ret > 0) {
                http_head = buf;
                last_left = buf + ret;
                left_len = recvsize - ret;
                return ret;
            }
        } while(false);
        if (recvsize + 1024 > len) {
            len += 10*1024;
            buf = (char*)realloc(buf, len);
        }
    } while(recvsize < len);
    return -1;
}

int XSocket::recv_one_http(std::string& one_http, int timeout_ms) {
    int buf_size = 1024*100;
    char *buf = (char*)malloc(buf_size);
    char *last_left = buf;
    int left_len = 0;
    char* http_head = NULL;
    int one_len = recv_http(buf, buf_size, last_left, left_len, http_head, timeout_ms);
    if (one_len > 0) {
        one_http.assign(buf, one_len);
    }
    free(buf);
    return one_len;
}

bool XSocket::mutex_close() {
	bool ret = false;
	if (!isstop) {
		mutex_.lock();
		close();
		mutex_.unlock();
		ret = true;
	}
	return ret;
}

bool XSocket::reconnect(const XSockAddr& addr) {
	bool ret = false;
	if (!isstop) {
		mutex_.lock();
		close();
		//int tryc = 0;
		int waittime = 100;
		while (!ret) {
			open(SOCK_STREAM);
			set_nonblock(true);
			ret = connect(addr);
			if (ret) {
				break;
			} else {
				close();
			}
			xcore::sleep(waittime);
			waittime *= 2;
			if (waittime > 10000) {
				waittime = 100;
			}
		}
		mutex_.unlock();
	}
	return ret;
}


int XSocket::recv_n(void* buf, int len, int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return -1;
	if ((buf == NULL) || len <= 0) return 0;

	int recvsize = 0;
	do
	{
		if (!can_recv(timeout_ms)) return recvsize;
		int ret = this->recv((char *)buf + recvsize, len - recvsize);
		if (ret < 0)
		{
			if (recvsize > 0)
				return recvsize;
			else
				return -1;
		}
		recvsize += ret;
	} while(recvsize < len);

	ASSERT(recvsize == len);
	return recvsize;
}

///////////////////////////////////////////////////////////////////////
// private interfaces
///////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__
bool XSocket::can_recv(int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return true;

	struct timeval tv = {};
	timeval *pcTimeout = NULL;
	if (timeout_ms >= 0)
	{
		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		pcTimeout = &tv;
	}

	do
	{
		fd_set rdset, exceptset;
		FD_ZERO(&rdset);
		FD_ZERO(&exceptset);
		FD_SET(m_sock, &rdset);
		FD_SET(m_sock, &exceptset);

		// If timeout is NULL (no timeout), select can block indefinitely.
		// In windows, pcTimeout not altered; In linux, pcTimeout may update.
		int ret = select((int)m_sock + 1, &rdset, NULL, &exceptset, pcTimeout);
		if (ret == 0) return false;
		if (FD_ISSET(m_sock, &rdset) || FD_ISSET(m_sock, &exceptset)) return true;
		if (_is_can_restore()) continue;
		//fprintf(stderr, "socket select return %d, errno:%d\n", ret, errno);
		//ASSERT(!"socket select exception.");
	} while (false);

	return false;
}

bool XSocket::can_send(int timeout_ms)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	struct timeval tv = {};
	timeval *pcTimeout = NULL;
	if (timeout_ms >= 0)
	{
		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		pcTimeout = &tv;
	}

	do
	{
		fd_set wrset, exceptset;
		FD_ZERO(&wrset);
		FD_ZERO(&exceptset);
		FD_SET(m_sock, &wrset);
		FD_SET(m_sock, &exceptset);

		// If timeout is NULL (no timeout), select can block indefinitely.
		// In windows, pcTimeout not altered; In linux, pcTimeout may update.
		int ret = select((int)m_sock + 1, NULL, &wrset, &exceptset, pcTimeout);
		if (ret == 0) return false;
		if (FD_ISSET(m_sock, &wrset)) return true;
		if (FD_ISSET(m_sock, &exceptset)) return false;
		if (_is_can_restore()) continue;
		fprintf(stderr, "socket select return %d, errno:%d\n", ret, errno);
		//ASSERT(!"socket select exception.");
	} while (false);

	return false;
}

bool XSocket::send_v(const iovec* iov, int cnt)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;
	if (iov == NULL || cnt == 0) return true;
	ASSERT(cnt <= 20);

	WSABUF bufs[20];
	ULONG total = 0;
	for (int i = 0; i < cnt; i++)
	{
		ASSERT(iov->iov_base);
		bufs[i].buf = (CHAR *)iov->iov_base;
		bufs[i].len = (ULONG)iov->iov_len;
		total += bufs[i].len;
	}

	DWORD snds = 0;
	if (0 != WSASend(m_sock, bufs, cnt, &snds, 0, NULL, NULL)) return false;
	if (snds != total) return false;
	return true;
}

bool XSocket::shutdown()
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		return (-1 != ::shutdown(m_sock, SD_SEND));
	}
	return true;
}

bool XSocket::close(int delay)
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		SOCKET sock = m_sock;
		m_sock = XCORE_INVALID_SOCKET;

		if (delay >= 0)
		{
			struct linger linger_ = { 1, delay };
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
		}

		return (-1 != ::closesocket(sock));
	}
	return true;
}

bool XSocket::_is_can_restore()
{
	return (WSAEINTR == WSAGetLastError());
}

bool XSocket::_is_already()
{
	return ((WSAEALREADY == WSAGetLastError()) ||
		    (WSAEINPROGRESS == WSAGetLastError()) ||
			(WSAEISCONN == WSAGetLastError()));
}

bool XSocket::_is_would_block()
{
	return ((WSAEWOULDBLOCK == WSAGetLastError()) ||
		    (WSA_IO_PENDING == WSAGetLastError()));
}

bool XSocket::_is_emfile()
{
	return (WSAEMFILE == WSAGetLastError());
}
#endif//__WINDOWS__

#ifdef __GNUC__
bool XSocket::can_recv(int timeout_ms)
{
	if (timeout_ms < 0) timeout_ms = -1;
	if (m_sock == XCORE_INVALID_SOCKET) return true;

	do
	{
		struct pollfd  event;
		event.fd = m_sock;
		event.events = POLLIN;  // ≤ªøº¬«¥¯Õ‚ ˝æ›
		int ret = poll(&event, 1, timeout_ms);
		if (ret > 0)
		{
			if (event.revents & POLLIN)
			{
				int err = 0;
				socklen_t len = (socklen_t)sizeof(err);
				if (getsockopt(m_sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) return false;
				return (err == 0);
			}
			else
			{
				ASSERT(false);
				return false;
			}
		}
		if (ret == 0) return false;
		if (_is_can_restore()) continue;
		fprintf(stderr, "socket poll return %d, errno:%d\n", ret, errno);
		//ASSERT(!"socket poll exception.");
	} while (false);

	return false;
}

bool XSocket::can_send(int timeout_ms)
{
	if (timeout_ms < 0) timeout_ms = -1;
	if (m_sock == XCORE_INVALID_SOCKET) return false;

	do
	{
		struct pollfd  event;
		event.fd = m_sock;
		event.events = POLLOUT;
		int ret = poll(&event, 1, timeout_ms);
		if (ret > 0)
		{
			if (event.revents & POLLOUT)
			{
				int err = 0;
				socklen_t len = (socklen_t)sizeof(err);
				if (getsockopt(m_sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) return false;
				return (err == 0);
			}
			else
			{
				return false;
			}
		}
		if (ret == 0) return false;
		if (_is_can_restore()) continue;
		fprintf(stderr, "socket poll return %d, errno:%d\n", ret, errno);
		ASSERT(!"socket poll exception.");
	} while (false);

	return false;
}

bool XSocket::send_v(const iovec* iov, int cnt)
{
	if (m_sock == XCORE_INVALID_SOCKET) return false;
	if (iov == NULL || cnt == 0) return true;

	size_t total = 0;
	for (int i = 0; i < cnt; i++)
	{
		ASSERT(iov->iov_base);
		total += iov->iov_len;
	}

	int ret = ::writev(m_sock, iov, cnt);
	if (ret < 0) return false;
	if (ret != (int)total) return false;
	return true;
}

bool XSocket::shutdown()
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		return (-1 != ::shutdown(m_sock, SHUT_WR));
	}
	return true;
}

bool XSocket::close(int delay)
{
	if (m_sock != XCORE_INVALID_SOCKET)
	{
		SOCKET sock = m_sock;
		m_sock = XCORE_INVALID_SOCKET;

		if (delay >= 0)
		{
			struct linger linger_ = { 1, delay };
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
		}

		return (-1 != ::close(sock));
	}
	return true;
}

bool XSocket::_is_can_restore()
{
	return (EINTR == errno);
}

bool XSocket::_is_already()
{
	return ((EALREADY == errno) ||
		    (EINPROGRESS == errno));
			//(EISCONN == errno));
}

bool XSocket::_is_would_block()
{
	return ((EAGAIN == errno) ||
		    (EWOULDBLOCK == errno));
}

bool XSocket::_is_emfile()
{
	return (EMFILE == errno);
}
#endif//__GNUC__

} // namespace xcore


// 2009-02-04
// xcore_str_util.cpp
// 

#include <math.h>
#include <string.h>

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XStrUtil
///////////////////////////////////////////////////////////////////////////////
string& XStrUtil::chop_head(string &strSrc, const char *pcszCharSet)
{
	if (pcszCharSet == NULL) return strSrc;
	size_t pos = strSrc.find_first_not_of(pcszCharSet);
	return strSrc.erase(0, pos);
}

string& XStrUtil::chop_tail(string &strSrc, const char *pcszCharSet)
{
	if (pcszCharSet == NULL) return strSrc;
	size_t pos = strSrc.find_last_not_of(pcszCharSet);
	if (pos == string::npos)
	{
		strSrc.clear();
		return strSrc;
	}
	return strSrc.erase(++pos);
}

string& XStrUtil::chop(string &strSrc, const char *pcszCharSet)
{
	chop_head(strSrc, pcszCharSet);
	return chop_tail(strSrc, pcszCharSet);
}

void XStrUtil::to_upper(char *pszSrc)
{
	if (pszSrc == NULL) return;
	char *pos = (char *)pszSrc;
	char diff = 'A' - 'a';
	while (*pos != '\0')
	{
		if ('a' <= *pos && *pos <= 'z')
		{
			*pos += diff;
		}
		pos++;
	}
	return;
}

void XStrUtil::to_lower(char *pszSrc)
{
	if (pszSrc == NULL) return;
	char *pos = (char *)pszSrc;
	char diff = 'A' - 'a';
	while (*pos != '\0')
	{
		if ('A' <= *pos && *pos <= 'Z')
		{
			*pos -= diff;
		}
		pos++;
	}
	return;
}

void XStrUtil::to_lower(string &strSrc)
{
	return to_lower((char *)strSrc.c_str());
}

void XStrUtil::to_upper(string &strSrc)
{
	return to_upper((char *)strSrc.c_str());
}

void XStrUtil::replace(char* str, char oldch, char newch)
{
	if (str == NULL || oldch == newch) return;
	char* pos = str;
	while (*pos) // (*pos != '\0')
	{
		if (*pos == oldch) *pos = newch;
		pos++;
	}
	return;
}

void XStrUtil::replace(char* str, const char* oldCharSet, char newch)
{
	if (str == NULL || oldCharSet == NULL) return;
	char* pos = str;
	const char* p = NULL;
	while (*pos) // (*pos != '\0')
	{
		for (p = oldCharSet; *p; p++)
		{
			if (*pos == *p)
			{
				*pos = newch;
				break;
			}
		}
		pos++;
	}
	return;
}

int XStrUtil::compare(const char* pszSrc1, const char* pszSrc2, int length)
{
	ASSERT(pszSrc1 && pszSrc2);

	int ret = 0;
	const char *left = pszSrc1;
	const char *right = pszSrc2;

	while ((length != 0) && (*left != '\0') && (*right != '\0'))
	{
		if (length > 0) length--;
		ret = *left++ - *right++;
		if (ret != 0) return ret;
	}
	if (length == 0) return 0;
	return (*left - *right);
}

int XStrUtil::compare(const string &str1, const string &str2, int length)
{
	return compare(str1.c_str(), str2.c_str(), length);
}

int XStrUtil::compare_nocase(const char* pszSrc1, const char* pszSrc2, int length)
{
	ASSERT(pszSrc1 && pszSrc2);

	int ret = 0;
	const char *left = pszSrc1;
	const char *right = pszSrc2;

	while ((length != 0) && (*left != '\0') && (*right != '\0'))
	{
		if (length > 0) length--;
		ret = ::tolower(*left++) - ::tolower(*right++);
		if (ret != 0) return ret;
	}
	if (length == 0) return 0;
	return (*left - *right);
}

int XStrUtil::compare_nocase(const string &str1, const string &str2, int length/* = -1*/)
{
	return compare_nocase(str1.c_str(), str2.c_str(), length);
}

uint32 XStrUtil::split(const string &strSrc, vector<string> &vItems, const char *pcszCharSet/* = " \r\n\t"*/, int nMaxCount/* = -1*/)
{
	vItems.clear();

	size_t pos_begin = 0;
	size_t pos_end = 0;
	int count = 0;
	while (pos_end != string::npos)
	{
		pos_begin = strSrc.find_first_not_of(pcszCharSet, pos_end);
		if (pos_begin == string::npos) break;
		pos_end = strSrc.find_first_of(pcszCharSet, pos_begin);
		string strTmp(strSrc, pos_begin, pos_end - pos_begin);
		if (!strTmp.empty())
		{
			count++;
			vItems.push_back(strTmp);
		}
		if (nMaxCount > 0 && count >= nMaxCount)
		{
			break;
		}
	}
	return (uint32)vItems.size();
}

bool XStrUtil::to_int(const string &strSrc, int &nValue, int radix/* = 10*/)
{
	char* endPtr = 0;
	string str = strSrc;
	
	chop(str);
	if (str.empty()) return false;

	errno = 0;
	nValue = strtol(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return false;
	}
	if (errno == ERANGE) return false;
	return true;
}

int XStrUtil::to_int_def(const string &strSrc, int def/* = -1*/, int radix/* = 10*/)
{
	char* endPtr = 0;
	int nValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	nValue = strtol(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return nValue;
}

int XStrUtil::try_to_int_def(const string &strSrc, int def/* = -1*/, int radix/* = 10*/)
 {
	char* endPtr = 0;
	int nValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	nValue = strtol(str.c_str(), &endPtr, radix);
	if (endPtr == str.c_str())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return nValue;
}

bool XStrUtil::to_uint(const string &strSrc, uint32 &uValue, int radix/* = 10*/)
{
	char* endPtr = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return false;

	errno = 0;
	uValue = strtoul(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return false;
	}
	if (errno == ERANGE) return false;
	return true;
}

uint32 XStrUtil::to_uint_def(const string &strSrc, uint32 def/* = 0*/, int radix/* = 10*/)
{
	char* endPtr = 0;
	uint32 uValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	uValue = strtoul(str.c_str(), &endPtr, radix);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return uValue;
}

uint32 XStrUtil::try_to_uint_def(const string &strSrc, uint32 def/* = 0*/, int radix/* = 10*/)
{
	char* endPtr = 0;
	uint32 uValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	uValue = strtoul(str.c_str(), &endPtr, radix);
	if (endPtr == str.c_str())
	{
		return def;
	}
	if (errno == ERANGE) return false;
	return uValue;
}

bool XStrUtil::to_float(const string &strSrc, double &value)
{
	char* endPtr = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return false;

	errno = 0;
	value = strtod(str.c_str(), &endPtr);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return false;
	}
	if (errno == ERANGE) return false;
	return true;
}

double XStrUtil::to_float_def(const string &strSrc, double def/* = 0.0*/)
{
	char* endPtr = 0;
	double fValue = 0.0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	fValue = strtod(str.c_str(), &endPtr);
	if (endPtr - str.c_str() != (int)str.size())
	{
		return def;
	}
	if (errno == ERANGE) return def;
	return fValue;
}

double	XStrUtil::try_to_float_def(const string &strSrc, double def/* = 0.0*/)
{
	char* endPtr = 0;
	double fValue = 0;
	string str = strSrc;

	chop(str);
	if (str.empty()) return def;

	errno = 0;
	fValue = strtod(str.c_str(), &endPtr);
	if (endPtr == str.c_str())
	{
		return def;
	}
	if (errno == ERANGE) return def;
	return fValue;
}

string XStrUtil::to_str(int nVal, const char* cpszFormat)
{
	char buf[128];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 100) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, nVal);
	else
		sprintf(buf, "%d", nVal);
	return buf;
}

string XStrUtil::to_str(uint32 uVal, const char* cpszFormat)
{
	char buf[128];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 100) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, uVal);
	else
		sprintf(buf, "%u", uVal);
	return buf;
}

string XStrUtil::to_str(int64 nlVal, const char* cpszFormat)
{
	char buf[256];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 240) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, nlVal);
	else
		sprintf(buf, "%lld", (long long int)nlVal);
	return buf;
}

string XStrUtil::to_str(uint64 ulVal, const char* cpszFormat)
{
	char buf[256];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 240) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, ulVal);
	else
		sprintf(buf, "%llu", (long long int)ulVal);
	return buf;
}

string XStrUtil::to_str(double fVal, const char* cpszFormat)
{
	char buf[256];
	ASSERT((!cpszFormat || strlen(cpszFormat) < 240) && "Format too long");
	if (cpszFormat)
		sprintf(buf, cpszFormat, fVal);
	else
		sprintf(buf, "%f", fVal);
	return buf;
}

uint32 XStrUtil::hash_code(const char* str)
{
	if (str == NULL) return 0;

	uint32 h = 0;
	while(*str)
	{
		h = 31 * h + (*str++);
	}
	return h;
}

uint32 XStrUtil::murmur_hash(const void* key, uint32 len)
{
	if (key == NULL) return 0;

	const uint32 m = 0X5BD1E995;
	const uint32 r = 24;
	const uint32 seed = 97;
	uint32 h = seed ^ len;

	// Mix 4 bytes at a time into the hash
	const BYTE* data = (const BYTE *)key;
	while(len >= 4)
	{
		uint32 k = *(uint32 *)data;
		k *= m; 
		k ^= k >> r; 
		k *= m; 
		h *= m; 
		h ^= k;
		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array
	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}

static char __dump_view(char ch)
{
	if (ch <= 31 || ch >= 127) return '.';
	return ch;
}

void XStrUtil::dump(string& result, const void* pdata, uint32 length)
{
	result.clear();
	if (pdata == NULL || length == 0) return;

	char buf[128];
	const uint8* src = (const uint8*)pdata;
	for ( ; length >= 16; length -= 16, src += 16)
	{
		sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X    ",
				src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7],
				src[8], src[9], src[10], src[11], src[12], src[13], src[14], src[15]);
		result += buf;

		sprintf(buf, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\r\n",
				__dump_view(src[0]), __dump_view(src[1]), __dump_view(src[2]), __dump_view(src[3]),
				__dump_view(src[4]), __dump_view(src[5]), __dump_view(src[6]), __dump_view(src[7]),
				__dump_view(src[8]), __dump_view(src[9]), __dump_view(src[10]), __dump_view(src[11]),
				__dump_view(src[12]), __dump_view(src[13]), __dump_view(src[14]), __dump_view(src[15]));
		result += buf;
	}

	for (uint32 i = 0; i < length; i++)
	{
		sprintf(buf, "%02X ", src[i]);
		result += buf;
	}
	if (length % 16) result.append((16 - length) * 3 + 3, ' ');

	for (uint32 i = 0; i < length; i++)
	{
		result += __dump_view(src[i]);
	}
	if (length % 16) result += "\r\n";
	return;
}

} // namespace xcore


// 2008-02-23
// xcore_thread.cpp
//

#ifdef __WINDOWS__
#include <process.h>
#endif//__WINDOWS__

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XThread
///////////////////////////////////////////////////////////////////////////////
XThread::XThread()
	: m_id(0)
	, m_state(CREATE)
	, m_runnable(NULL)
	, m_handle(INVALID_HANDLE_VALUE)
	, m_quit(true)
{
	// empty
}

XThread::~XThread()
{
	ASSERT(!m_id && "Need to call join() before XThread destruction.");
}

#ifdef __WINDOWS__
unsigned __stdcall invok_proc_win(void *arg)
{
	static XAtomic32 seed(0);
	::srand((uint32)time(NULL) | ((uint32)(++seed) << 16));

	XThread *pThis = static_cast<XThread *>(arg);
	ASSERT(pThis);
	ASSERT(pThis->m_runnable);
	pThis->m_state = XThread::RUNNING;
	try
	{
	pThis->m_runnable->run(pThis);
	}
	catch(...)
	{
		ASSERT(!"XThread::run() throw exception.");
	}
	pThis->m_state = XThread::STOP;
	return 0;
}

void* invok_proc_posix(void *pParam)
{
	return NULL;
}

bool XThread::start(XRunnable* r, uint32 stack_size)
{
	ASSERT(m_id == 0 && "Thread already start.");
	if (m_id != 0) return true;
	if (r == NULL)
		m_runnable = this;
	else
		m_runnable = r;

	unsigned id = 0;
	m_state = START;
	m_handle = (HANDLE)_beginthreadex(NULL, stack_size, invok_proc_win, this, 0, &id);
	if (m_handle == (HANDLE)-1L)
	{
		m_state = JOINED;
		return false;  // msdn: _beginthread returns -1L on an error.
	}
	m_id = (uint64)id;
	return true;
}

void XThread::stop()
{
	m_quit.set();
}

void XThread::join()
{
	if (m_id == 0) return;
	ASSERT(m_id != xcore::thread_id() && "Cannot join self.");
	WaitForSingleObject(m_handle, INFINITE);
	CloseHandle(m_handle);
	m_handle = INVALID_HANDLE_VALUE;
	m_id = 0;
	m_quit.reset();
	m_state = CREATE;
	return;
}

void XThread::kill()
{
	ASSERT(m_id != xcore::thread_id() && "Cannot kill self.");
	TerminateThread(m_handle, 0);
	m_state = STOP;
}

bool XThread::wait_quit(uint32 msec)
{
	return m_quit.trywait(msec);
}

uint32 thread_id()
{
	return (uint32)GetCurrentThreadId();
}

void sleep(uint32 msec)
{
	// windowsœ¬”√select()µƒ µœ÷µƒsleepπ¶ƒ‹≤ªƒ‹”√(≤ª µ”√)
	// windowsœ¬”√WaitForSingleObject() µœ÷µƒsleepπ¶ƒ‹£¨æ´∂»”Î::Sleep()œ‡Õ¨
	// windowsœ¬::Sleep(0)ª·±ª∫ˆ¬‘
	// windowsœ¬::Sleep()≤ª◊º»∑£¨”Î ±º‰∆¨”–πÿ£¨æ´»∑∂»“ª∞„Œ™10ms
	if (msec == 0) msec = 1;
	::Sleep(msec);
}
#endif//__WINDOWS__


///////////////////////////////////////////////////////////////////////////////
// class XThread
///////////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__
unsigned __stdcall invok_proc_win(void *arg)
{
	return 0;
}

void* invok_proc_posix(void *arg)
{
	static XAtomic32 seed(0);
	::srand((uint32)time(NULL) | ((uint32)(++seed) << 16));

	XThread *pThis = static_cast<XThread *>(arg);
	ASSERT(pThis);
	ASSERT(pThis->m_runnable);
	pThis->m_state = XThread::RUNNING;
	try
	{
	pThis->m_runnable->run(pThis);
	}
	catch(...)
	{
		//ASSERT(!"XThread::run() throw exception.");
	}
	pThis->m_state = XThread::STOP;
	return NULL;
}

bool XThread::start(XRunnable* r, uint32 stack_size)
{
	ASSERT(m_id == 0 && "Thread already start.");
	if (m_id != 0) return true;
	if (r == NULL)
		m_runnable = this;
	else
		m_runnable = r;

	pthread_t id = 0;
	int ret = 0;
	m_state = START;
	if (stack_size > 0)
	{
		pthread_attr_t attr;
		if(stack_size < PTHREAD_STACK_MIN) stack_size = PTHREAD_STACK_MIN;
		VERIFY(!pthread_attr_init(&attr));
		VERIFY(!pthread_attr_setstacksize(&attr, stack_size));
		ret = pthread_create(&id, &attr, invok_proc_posix, this);
		VERIFY(!pthread_attr_destroy(&attr));
	}
	else
	{
		ret = pthread_create(&id, NULL, invok_proc_posix, this);
	}
	if (0 != ret)
	{
		m_state = JOINED;
		return false;
	}
	m_id = (uint64)id;
	return true;
}

void XThread::stop()
{
	m_quit.set();
}

void XThread::join()
{
	if (m_id == 0) return;
	ASSERT(m_id != xcore::thread_id() && "Cannot join self.");
	pthread_join(m_id, NULL);
	m_id = 0;
	m_quit.reset();
	m_state = CREATE;
	return;
}

void XThread::kill()
{
	ASSERT(m_id != xcore::thread_id() && "Cannot kill self.");
	pthread_cancel((pthread_t)m_id);
	m_state = STOP;
}

bool XThread::wait_quit(uint32 msec)
{
	return m_quit.trywait(msec);
}

uint32 thread_id()
{
	return (uint32)pthread_self();
}

void sleep(uint32 msec)
{
	// linux…œµƒnanosleep∫Õalarmœ‡Õ¨£¨∂º «ª˘”⁄ƒ⁄∫À ±÷”ª˙÷∆ µœ÷£¨ ‹linuxƒ⁄∫À ±÷” µœ÷µƒ”∞œÏ
	// Linux/i386…œ «10 ms £¨Linux/Alpha…œ «1ms
	// nanosleep();
	// clock_nanosleep();

	// msecŒ™0ª·±ª∫ˆ¬‘
	// linuxœ¬select()æ´∂»Œ™1ms
	if (msec == 0) msec = 1;

	struct timeval tv;
	tv.tv_sec = msec / 1000;
	tv.tv_usec = (msec % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
	return;
}

#endif//__GNUC__

} // namespace xcore



// 2009-07-28
// xcore_timer.cpp
//


#include <map>
#include <new>

using std::multimap;
using std::make_pair;
using std::nothrow;

namespace xcore {

static XAtomic32 XTimer_ID_Generater(0);

///////////////////////////////////////////////////////////////////////////////
// class __TimerTask
////////////////////////////////////////////////////////////////////////////////
class __TimerTask
{
public:
	uint32             m_id;
	uint32             m_interval_ms; // ÷¥––º‰∏Ù ±º‰£¨µ•Œª£∫∫¡√Î
	int                m_repeat;     // ÷¥––º∆ ˝£¨<0 ”¿æ√÷ÿ∏¥÷¥––£¨0 ÕÀ≥ˆ£¨>0  £”‡÷¥––¥Œ ˝
	void*              m_ptrData; // ÷ª‘ –Ìª˘±æ¿‡–Õ
	XTimeSpan          m_expires;
	volatile bool      m_del_flag;
	XTimer::ICallBack* m_callback;

	__TimerTask(XTimer::ICallBack* callback, uint32 interval_ms, int repeat, void* ptr = NULL)
		: m_id(++XTimer_ID_Generater)
		, m_interval_ms(interval_ms)
		, m_repeat(repeat)
		, m_ptrData(ptr)
		, m_del_flag(false)
		, m_callback(callback)
	{
		update();
	}

    ~__TimerTask() {
        if (m_callback) {
            delete m_callback;
        }
        if (m_ptrData) {
            delete (int*)m_ptrData; // ’‚¿Ô µœ÷≤ª∫√÷ª‘ –Ìª˘±æ¿‡–Õ
        }
    }

	void update()
	{
		m_expires = xcore::running_time() + XTimeSpan(0, m_interval_ms * 1000);
	}

	//void* operator new(std::size_t count)
	//{
	//	ASSERT(count >= 0);
	//	return xcore::memory_alloc((uint32)count);
	//}

	//void operator delete(void* p)
	//{
	//	xcore::memory_free(p);
	//}
};

///////////////////////////////////////////////////////////////////////////////
// class XTimer::XTimerImpl
////////////////////////////////////////////////////////////////////////////////
class XTimer::XTimerImpl : public XRunnable
{
public:
    XTimerImpl(){}

	XTimerImpl(XTimer* pTimer);

	virtual ~XTimerImpl() {
        stop();
    }

	uint32 schedule(XTimer::ICallBack* callback, uint32 interval_ms, int repeat = 1, void* ptr = NULL);

	uint32 cancel(uint32 id);

	uint32 cancel(void* ptr);

	int32  next_timed();

    void   entry();

	virtual void   run(XThread* pThread) {
        entry();
    }

    bool   stop();

private:
	bool _once_run();

	void _adjust_front();

	void _del_index(void* task, void* ptr);

private:
	typedef multimap<XTimeSpan, void*> Table;
	typedef multimap<XTimeSpan, void*>::iterator Iter;
	typedef multimap<void*, void*> Index;
	typedef multimap<void*, void*>::iterator IdxIter;
	Table            m_tasks;
	Index            m_indexs;
	XCritical        m_lock;
	XCritical        m_lock_curr;
	volatile uint32  m_curr_id;
	XTimer*          m_pTimer;
    bool             isstop;
    XSemaphore       xsem;
    bool             is_running;
};

XTimer::XTimerImpl::XTimerImpl(XTimer* pTimer)
	: m_curr_id(0)
	, m_pTimer(pTimer)
{
	ASSERT(m_pTimer);
    isstop = false;
    is_running = false;
}

bool XTimer::XTimerImpl::stop() {
    isstop = true;
	m_lock.lock();
	m_lock_curr.lock();
	for (Iter it = m_tasks.begin(); it != m_tasks.end(); ++it)
	{
		xsem.post();
	}
	xsem.post();
	for (Iter it = m_tasks.begin(); it != m_tasks.end(); ++it)
	{
		delete (__TimerTask*)it->second;
	}
	m_tasks.clear();
	m_indexs.clear();
	m_lock_curr.unlock();
	m_lock.unlock();
    while (is_running) {
		xsem.post();
        usleep(1000);
    }
    return true;
}

uint32 XTimer::XTimerImpl::schedule(XTimer::ICallBack* callback, uint32 interval_ms, int repeat, void* ptr)
{
	ASSERT(callback);
	ASSERT(repeat != 0);
	if (repeat > 1 && interval_ms == 0) interval_ms = 1;

	__TimerTask* task_ = new __TimerTask(callback, interval_ms, repeat, ptr);
	ASSERT(task_);

	XLockGuard<XCritical> lock(m_lock);  // auto lock
	m_tasks.insert(make_pair(task_->m_expires, task_));
	if (ptr) m_indexs.insert(make_pair(ptr, task_));
    xsem.post();
	return task_->m_id;
}

uint32 XTimer::XTimerImpl::cancel(uint32 id)
{
	uint32 count = 0;
	if (id == 0) return 0;

	m_lock.lock();
	for (Iter it = m_tasks.begin(); it != m_tasks.end(); ++it)
	{
		__TimerTask* task_ = (__TimerTask*)it->second;
		if (task_->m_id == id)
		{
			if (!task_->m_del_flag)
			{
				_del_index(task_, task_->m_ptrData);
				task_->m_del_flag = true;
				count++;
			}
			break;
		}
	}
	m_lock.unlock();

	if (m_curr_id == id)
	{
		m_lock_curr.lock();  // wait on_timer() finished.
		m_lock_curr.unlock();
	}
    xsem.post();
	return count;
}

uint32 XTimer::XTimerImpl::cancel(void* ptr)
{
	uint32 count = 0;
	bool bWait = false;
	IdxIter begin_, end_, tmp_;
	if (ptr == NULL) return 0;

	m_lock.lock();  // lock
	begin_ = m_indexs.lower_bound(ptr);
	end_   = m_indexs.upper_bound(ptr);
	for (tmp_ = begin_; tmp_ != end_; ++tmp_)
	{
		__TimerTask* task_ = (__TimerTask*)tmp_->second;
		if (m_curr_id == task_->m_id) bWait = true;
		ASSERT(!task_->m_del_flag);
		task_->m_del_flag = true;
		count++;
	}
	m_indexs.erase(begin_, end_);
	m_lock.unlock();  // unlock

	if (bWait)
	{
		m_lock_curr.lock();  // wait on_timer() finished.
		m_lock_curr.unlock();
	}
    xsem.post();
	return count;
}

int32 XTimer::XTimerImpl::next_timed()
{
	XLockGuard<XCritical> lock(m_lock);
	if (m_tasks.empty()) return -1;
	XTimeSpan now = xcore::running_time();
	__TimerTask* task_ = (__TimerTask*)m_tasks.begin()->second;
	if (task_->m_expires <= now) return 0;
	return (uint32)(task_->m_expires - now).to_msecs();
}

void XTimer::XTimerImpl::entry()
{
    is_running = true;
	while (!isstop)
	{
        _once_run();
	}
    is_running = false;
}

bool XTimer::XTimerImpl::_once_run()
{
	__TimerTask* task_ = NULL;
	bool bRet = false;
	XTimeSpan now = xcore::running_time();

	m_lock.lock();
    int64 time_to_wait = 0;
    bool isempty = false;
	if (!m_tasks.empty())
	{
		task_ = (__TimerTask*)m_tasks.begin()->second;
        if (!task_) {
        }
        else if (task_->m_del_flag)
		{
			m_tasks.erase(m_tasks.begin());
			delete task_;
			task_ = NULL;
			bRet = true;
		}
		else if (task_->m_repeat == 0)
		{
			_del_index(task_, task_->m_ptrData);
			m_tasks.erase(m_tasks.begin());
			delete task_;
			task_ = NULL;
			bRet = true;
		}
		else if (task_->m_expires > now)
		{
            time_to_wait = task_->m_expires.to_msecs() - now.to_msecs();
			task_ = NULL;
			bRet = false;
		}
		else
		{
			_adjust_front();
			m_lock_curr.lock();
			m_curr_id = task_->m_id;
			bRet = true;
		}
	}
    else {
        isempty = true;
    }
	m_lock.unlock();

    if (isempty) {
        xsem.wait();
        return bRet;
    }

    if (time_to_wait > 0) {
        xsem.trywait((uint32)time_to_wait);
        return bRet;
    }

	if (task_ == NULL) {
        return bRet;
    }

    if (!isstop)
        task_->m_callback->on_timer(m_pTimer, task_->m_id, task_->m_ptrData);

	m_curr_id = 0;
	m_lock_curr.unlock();

	return bRet;
}

void XTimer::XTimerImpl::_adjust_front()
{
	__TimerTask* task_ = (__TimerTask*)m_tasks.begin()->second;
	if (task_->m_repeat > 0) task_->m_repeat--;
	if (task_->m_repeat <= 0) return;
	m_tasks.erase(m_tasks.begin());
	task_->update();
	m_tasks.insert(make_pair(task_->m_expires, task_));
	return;
}

void XTimer::XTimerImpl::_del_index(void* task, void* ptr)
{
	if (ptr == NULL) return;
	IdxIter begin_ = m_indexs.lower_bound(ptr);
	IdxIter end_   = m_indexs.upper_bound(ptr);
	for ( ; begin_ != end_; ++begin_)
	{
		if (task == begin_->second)
		{
			m_indexs.erase(begin_);
			break;
		}
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////
// class XTimer
////////////////////////////////////////////////////////////////////////////////
XTimer::XTimer()
	: m_impl(new(nothrow) XTimerImpl(this))
{
}

XTimer::~XTimer()
{
    delete m_impl;
}

uint32 XTimer::schedule(ICallBack* callback, uint32 interval_ms, int repeat, void* ptr)
{
	return m_impl->schedule(callback, interval_ms, repeat, ptr);
}

uint32 XTimer::cancel(uint32 id)
{
	return m_impl->cancel(id);
}

uint32 XTimer::cancel(void* ptr)
{
	return m_impl->cancel(ptr);
}

bool XTimer::start() {
    thread_.start(m_impl);
    return true;
}

bool XTimer::stop() {
    m_impl->stop();
    thread_.stop();
	thread_.join();
    return true;
}

int32 XTimer::next_timed()
{
	return m_impl->next_timed();
}

XTimer XTimer::StaticTimer;

}//namespace xcore


// 20010-09-29
// xcore_time_span.cpp
// 

namespace xcore {
	
///////////////////////////////////////////////////////////////////////////////
// class XTimeSpan
////////////////////////////////////////////////////////////////////////////////
XTimeSpan::XTimeSpan()
	: m_sec(0)
	, m_usec(0)
{
	// empty
}

XTimeSpan::XTimeSpan(const XTimeSpan& ts)
{
	m_sec = ts.m_sec;
	m_usec = ts.m_usec;
}

XTimeSpan::XTimeSpan(int64 sec, int64 usec)
{
	set(sec, usec);
}

XTimeSpan::XTimeSpan(const timeval& tv)
{
	set(tv.tv_sec, tv.tv_usec);
}

XTimeSpan::XTimeSpan(double d)
{
	set(d);
}

XTimeSpan::~XTimeSpan()
{
	// empty
}

XTimeSpan& XTimeSpan::set(int64 sec, int64 usec)
{
	m_sec = sec;
	m_usec = usec;
	_normalize();
	return *this;
}

XTimeSpan& XTimeSpan::set(double d)
{
	m_sec = (int64)d;
	if (d > 0.0)
		m_usec = (int64)((d - (double)m_sec) * 1000000.0 + 0.5);
	else
		m_usec = (int64)((d - (double)m_sec) * 1000000.0 - 0.5);
	_normalize();
	return *this;
}

XTimeSpan& XTimeSpan::set(const timeval& tv)
{
	set(tv.tv_sec, tv.tv_usec);
	return *this;
}

XTimeSpan& XTimeSpan::from_usecs(int64 usec)
{
	m_sec = usec / 1000000;
	m_usec = usec % 1000000;
	return *this;
}

XTimeSpan& XTimeSpan::from_msecs(int64 msec)
{
	m_sec = msec / 1000;
	m_usec = (msec % 1000) * 1000;
	return *this;
}

int64 XTimeSpan::to_usecs() const
{
	return m_sec * 1000000 + m_usec;
}

int64 XTimeSpan::to_msecs() const
{
	int64 msec = m_sec;
	msec *= 1000;
	if (m_usec > 0)
		msec += ((m_usec + 500) / 1000);
	else
		msec += ((m_usec - 500) / 1000);
	return msec;
}

int64 XTimeSpan::to_secs() const
{
	int64 sec = m_sec;
	if (m_usec > 0)
		sec += ((m_usec + 500000) / 1000000);
	else
		sec += ((m_usec - 500000) / 1000000);
	return sec;
}

string XTimeSpan::to_str() const
{
	char buf[64];
	sprintf(buf, "%.6f", (double)*this);
	return buf;
}

XTimeSpan::operator double() const
{
	double d = (double)m_usec;
	d /= 1000000.0;
	d += (double)m_sec;
	return d;
}

XTimeSpan& XTimeSpan::operator += (const XTimeSpan& t)
{
	this->set(this->m_sec + t.m_sec, this->m_usec + t.m_usec);
	return *this;
}

XTimeSpan& XTimeSpan::operator -= (const XTimeSpan& t)
{
	this->set(this->m_sec - t.m_sec, this->m_usec - t.m_usec);
	return *this;
}

XTimeSpan& XTimeSpan::operator = (const XTimeSpan& t)
{
	if (this != &t)
	{
		m_sec = t.m_sec;
		m_usec = t.m_usec;
	}
	return *this;
}

void XTimeSpan::_normalize()
{
	m_sec += m_usec / 1000000;
	m_usec %= 1000000;
	if (m_sec > 0 && m_usec < 0)
	{
		m_sec--;
		m_usec += 1000000;
	}
	else if (m_sec < 0 && m_usec > 0)
	{
		m_sec++;
		m_usec -= 1000000;
	}
	return;
}


 bool operator < (const XTimeSpan& t1, const XTimeSpan& t2)
 {
 	return ((t1.m_sec < t2.m_sec) ||
 			((t1.m_sec == t2.m_sec) && (t1.m_usec < t2.m_usec)));
 }

bool operator > (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec > t2.m_sec) ||
			((t1.m_sec == t2.m_sec) && (t1.m_usec > t2.m_usec)));
}

bool operator <= (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec < t2.m_sec) ||
			((t1.m_sec == t2.m_sec) && (t1.m_usec <= t2.m_usec)));
}

bool operator >= (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec > t2.m_sec) ||
			((t1.m_sec == t2.m_sec) && (t1.m_usec >= t2.m_usec)));
}

bool operator == (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec == t2.m_sec) && (t1.m_sec == t2.m_sec));
}

bool operator != (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec != t2.m_sec) || (t1.m_sec != t2.m_sec));
}

XTimeSpan operator + (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return XTimeSpan(t1.m_sec + t2.m_sec, t1.m_usec + t2.m_usec);
}

XTimeSpan operator - (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return XTimeSpan(t1.m_sec - t2.m_sec, t1.m_usec - t2.m_usec);
}

XTimeSpan operator * (double d, const XTimeSpan& t)
{
	return XTimeSpan((double)t * d);
}

XTimeSpan operator * (const XTimeSpan& t, double d)
{
	return XTimeSpan((double)t * d);
}

XTimeSpan operator / (const XTimeSpan& t, double d)
{
	ASSERT(!(d >= -0.0001 && d <= 0.0001));
	return XTimeSpan((double)t / d);
}

}//namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

//#ifdef _XCORE_NEED_TEST
//
//#include "xcore_test.h"
//
//namespace xcore
//{
//
//bool xcore_test_time_span()
//{
//	timeval val = {23, 34};
//	XTimeSpan ts1;
//	XTimeSpan ts2(100, 340);
//	XTimeSpan ts3(ts2);
//	XTimeSpan ts4((double)ts2);
//	XTimeSpan ts5(val);
//	ASSERT(ts1.to_str() == "0.000000");
//	ASSERT(ts2.to_str() == "100.000340");
//	ASSERT(ts3.to_str() == "100.000340");
//	ASSERT(ts4.to_str() == "100.000340");
//	ASSERT(ts5.to_str() == "23.000034");
//
//	ts1.set(234, -574924455);
//	ASSERT(ts1.to_str() == "-340.924455");
//	ts1.set(val);
//	ASSERT(ts1.to_str() == "23.000034");
//	ts1.set(345.52462);
//	ASSERT(ts1.to_str() == "345.524620");
//	ts1.set(345.52462378);
//	ASSERT(ts1.to_str() == "345.524624");
//	printf("ts1 is %.6f\n", (double)ts1);
//
//	ASSERT(ts1.sec() == 345);
//	ASSERT(ts1.usec() == 524624);
//	ASSERT(ts1.to_secs() == 346);
//	ASSERT(ts1.to_msecs() == 345525);
//	ASSERT(ts1.to_usecs() == 345524624);
//	ASSERT(ts1.to_msecs() == ts2.from_msecs(ts1.to_msecs()).to_msecs());
//	ASSERT(ts1.to_usecs() == ts2.from_usecs(ts1.to_usecs()).to_usecs());
//
//	return true;
//}
//
//}//namespace xcore
//
//#endif//_XCORE_NEED_TEST
