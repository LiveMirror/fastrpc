// 2008-02-23
// xcore_critical.h
// 
// 临界区类(允许递归锁)


#ifndef _XCORE_CRITICAL_H_
#define _XCORE_CRITICAL_H_

#include "xcore_define.h"
#include "xcore_smart_ptr.h"
#include "xcore_noncopyable.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XCritical
///////////////////////////////////////////////////////////////////////////////
class XCritical
{
public:
	XCritical();

	~XCritical();

	static XCritical StaticCritical;

public:
	void lock();

	bool trylock();

	void unlock();

private:
	class XCriticalImpl;
	scoped_ptr<XCriticalImpl> m_impl;
};


///////////////////////////////////////////////////////////////////////////////
// XLockGuard
///////////////////////////////////////////////////////////////////////////////
template<class XLock, bool locked = false>
class XLockGuard : XNoncopyable
{
public:
	explicit XLockGuard(XLock& lock)
		: m_lock(lock)
	{
		if (!locked) m_lock.lock();
	}

	~XLockGuard()
	{
		m_lock.unlock();
	}

private:
	XLock& m_lock;
};


// Prevent misuse like:
// XLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define XLockGuard(x) error "Missing lock guard object name"

} // namespace xcore

using namespace xcore;

#endif//_XCORE_CRITICAL_H_
