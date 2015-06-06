// 2011-06-16
// xcore_thread_pool.h
// 
// 线程池实现


#ifndef _XCORE_THREAD_POOL_H_
#define _XCORE_THREAD_POOL_H_

#include "xcore_define.h"
#include "xcore_thread.h"
#include "xcore_atomic.h"
#include "xcore_critical.h"
#include "xcore_thread_queue.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XThreadPool
////////////////////////////////////////////////////////////////////////////////
class XThreadPool : private XNoncopyable, public XRunnable
{
public:
	XThreadPool();

	virtual ~XThreadPool();

	virtual bool start(uint32 min_count = 1, uint32 max_count = 0XFFFF, uint32 stack_size = 0);

	virtual void stop();

	virtual void join();

	void add_task(XRunnable* task);

	uint32 task_count_pending();

	uint32 task_count_total();

	uint32 thread_count_active();

	uint32 thread_count_running();

	uint32 thread_count_total();

protected:
	virtual void run(XThread* pThread);

	virtual bool need_start_thread();

	virtual bool need_stop_thread();

private:
	XThreadQueue<XRunnable*> m_tasks;
	list<XThread *>   m_threads;
	XCritical         m_lock;
	uint32            m_stack_size;
	uint32            m_min_count;
	uint32            m_max_count;
	volatile uint32   m_query_count;
	XAtomic32           m_thread_count_total;
	XAtomic32           m_thread_count_running;
	XAtomic32           m_thread_count_active;
	XAtomic32           m_task_count_total;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_THREAD_POOL_H_
