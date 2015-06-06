// 2008-02-23
// xcore_event.cpp
// 
// windows下的事件概念


#include "xcore_event.h"

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


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_event()
{
	bool bl = false;
	XEvent event1;
	bl = event1.trywait(100);
	ASSERT(bl == false);
	event1.set();
	event1.wait();
	event1.set();
	bl = event1.trywait(100);
	ASSERT(bl == true);
	bl = event1.trywait(100);
	ASSERT(bl == false);
	event1.set();
	bl = event1.trywait(100);
	ASSERT(bl == true);

	XEvent event2(true);
	bl = event2.trywait(100);
	ASSERT(bl == false);
	event2.set();
	event2.wait();
	event2.wait();
	bl = event2.trywait(100);
	ASSERT(bl == true);
	event2.reset();
	bl = event2.trywait(100);
	ASSERT(bl == false);
	event2.set();
	bl = event2.trywait(100);
	ASSERT(bl == true);

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
