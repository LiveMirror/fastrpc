// 2008-02-23
// xcore_mutex.cpp
// 
// »¥³âËø(²»ÔÊÐíµÝ¹éËø)


#include "xcore_mutex.h"
#include "xcore_thread.h"

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


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_mutex()
{
	XMutex mutex_;
	mutex_.lock();
	mutex_.unlock();
	VERIFY(mutex_.trylock());
	mutex_.unlock();
	//mutex_.unlock();
	mutex_.lock();
	//mutex_.lock();
	mutex_.unlock();
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
