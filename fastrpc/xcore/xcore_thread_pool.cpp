// 2011-06-16
// xcore_thread_pool.cpp
// 
// 线程池实现


#include "xcore_thread_pool.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XThreadPool
////////////////////////////////////////////////////////////////////////////////
XThreadPool::XThreadPool()
	: m_stack_size(0)
	, m_min_count(1)
	, m_max_count(1)
	, m_query_count(0)
{
	// empty
}

XThreadPool::~XThreadPool()
{
	ASSERT(m_thread_count_total == 0 && "Need to call join() before XThreadPool destruction.");
	stop();
	join();

	XRunnable* task_ = NULL;
	while (!m_tasks.take(task_, 0))
	{
		delete task_;
	}
}

bool XThreadPool::start(uint32 min_count, uint32 max_count, uint32 stack_size)
{
	if (min_count == 0) min_count = 1;
	if (max_count == 0) max_count = 1;
	if (min_count > max_count) min_count = max_count;
	m_min_count = min_count;
	m_max_count = max_count;
	m_stack_size = stack_size;

	for (uint32 i = 0; i < m_min_count; i++)
	{
		XThread* thread_ = new XThread;
		ASSERT(thread_);
		if (!thread_->start(this, stack_size))
		{
			delete thread_;
			thread_ = NULL;

			this->stop();
			this->join();
			--m_thread_count_total;
			return false;
		}
		m_lock.lock();
		m_threads.push_back(thread_);
		m_lock.unlock();
		++m_thread_count_total;
	}
	return true;
}

void XThreadPool::stop()
{
	XLockGuard<XCritical> lock(m_lock);

	for (list<XThread *>::iterator it = m_threads.begin(); it != m_threads.end(); ++it)
	{
		XThread* thread_ = *it;
		thread_->stop();
	}
	m_tasks.notify_exit();
}

void XThreadPool::join()
{
	while (true)
	{
		XThread* thread_ = NULL;
		m_lock.lock();
		if (m_threads.size() > 0)
		{
			thread_ = m_threads.front();
			m_threads.pop_front();
		}
		m_lock.unlock();
		if (thread_ == NULL) break;
		thread_->join();
		delete thread_;
		--m_thread_count_total;
	}
	return;
}

void XThreadPool::add_task(XRunnable* task)
{
	if (task == NULL) return;

	m_tasks.put(task);
	++m_task_count_total;

	if (need_start_thread())
	{
		XThread* thread_ = new XThread;
		ASSERT(thread_);
		VERIFY(thread_->start(this, m_stack_size));
		m_lock.lock();
		m_threads.push_back(thread_);
		m_lock.unlock();
		++m_thread_count_total;
	}
}

uint32 XThreadPool::task_count_pending()
{
	XLockGuard<XCritical> lock(m_lock);
	return (uint32)m_tasks.size();
}

uint32 XThreadPool::task_count_total()
{
	return m_task_count_total;
}

uint32 XThreadPool::thread_count_active()
{
	return m_thread_count_active;
}

uint32 XThreadPool::thread_count_running()
{
	return m_thread_count_running;
}

uint32 XThreadPool::thread_count_total()
{
	return m_thread_count_total;
}

void XThreadPool::run(XThread* pThread)
{
	ASSERT(pThread);

	++m_thread_count_running;

	while (!pThread->wait_quit(0))
	{
		XRunnable* task_ = NULL;
		while (!pThread->wait_quit(0) && m_tasks.take(task_, 500))
		{
			++m_thread_count_active;
			try
			{
				task_->run(pThread);
			}
			catch (...)
			{
				ASSERT(!"XThreadPool::run(), Task run throw exception.");
			}
			delete task_;
			--m_task_count_total;
			--m_thread_count_active;
		}

		// clean dead threads
		list<XThread *> dead_threads;
		m_lock.lock();
		list<XThread *>::iterator it = m_threads.begin();
		while (it != m_threads.end())
		{
			XThread* thread_ = *it;
			if (thread_->state() != XThread::STOP)
			{
				++it;
				continue;
			}
			dead_threads.push_back(thread_);
			it = m_threads.erase(it);
		}
		m_lock.unlock();
		while (!dead_threads.empty())
		{
			XThread* thread_ = dead_threads.front();
			dead_threads.pop_front();
			thread_->stop();
			thread_->join();
			delete thread_;
			--m_thread_count_total;
		}
		
		if (need_stop_thread())
		{
			break;
		}
	}

	--m_thread_count_running;
	return;
}

bool XThreadPool::need_start_thread()
{
	return ((m_task_count_total > m_thread_count_total) && (m_max_count > (uint32)m_thread_count_total));
}

bool XThreadPool::need_stop_thread()
{
	if (++m_query_count % 20 != 0) return false;
	return ((m_task_count_total < m_thread_count_running) && (m_min_count < (uint32)m_thread_count_running));
}

} // namespace xcore



////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

class ThreadPoolTask : public XRunnable
{
public:
	ThreadPoolTask() {}
	virtual ~ThreadPoolTask() {}

	virtual void run(XThread* pThread)
	{
		ASSERT(pThread);
		printf("ThreadPoolTask(thread-id:%d) start....\n", pThread->id());
		for (int i = 0; i < 5 && !pThread->wait_quit(0); i++)
		{
			printf("ThreadPoolTask(thread-id:%d) is %d.\n", pThread->id(), i);
			xcore::sleep(5000);
		}
		printf("ThreadPoolTask(thread-id:%d) stop...\n", pThread->id());
		return;
	}
};

bool xcore_test_thread_pool()
{
	XThreadPool pool_;
	printf("thread pool start.\n");
	bool bl = pool_.start(10, 30);
	ASSERT(bl);
	for (int i = 0; i < 50; i++)
	{
		pool_.add_task(new ThreadPoolTask);
		printf("thread pool task:%d, thread total:%d, running:%d, active:%d\n", 
			pool_.task_count_total(), pool_.thread_count_total(), pool_.thread_count_running(), pool_.thread_count_active());
	}
	while (pool_.task_count_total() > 0)
	{
		printf("thread pool task:%d, thread total:%d, running:%d, active:%d\n", 
			   pool_.task_count_total(), pool_.thread_count_total(), pool_.thread_count_running(), pool_.thread_count_active());
		xcore::sleep(500);
	}
	for (int i = 0; i < 20; i++)
	{
		printf("thread pool task:%d, thread total:%d, running:%d, active:%d\n", 
			pool_.task_count_total(), pool_.thread_count_total(), pool_.thread_count_running(), pool_.thread_count_active());
		xcore::sleep(500);
	}
	pool_.stop();
	pool_.join();
	printf("thread pool finished.\n");
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
