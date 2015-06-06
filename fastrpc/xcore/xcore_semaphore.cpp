// 2008-02-27
// xcore_semaphore.h
//
// ÐÅºÅÁ¿Àà


#include "xcore_semaphore.h"

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

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_semaphore()
{
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
