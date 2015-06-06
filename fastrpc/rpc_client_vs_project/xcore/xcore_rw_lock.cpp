// 2010-05-02
// xcore_rw_lock.cpp
// 
// ¶ÁÐ´Ëø


#include "xcore_rw_lock.h"
#include "xcore_event.h"
#include "xcore_critical.h"
#include "xcore_thread.h"
#include "xcore_atomic.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// XRWLock
////////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

class XRWLock::XRWLockImpl
{
public:
	XRWLockImpl(PRIORITY_TYPE type)
		: m_readers(0)
		, m_type(type)
		, m_wr_waitting(false)
	{
		ASSERT((m_writeThreadID = 0, true));
		m_event.set();
	}

	~XRWLockImpl()
	{
		ASSERT(m_writeThreadID == 0 && "the read-write locker is currently locked.");
		ASSERT(total() == 0 && "the read-write locker is currently locked.");
	}

	void rd_lock()
	{
		XLockGuard<XCritical> lock_(m_lock_read);
		ASSERT(m_writeThreadID != xcore::thread_id() && "read-write locker can't recurrence lock.");
		ASSERT(count(xcore::thread_id()) == 0 && "read-write locker can't recurrence lock.");
		while (m_type == WRITE_PRIORITY && m_wr_waitting) xcore::sleep(0);
		while (m_readers == 0)
		{
			m_event.wait();
			if (m_type == WRITE_PRIORITY && m_wr_waitting)
			{
				m_event.set();
				xcore::sleep(0);
				continue;
			}
			break;
		}
		++m_readers;
		ASSERT((insert(xcore::thread_id()), true));
		return;
	}

	bool rd_trylock()
	{
		if (m_type == WRITE_PRIORITY && m_wr_waitting) return false;
		if (!m_lock_read.trylock()) return false;
		XLockGuard<XCritical, true> lock_(m_lock_read);
		ASSERT(m_writeThreadID != xcore::thread_id() && "read-write locker can't recurrence lock.");
		ASSERT(count(xcore::thread_id()) == 0 && "read-write locker can't recurrence lock.");
		if (m_readers > 0 || m_event.trywait(0))
		{
			++m_readers;
			ASSERT((insert(xcore::thread_id()), true));
			return true;
		}
		return false;
	}

	void rd_unlock()
	{
		ASSERT(count(xcore::thread_id()) > 0 && "the calling thread does not own the mutex.");
		ASSERT((erase(xcore::thread_id()), true));
		ASSERT(m_readers > 0);
		if ((--m_readers) == 0) m_event.set();
	}

	void wr_lock()
	{
		XLockGuard<XCritical> lock_(m_lock_write);
		ASSERT(m_writeThreadID != xcore::thread_id() && "read-write locker can't recurrence lock.");
		ASSERT(count(xcore::thread_id()) == 0 && "read-write locker can't recurrence lock.");
		m_wr_waitting = true;
		m_event.wait();
		m_wr_waitting = false;
		ASSERT((m_writeThreadID = xcore::thread_id(), true));
	}

	bool wr_trylock()
	{
		if (!m_lock_write.trylock()) return false;
		XLockGuard<XCritical, true> lock_(m_lock_write);
		ASSERT(m_writeThreadID != xcore::thread_id() && "read-write locker can't recurrence lock.");
		ASSERT(count(xcore::thread_id()) == 0 && "read-write locker can't recurrence lock.");
		m_wr_waitting = true;
		if (m_event.trywait(0))
		{
			ASSERT((m_writeThreadID = xcore::thread_id(), true));
			m_wr_waitting = false;
			return true;
		}
		m_wr_waitting = false;
		return false;
	}

	void wr_unlock()
	{
		ASSERT(m_writeThreadID == xcore::thread_id() && "the calling thread does not own the mutex.");
		ASSERT((m_writeThreadID = 0, true));
		m_event.set();
	}

private:
	XCritical m_lock_read;  // use to readers queue up
	XCritical m_lock_write;  // use to writers queue up
	XAtomic32   m_readers;
	XEvent    m_event;
	int       m_type;
	volatile bool m_wr_waitting;

