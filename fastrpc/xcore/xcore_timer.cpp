// 2009-07-28
// xcore_timer.cpp
//
// 定时器实现


#include "xcore_timer.h"
#include "xcore_clock.h"
#include "xcore_atomic.h"
#include "xcore_critical.h"
#include "xcore_time_span.h"
#include "xcore_memory.h"
#include "xcore_semaphore.h"

namespace xcore {

static XAtomic32 XTimer_ID_Generater(0);

///////////////////////////////////////////////////////////////////////////////
// class __TimerTask
////////////////////////////////////////////////////////////////////////////////
class __TimerTask
{
public:
	uint32             m_id;
	uint32             m_interval_ms; // 执行间隔时间，单位：毫秒
	int                m_repeat;     // 执行计数，<0 永久重复执行，0 退出，>0 剩余执行次数
	void*              m_ptrData;
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
            delete m_ptrData;
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
};

XTimer::XTimerImpl::XTimerImpl(XTimer* pTimer)
	: m_curr_id(0)
	, m_pTimer(pTimer)
{
	ASSERT(m_pTimer);
    isstop = false;
}

bool XTimer::XTimerImpl::stop() {
    isstop = true;
	m_lock.lock();
	m_lock_curr.lock();
	for (Iter it = m_tasks.begin(); it != m_tasks.end(); ++it)
	{
		delete (__TimerTask*)it->second;
	}
	m_tasks.clear();
	m_indexs.clear();
	for (Iter it = m_tasks.begin(); it != m_tasks.end(); ++it)
	{
		xsem.post();
	}
	xsem.post();
	m_lock_curr.unlock();
	m_lock.unlock();
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
	while (!isstop)
	{
        _once_run();
	}
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

	if (task_ == NULL) return bRet;

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



////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_timer()
{
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
