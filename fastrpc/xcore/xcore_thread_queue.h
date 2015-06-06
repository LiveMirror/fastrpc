// 2012-04-05
// xcore_thread_queue.h
// 
// 任务队列


#ifndef _XCORE_THREAD_QUEUE_H_
#define _XCORE_THREAD_QUEUE_H_

#include "xcore_define.h"
#include "xcore_noncopyable.h"
#include "xcore_critical.h"
#include "xcore_event.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XThreadQueue
///////////////////////////////////////////////////////////////////////////////
template<typename TaskT_>
class XThreadQueue : public XNoncopyable
{
public:
	XThreadQueue(uint32 capacity_ = 0XFFFFFFFF)
		: m_capacity(capacity_)
		, m_exit(false)
	{
		ASSERT(m_capacity > 0);
	}

	virtual ~XThreadQueue()
	{
		// empty
	}

	bool put(const TaskT_& task_, int timeout_ms = -1)
	{
		m_lock.lock();
		while ((uint32)m_tasks.size() >= m_capacity)
		{
			m_lock.unlock();
			if (!m_notFull.trywait(timeout_ms) || m_exit) return false;
			m_lock.lock();
		}
		m_tasks.push_back(task_);
		m_lock.unlock();
		m_notEmpty.set();
		return true;
	}

	bool take(TaskT_& task_, int timeout_ms = -1)
	{
		m_lock.lock();
		while (m_tasks.empty())
		{
			m_lock.unlock();
			if (!m_notEmpty.trywait(timeout_ms) || m_exit) return false;
			m_lock.lock();
		}
		task_ = m_tasks.front();
		m_tasks.pop_front();
		m_lock.unlock();
		m_notFull.set();
		return true;
	}

	void notify_exit()
	{
		m_exit = true;
		m_notFull.set();
		m_notEmpty.set();
		return;
	}

	bool is_empty()
	{
		XLockGuard<XCritical> lock(m_lock);
		return m_tasks.empty();
	}

	bool is_full()
	{
		XLockGuard<XCritical> lock(m_lock);
		return (uint32)m_tasks.size() >= m_capacity;
	}

	uint32 size()
	{
		XLockGuard<XCritical> lock(m_lock);
		return (uint32)m_tasks.size();
	}

	uint32 capacity() const
	{
		return m_capacity;
	}

private:
	uint32        m_capacity;
	bool          m_exit;
	XCritical     m_lock;
	XEvent        m_notEmpty;
	XEvent        m_notFull;
	list<TaskT_>  m_tasks;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_THREAD_QUEUE_H_
