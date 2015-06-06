// 2011-06-16
// xcore_thread_array.cpp
// 
// 线程组实现


#include "xcore_thread_array.h"
#include "xcore_log.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XThreadArray
///////////////////////////////////////////////////////////////////////////////
XThreadArray::XThreadArray()
	: m_threads(NULL)
	, m_thread_count(0)
{
	// empty
}

XThreadArray::~XThreadArray()
{
	ASSERT(m_threads == NULL && m_thread_count == 0 && "Need to call join() before XThreadArray destruction.");
	stop();
	join();

	XRunnable* task = NULL;
	while (m_tasks.take(task, 0))
	{
		delete task;
	}
}

 bool XThreadArray::start(uint32 thread_count, uint32 stack_size)
 {
	 ASSERT(m_threads == NULL && "Thread array already start.");
	 if (m_threads) return true;
	 if (thread_count == 0) return false;
	 m_threads = new(nothrow)XThread[thread_count];
	 ASSERT(m_threads);
	 m_thread_count = 0;

	 for (uint32 i = 0; i < thread_count; i++)
	 {
		 if (!m_threads[i].start(this, stack_size))
		 {
			 this->stop();
			 this->join();
			 return false;
		 }
		 m_thread_count++;
	 }
	 return true;
 }

 void XThreadArray::stop()
 {
	 if (m_threads && m_thread_count > 0)
	 {
		 for (uint32 i = 0; i < m_thread_count; i++)
		 {
			 m_threads[i].stop();
		 }
	 }
	 return;
 }

 void XThreadArray::join()
 {
	 if (m_threads && m_thread_count > 0)
	 {
		 for (uint32 i = 0; i < m_thread_count; i++)
		 {
			 m_threads[i].join();
		 }
	 }
	 SAFE_DELETE_ARRAY(m_threads);
	 m_thread_count = 0;
	 return;
 }

 void XThreadArray::add_task(XRunnable* task)
 {
	 if (task == NULL) return;
	 m_tasks.put(task);
	 ++m_task_count;
 }

 uint32 XThreadArray::task_count_pending()
 {
	 return m_tasks.size();
 }

 uint32 XThreadArray::task_count_total()
 {
	 return m_task_count;
 }

 void XThreadArray::run(XThread* pThread)
 {
	 ASSERT(pThread);

	 XRunnable* task_ = NULL;
	 while (!pThread->wait_quit(0))
	 {
		 while (!pThread->wait_quit(0) && m_tasks.take(task_, 500))
		 {
			 try
			 {
				 task_->run(pThread);
			 }
			 catch (...)
			 {
				 XERROR("XThreadArray::run(), Task run throw exception.");
			 }
			 
			 delete task_;
			 --m_task_count;
		 }
	 }
 }

} // namespace xcore



////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

class ThreadArrayTask : public XRunnable
{
public:
	ThreadArrayTask() {}
	virtual ~ThreadArrayTask() {}

	virtual void run(XThread* pThread)
	{
		ASSERT(pThread);
		printf("ThreadArrayTask(thread-id:%d) start....\n", pThread->id());
		for (int i = 0; i < 5 && !pThread->wait_quit(0); i++)
		{
			printf("ThreadArrayTask(thread-id:%d) is %d.\n", pThread->id(), i);
			xcore::sleep(1000);
		}
		printf("ThreadArrayTask(thread-id:%d) stop...\n", pThread->id());
		return;
	}
};

bool xcore_test_thread_array()
{
	XThreadArray threadArray_;
	VERIFY(threadArray_.start(5));
	for (int i = 0; i < 13; i++)
	{
		threadArray_.add_task(new ThreadArrayTask);
	}
	while (threadArray_.task_count_total() > 0) xcore::sleep(500);
	threadArray_.stop();
	threadArray_.join();

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