	#ifdef __DEBUG__
	volatile uint32  m_writeThreadID;  // check write recurrence lock
	std::set<uint32> m_lockedThreadIDs;  // check read recurrence lock
	XCritical        m_lock_debug;

	uint32 count(uint32 id)
	{
		XLockGuard<XCritical> lock_(m_lock_debug);
		return (uint32)m_lockedThreadIDs.count(id);
	}
	uint32 total()
	{
		XLockGuard<XCritical> lock_(m_lock_debug);
		return (uint32)m_lockedThreadIDs.size();
	}
	void insert(uint32 id)
	{
		XLockGuard<XCritical> lock_(m_lock_debug);
		m_lockedThreadIDs.insert(id);
	}
	void erase(uint32 id)
	{
		XLockGuard<XCritical> lock_(m_lock_debug);
		m_lockedThreadIDs.erase(id);
	}
	#endif//__DEBUG__
};

#endif//__WINDOWS__

////////////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__

class XRWLock::XRWLockImpl
{
public:
	XRWLockImpl(int type)
		: m_type(type)
	{
		pthread_rwlockattr_t attr;
		VERIFY(!pthread_rwlockattr_init(&attr));
		if (type == READ_PRIORITY)
			VERIFY(!pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_READER_NP));
		else
			VERIFY(!pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP));
		VERIFY(!pthread_rwlock_init(&m_handle, &attr));
		VERIFY(!pthread_rwlockattr_destroy(&attr));
	}

	~XRWLockImpl()
	{
		VERIFY(!pthread_rwlock_destroy(&m_handle));
	}

	void rd_lock()
	{
		VERIFY(!pthread_rwlock_rdlock(&m_handle));
	}

	bool rd_trylock()
	{
		return (!pthread_rwlock_tryrdlock(&m_handle));
	}

	void rd_unlock()
	{
		VERIFY(!pthread_rwlock_unlock(&m_handle));
	}

	void wr_lock()
	{
		VERIFY(!pthread_rwlock_wrlock(&m_handle));
	}

	bool wr_trylock()
	{
		return (!pthread_rwlock_trywrlock(&m_handle));
	}

	void wr_unlock()
	{
		VERIFY(!pthread_rwlock_unlock(&m_handle));
	}

private:
	int              m_type;
	pthread_rwlock_t m_handle;
};

#endif//__GNUC__

////////////////////////////////////////////////////////////////////////////////
XRWLock::XRWLock(PRIORITY_TYPE type)
	: m_rlock(*this)
	, m_wlock(*this)
	, m_impl(new(nothrow) XRWLockImpl(type))
{
	ASSERT(type == WRITE_PRIORITY || type == READ_PRIORITY);
}

XRWLock::~XRWLock()
{
}


////////////////////////////////////////////////////////////////////////////////
// XRLock
////////////////////////////////////////////////////////////////////////////////
void XRLock::lock()
{
	m_rwlock.m_impl->rd_lock();
}

bool XRLock::trylock()
{
	return m_rwlock.m_impl->rd_trylock();
}

void XRLock::unlock()
{
	m_rwlock.m_impl->rd_unlock();
}

////////////////////////////////////////////////////////////////////////////////
// XWLock
////////////////////////////////////////////////////////////////////////////////
void XWLock::lock()
{
	m_rwlock.m_impl->wr_lock();
}

bool XWLock::trylock()
{
	return m_rwlock.m_impl->wr_trylock();
}

void XWLock::unlock()
{
	m_rwlock.m_impl->wr_unlock();
}

}//namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_rwlock()
{
	XRWLock rwlock_;
	rwlock_.rlock().lock();
	//rwlock_.rlock().lock();
	//VERIFY(rwlock_.rlock().trylock() == false);
	//rwlock_.wlock().lock();
	rwlock_.rlock().unlock();
	VERIFY(rwlock_.rlock().trylock());
	rwlock_.rlock().unlock();

	rwlock_.wlock().lock();
	//rwlock_.rlock().lock();
	//VERIFY(rwlock_.wlock().trylock() == false);
	rwlock_.wlock().unlock();
	VERIFY(rwlock_.wlock().trylock());
	rwlock_.wlock().unlock();

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
