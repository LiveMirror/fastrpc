// 2010-05-02
// xcore_rw_lock.h
// 
// ¶ÁÐ´Ëø


#ifndef _XCORE_RW_LOCK_H_
#define _XCORE_RW_LOCK_H_

#include "xcore_define.h"
#include "xcore_noncopyable.h"
#include "xcore_smart_ptr.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XRWLock
////////////////////////////////////////////////////////////////////////////////
class XRWLock
{
public:
	typedef enum { WRITE_PRIORITY, READ_PRIORITY } PRIORITY_TYPE;

	class XRLock_ : XNoncopyable
	{
	public:
		explicit XRLock_(XRWLock& rwlock) : m_rwlock(rwlock) {}

		~XRLock_() {}

		void lock();

		bool trylock();

		void unlock();

	private:
		XRWLock&  m_rwlock;
	};

	class XWLock_ : XNoncopyable
	{
	public:
		explicit XWLock_(XRWLock& rwlock) : m_rwlock(rwlock) {}

		~XWLock_() {}

		void lock();

		bool trylock();

		void unlock();

	private:
		XRWLock&  m_rwlock;
	};

public:
	explicit XRWLock(PRIORITY_TYPE type = WRITE_PRIORITY);
	~XRWLock();

	XRLock_&  rlock() { return m_rlock; }
	XWLock_&  wlock() { return m_wlock; }

private:
	friend class XRLock_;
	friend class XWLock_;
	class XRWLockImpl;

	XRLock_  m_rlock;
	XWLock_  m_wlock;
	scoped_ptr<XRWLockImpl> m_impl;
};

typedef XRWLock::XRLock_ XRLock;
typedef XRWLock::XWLock_ XWLock;

}//namespace xcore

using namespace xcore;

#endif//_XCORE_RW_LOCK_H_
