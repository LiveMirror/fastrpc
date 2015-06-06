// 2008-02-23
// xcore_thread.h
// 
// 线程类实现


#ifndef _XCORE_THREAD_H_
#define _XCORE_THREAD_H_

#include "xcore_define.h"
#include "xcore_noncopyable.h"
#include "xcore_event.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// Interface XRunnable
///////////////////////////////////////////////////////////////////////////////
class XThread;
class XRunnable
{
public:
	virtual ~XRunnable() {}

	virtual void run(XThread* pThread) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// class XThread
///////////////////////////////////////////////////////////////////////////////
class XThread : private XNoncopyable, public XRunnable
{
public:
	typedef enum { CREATE, START, IDLE, RUNNING, STOP, JOINED } STATE;

	XThread();

	virtual ~XThread();

	virtual bool start(XRunnable* r = NULL, uint32 stack_size = 0);

	virtual void stop();

	virtual void join();

	virtual void kill();

	virtual bool wait_quit(uint32 msec = 0);

	uint64 id() { return m_id; }

	int state() { return m_state; }

protected:
	virtual void run(XThread* pThread) {};

	friend unsigned __stdcall invok_proc_win(void *arg);
	friend void* invok_proc_posix(void *arg);

protected:
	volatile uint64  m_id;     // thread id that be managed
	volatile int     m_state;
	XRunnable*       m_runnable;
	HANDLE		     m_handle; // use at windows platform only
	XEvent		     m_quit;   // manual reset event
};

///////////////////////////////////////////////////////////////////////////////

uint32 thread_id();

void sleep(uint32 msec);

} // namespace xcore

using namespace xcore;

#endif//_XCORE_THREAD_H_
