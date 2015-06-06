// 2011-06-16
// xcore_thread_array.h
// 
// 线程组实现


#ifndef _XCORE_THREAD_ARRAY_H_
#define _XCORE_THREAD_ARRAY_H_

#include "xcore_define.h"
#include "xcore_thread.h"
#include "xcore_critical.h"
#include "xcore_event.h"
#include "xcore_atomic.h"
#include "xcore_thread_queue.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XThreadArray
///////////////////////////////////////////////////////////////////////////////
class XThreadArray : private XNoncopyable, public XRunnable
{
public:
	XThreadArray();

	virtual ~XThreadArray();

	virtual bool start(uint32 thread_count, uint32 stack_size = 0);

	virtual void stop();

	virtual void join();

	void add_task(XRunnable* task);

	uint32 task_count_pending();

	uint32 task_count_total();

protected:
	virtual void run(XThread* pThread);

private:
	XThread*                 m_threads;
	uint32                   m_thread_count;
	XAtomic32                  m_task_count;
	XThreadQueue<XRunnable*> m_tasks;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_THREAD_ARRAY_H_
