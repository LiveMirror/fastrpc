// 2008-02-23
// xcore_critical.cpp
// 
// 临界区类(允许递归锁)


#include "xcore_critical.h"

namespace xcore {

XCritical XCritical::StaticCritical;

///////////////////////////////////////////////////////////////////////////////
// class XCritical
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

class XCritical::XCriticalImpl
{
public:
	XCriticalImpl()
	{
		InitializeCriticalSection(&m_critical);
	}

	~XCriticalImpl()
	{
		DeleteCriticalSection(&m_critical);
	}

	void lock()
	{
		EnterCriticalSection(&m_critical);
	}

	bool trylock()
	{
		return !!TryEnterCriticalSection(&m_critical);
	}

	void unlock()
	{
		LeaveCriticalSection(&m_critical);
	}

private:
	CRITICAL_SECTION  m_critical;
};

#endif //__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

class XCritical::XCriticalImpl
{
public:
	XCriticalImpl()
	{
		pthread_mutexattr_t attr;
		VERIFY(!pthread_mutexattr_init(&attr));
		VERIFY(!pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)); // PTHREAD_MUTEX_RECURSIVE_NP
		VERIFY(!pthread_mutex_init(&m_mutex, &attr));
		VERIFY(!pthread_mutexattr_destroy(&attr));
	}

	~XCriticalImpl()
	{
		int ret = pthread_mutex_destroy(&m_mutex);
		ASSERT(!ret && "the mutex is currently locked.");
		ret = 0;
	}

	void lock()
	{
		int ret = pthread_mutex_lock(&m_mutex);
		ASSERT(!ret && "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
		ret = 0;
	}

	bool trylock()
	{
		int ret = pthread_mutex_trylock(&m_mutex);
		if (0 == ret) return true;
		if (errno == EAGAIN) return false;
		ASSERT(!"The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.");
		ret = 0;
	}

	void unlock()
	{
		int ret = pthread_mutex_unlock(&m_mutex);
		ASSERT(!ret && "The current thread does not own the mutex.");
		ret = 0;
	}

private:
	pthread_mutex_t  m_mutex;
};

#endif //__GNUC__

///////////////////////////////////////////////////////////////////////////////
XCritical::XCritical()
	: m_impl(new(nothrow) XCriticalImpl)
{
	// empty
}

XCritical::~XCritical()
{
}

void XCritical::lock()
{
	m_impl->lock();
}

bool XCritical::trylock()
{
	return m_impl->trylock();
}

void XCritical::unlock()
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

bool xcore_test_critical()
{
	XCritical* pCritical = new XCritical();
	delete pCritical;
	//XCritical c1;
	//XCritical c2(c1);
	//XCritical c3;
	//c3 = c1;

	XCritical lock;
	lock.lock();
	//lock.lock();
	lock.unlock();
	VERIFY(lock.trylock());
	lock.unlock();
	//lock.unlock();
	XCritical::StaticCritical.lock();
	XCritical::StaticCritical.unlock();

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
