// 2013-05-22
// common define for rpc
#include "tconnd_xcore_define.h"
#include "tconnd_closure.h"

#include <deque>


class XcoreMgr;
class CroTimer;
class TimerMgr;
class ClosureMap;
class PFMutMap;
class HookMgr;

namespace xcore {
class XAtomic32;
class XAtomic64;
class XClock;
class XCritical;
class XEvent;
class IXBuffer;
class XMutex;
class XNoncopyable;
class XSemaphore;
class XSockAddr;
class XSocket;
class XStrUtil;
class XThread;
class XRunnable;
class XThread;
class XTimer;
class XTimeSpan;
} // namespace xcore

using namespace xcore;

// 2010-12-02
// xcore_noncopyable.h
// 

#ifndef _XCORE_NONCOPYABLE_H_
#define _XCORE_NONCOPYABLE_H_

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// interface XNoncopyable
///////////////////////////////////////////////////////////////////////////////
class XNoncopyable
{
protected:
	XNoncopyable() {}
	~XNoncopyable() {}

private:
	XNoncopyable(const XNoncopyable&);
	XNoncopyable& operator=(const XNoncopyable&);
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_NONCOPYABLE_H_

// 2009-07-27
// xcore_atomic.h
// 


#ifndef _XCORE_ATOMIC_H_
#define _XCORE_ATOMIC_H_

#include "tconnd_xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XAtomic32
///////////////////////////////////////////////////////////////////////////////
class XAtomic32
{
public:
	XAtomic32(int32 i = 0);
	XAtomic32(const XAtomic32& from);
	~XAtomic32();
	
	int32 get_value() const;
	int32 set_value(int32 i);
	int32 test_zero_inc();

	operator int32();
	XAtomic32& operator= (int32 i);
	XAtomic32& operator= (const XAtomic32& from);

	int32 operator+= (int32 i);
	int32 operator-= (int32 i);
	int32 operator++ (int);
	int32 operator-- (int);
	int32 operator++ ();
	int32 operator-- ();

private:
	volatile int32 m_counter;
};


///////////////////////////////////////////////////////////////////////////////
// class XAtomic64
///////////////////////////////////////////////////////////////////////////////
class XAtomic64
{
public:
	XAtomic64(int64 i = 0);
	XAtomic64(const XAtomic64& from);
	~XAtomic64();

	int64 get_value() const;
	int64 set_value(int64 i);
	int64 test_zero_inc();

	operator int64();
	XAtomic64& operator= (int64 i);
	XAtomic64& operator= (const XAtomic64& from);

	int64 operator+= (int64 i);
	int64 operator-= (int64 i);
	int64 operator++ (int);
	int64 operator-- (int);
	int64 operator++ ();
	int64 operator-- ();

private:
	volatile int64 m_counter;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_ATOMIC_H_

// 2012-04-08
// xcore_smart_ptr.h
// 

#ifndef _XCORE_SMART_PTR_H_
#define _XCORE_SMART_PTR_H_

#include "tconnd_xcore_define.h"

#include <iostream>     // for std::ostream
#include <memory>       // for std::auto_ptr
#include <cstddef>      // for std::ptrdiff_t
#include <algorithm>    // for std::swap
#include <functional>   // for std::less
#include <typeinfo>     // for std::bad_cast

namespace xcore {

template<typename T> class scoped_ptr;
template<typename T> class scoped_array;
template<typename T> class shared_ptr;
template<typename T> class shared_array;
template<typename T> class weak_ptr;

namespace detail {

class weak_count;
class shared_count;

struct static_cast_tag {};
struct const_cast_tag {};
struct dynamic_cast_tag {};
struct polymorphic_cast_tag {};

///////////////////////////////////////////////////////////////////////////////
// class smart_counted_base
///////////////////////////////////////////////////////////////////////////////
class smart_counted_base
{
private:
	XAtomic32 m_use_count;
	XAtomic32 m_weak_count;

	smart_counted_base(smart_counted_base&);
	smart_counted_base& operator=(const smart_counted_base&);

protected:
	virtual ~smart_counted_base()
	{
		// empty
	}

public:
	smart_counted_base()
		: m_use_count(1)
		, m_weak_count(1)
	{
		// empty
	}

	virtual void dispose() = 0;

	void add_ref_copy()
	{
		++m_use_count;
	}

	bool add_ref_lock()
	{
		//if (m_use_count == 0) return false;
		//m_use_count++;
		//return true;
		return (m_use_count.test_zero_inc() != 0);
	}

	void release()
	{
		if (--m_use_count == 0)
		{
			dispose();
			weak_release();
		}
	}

	void weak_add_ref()
	{
		++m_weak_count;
	}

	void weak_release()
	{
		if (--m_weak_count == 0)
		{
			delete this;
		}
	}

	long use_count() const
	{
		return m_use_count.get_value();
	}
};

///////////////////////////////////////////////////////////////////////////////
// class smart_counted_base_impl
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename D = checked_deleter<T> >
class smart_counted_base_impl : public smart_counted_base
{
private:
	T* m_ptr;
	D  m_deleter;

	typedef smart_counted_base_impl<T, D> this_type;

	smart_counted_base_impl(const smart_counted_base_impl&);
	smart_counted_base_impl& operator= (const smart_counted_base_impl&);

public:
	smart_counted_base_impl(T* p, D d = checked_deleter<T>())
		: smart_counted_base()
		, m_ptr(p)
		, m_deleter(d)
	{
		// empty
	}

	virtual void dispose()
 	{
		if (m_ptr)
		{
 			m_deleter(m_ptr);
			m_ptr = NULL;
		}
 	}

	void * operator new(std::size_t)
	{
		return std::allocator<this_type>().allocate(1, static_cast<this_type *>(0));
	}

	void operator delete(void * p)
	{
		std::allocator<this_type>().deallocate(static_cast<this_type *>(p), 1);
	}
};

///////////////////////////////////////////////////////////////////////////////
// class shared_count
///////////////////////////////////////////////////////////////////////////////
class shared_count
{
private:
	smart_counted_base* m_pi;

	friend class weak_count;

public:
	shared_count()
		: m_pi(NULL)
	{
		// empty
	}

	template<typename Y>
	explicit shared_count(Y* p)
	{
		m_pi = new smart_counted_base_impl<Y>(p);
		ASSERT(m_pi);
	}

	template<typename T, typename D>
	shared_count(T* p, D d)
	{
		m_pi = new smart_counted_base_impl<T, D>(p, d);
		ASSERT(m_pi);
	}

	template<typename Y>
	explicit shared_count(std::auto_ptr<Y>& r)
	{
		m_pi = new smart_counted_base_impl<Y>(r.get());
		ASSERT(m_pi);
		r.release();
	}

	~shared_count()
	{
		if (m_pi) m_pi->release();
	}

	shared_count(const shared_count& r)
		: m_pi(r.m_pi)
	{
		if (m_pi) m_pi->add_ref_copy();
	}

	explicit shared_count(const weak_count& r);

	shared_count& operator= (const shared_count& r)
	{
		smart_counted_base* tmp = r.m_pi;
		
		if (tmp != m_pi)
		{
			if (tmp) tmp->add_ref_copy();
			if (m_pi) m_pi->release();
			m_pi = tmp;
		}
		return *this;
	}

	void swap(shared_count& r)
	{
		smart_counted_base* tmp = r.m_pi;
		r.m_pi = m_pi;
		m_pi = tmp;
	}

	long use_count() const
	{
		return m_pi ? m_pi->use_count() : 0;
	}

	bool unique() const
	{
		return use_count() == 1;
	}

	bool empty() const
	{
		return m_pi == NULL;
	}

	friend inline bool operator== (const shared_count& a, const shared_count& b)
	{
		return a.m_pi == b.m_pi;
	}

	friend inline bool operator< (const shared_count& a, const shared_count& b)
	{
		return std::less<smart_counted_base *>()(a.m_pi, b.m_pi);
	}
};

///////////////////////////////////////////////////////////////////////////////
// class weak_count
///////////////////////////////////////////////////////////////////////////////
class weak_count
{
private:
	smart_counted_base* m_pi;

	friend class shared_count;

public:
	weak_count()
		: m_pi(NULL)
	{
		// empty
	}

	weak_count(const shared_count& r)
		: m_pi(r.m_pi)
	{
		if (m_pi) m_pi->weak_add_ref();
	}

	weak_count(const weak_count& r)
		: m_pi(r.m_pi)
	{
		if (m_pi) m_pi->weak_add_ref();
	}

	~weak_count()
	{
		if (m_pi) m_pi->weak_release();
	}

	weak_count& operator= (const shared_count& r)
	{
		smart_counted_base* tmp = r.m_pi;

		if (tmp != m_pi)
		{
			if (tmp) tmp->weak_add_ref();
			if (m_pi) m_pi->weak_release();
			m_pi = tmp;
		}
		return *this;
	}

	weak_count& operator= (const weak_count&  r)
	{
		smart_counted_base* tmp = r.m_pi;

		if (tmp != m_pi)
		{
			if (tmp) tmp->weak_add_ref();
			if (m_pi) m_pi->weak_release();
			m_pi = tmp;
		}
		return *this;
	}

	void swap(weak_count& r)
	{
		smart_counted_base* tmp = r.m_pi;
		r.m_pi = m_pi;
		m_pi = tmp;
	}

	long use_count() const
	{
		return m_pi ? m_pi->use_count() : 0;
	}

	bool empty() const
	{
		return m_pi == NULL;
	}

	friend inline bool operator== (const weak_count& a, const weak_count& b)
	{
		return a.m_pi == b.m_pi;
	}

	friend inline bool operator< (const weak_count& a, const weak_count& b)
	{
		return std::less<smart_counted_base *>()(a.m_pi, b.m_pi);
	}
};

inline shared_count::shared_count(const weak_count& r)
	: m_pi(r.m_pi)
{
	if (m_pi && !m_pi->add_ref_lock())
	{
		m_pi = NULL;
	}
}

}// namespace detail


	
///////////////////////////////////////////////////////////////////////////////
// class scoped_ptr
///////////////////////////////////////////////////////////////////////////////
template<typename T> 
class scoped_ptr
{
private:
	T * m_ptr;

	typedef T element_type;
	typedef scoped_ptr<T> this_type;

	scoped_ptr(const scoped_ptr&);
	scoped_ptr& operator=(const scoped_ptr&);
	void operator==(const scoped_ptr&) const;
	void operator!=(const scoped_ptr&) const;

public:
	explicit scoped_ptr(T* p = NULL)
		: m_ptr(p)
	{
		// empty
	}

	explicit scoped_ptr(std::auto_ptr<T>& p)
		: m_ptr(p.release())
	{
		// empty
	}

	~scoped_ptr()
	{
		checked_delete(m_ptr);
	}

	void reset(T* p = NULL)
	{
		ASSERT( p == NULL || p != m_ptr ); // catch self-reset errors
		this_type(p).swap(*this);
	}

	T& operator*() const
	{
		ASSERT(m_ptr != NULL);
		return *m_ptr;
	}

	T* operator->() const
	{
		ASSERT(m_ptr != NULL);
		return m_ptr;
	}

	T* get() const
	{
		return m_ptr;
	}

	void swap(scoped_ptr & b)
	{
		T* tmp = b.m_ptr;
		b.m_ptr = m_ptr;
		m_ptr = tmp;
	}
};

template<class T>
inline void swap(scoped_ptr<T> & a, scoped_ptr<T> & b)
{
	a.swap(b);
}


///////////////////////////////////////////////////////////////////////////////
// class scoped_array
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class scoped_array
{
private:
	T * m_ptr;

	typedef T element_type;
	typedef scoped_array<T> this_type;

	scoped_array(const scoped_array&);
	scoped_array& operator= (const scoped_array&);
	void operator==(const scoped_array&) const;
	void operator!=(const scoped_array&) const;

public:
	explicit scoped_array(T * p = NULL)
		: m_ptr( p )
	{
		// empty
	}

	~scoped_array()
	{
		checked_array_delete(m_ptr);
	}

	void reset(T* p = NULL)
	{
		ASSERT(p == NULL || p != m_ptr); // catch self-reset errors
		this_type(p).swap(*this);
	}

	T& operator[](std::ptrdiff_t i) const
	{
		ASSERT(m_ptr != NULL);
		ASSERT(i >= 0);
		return m_ptr[i];
	}

	T* get() const
	{
		return m_ptr;
	}

	void swap(scoped_array & b) // never throws
	{
		T * tmp = b.m_ptr;
		b.m_ptr = m_ptr;
		m_ptr = tmp;
	}
};

template<class T>
inline void swap(scoped_array<T> & a, scoped_array<T> & b)
{
	a.swap(b);
}

///////////////////////////////////////////////////////////////////////////////
// class shared_ptr
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class shared_ptr
{
private:
	T* m_ptr;  // contained pointer
	detail::shared_count m_count; // reference pointer

	typedef T element_type;
	typedef T value_type;
	typedef checked_deleter<T> deleter;
	typedef shared_ptr<T> this_type;

	template<typename Y> friend class shared_ptr;
	template<typename Y> friend class weak_ptr;

public:
	shared_ptr()
		: m_ptr(NULL)
		, m_count()
	{
		// empty
	}

	template<typename Y>
	explicit shared_ptr(Y* p)
		: m_ptr(p)
		, m_count(p, deleter())
	{
		// empty
	}

	template<typename Y, typename D>
	explicit shared_ptr(Y* p, D d)
		: m_ptr(p)
		, m_count(p, d)
	{
		// empty
	}

	shared_ptr(const shared_ptr& r)
		: m_ptr(r.m_ptr)
		, m_count(r.m_count)
	{
		// empty
	}

	template<typename Y>
	shared_ptr(const shared_ptr<Y>& r)
		: m_ptr(r.m_ptr)
		, m_count(r.m_count)
	{
		// empty
	}

	template<typename Y>
	explicit shared_ptr(const weak_ptr<Y>& r)
		: m_ptr(NULL)
		, m_count(r.m_count)
	{
		if (!m_count.empty())
		{
			m_ptr = r.m_ptr;
		}
	}


	template<typename Y>
	explicit shared_ptr(const std::auto_ptr<Y>& r)
		: m_ptr(r.get())
		, m_count(r)
	{
		// empty
	}

	template<class Y>
	shared_ptr(const shared_ptr<Y>& r, detail::static_cast_tag)
		: m_ptr(static_cast<element_type *>(r.m_ptr))
		, m_count(r.m_count)
	{
		// empty
	}

	template<class Y>
	shared_ptr(const shared_ptr<Y>& r, detail::const_cast_tag)
		: m_ptr(const_cast<element_type *>(r.m_ptr))
		, m_count(r.m_count)
	{
		// empty
	}

	template<class Y>
	shared_ptr(const shared_ptr<Y>& r, detail::dynamic_cast_tag)
		: m_ptr(dynamic_cast<element_type *>(r.m_ptr))
		, m_count(r.m_count)
	{
		if(m_ptr == NULL) // need to allocate new counter -- the cast failed
		{
			m_count = detail::shared_count();
		}
	}

	template<class Y>
	shared_ptr(const shared_ptr<Y>& r, detail::polymorphic_cast_tag)
		: m_ptr(dynamic_cast<element_type *>(r.m_ptr))
		, m_count(r.m_count)
	{
		if(m_ptr == NULL)
		{
			throw(std::bad_cast());
		}
	}

	shared_ptr& operator= (const shared_ptr& r)
	{
		shared_ptr(r).swap(*this);
		return *this;
	}

	template<typename Y>
	shared_ptr& operator= (const shared_ptr<Y>& r)
	{
		this_type(r).swap(*this);
		return *this;
	}

	template<typename Y>
	shared_ptr& operator= (const std::auto_ptr<Y>& r)
	{
		this_type(r).swap(*this);
		return *this;
	}

	void reset()
	{
		this_type().swap(*this);
	}

	template<typename Y>
	void reset(Y* p = NULL)
	{
		ASSERT(p == NULL || p != m_ptr);
		shared_ptr(p).swap(*this);
	}

	template<typename Y, typename D>
	void reset(Y* p, D d)
	{
		ASSERT(p == NULL || p != m_ptr);
		this_type(p, d).swap(*this);
	}

	T& operator* () const
	{
		ASSERT(m_ptr != NULL);
		return *m_ptr;
	}

	T* operator-> () const
	{
		ASSERT(m_ptr != NULL);
		return m_ptr;
	}

	T* get() const
	{
		return m_ptr;
	}

	bool unique() const
	{
		return m_count.unique();
	}

	long use_count() const
	{
		return m_count.use_count();
	}

	void swap(shared_ptr<T>& r)
	{
		std::swap(m_ptr, r.m_ptr);
		m_count.swap(r.m_count);
	}

	template<typename Y>
	bool owner_before(const shared_ptr<Y>& r) const
	{
		return m_count < r.m_count;
	}

	template<typename Y>
	bool owner_before(const weak_ptr<Y>& r) const
	{
		return m_count < r.m_count;
	}
};

template<typename T, typename U>
bool operator== (const shared_ptr<T>& a, const shared_ptr<U>& b)
{
	return a.get() == b.get();
}

template<typename T, typename U>
bool operator!= (const shared_ptr<T>& a, const shared_ptr<U>& b)
{
	return a.get() != b.get();
}

template<typename T, typename U>
bool operator< (const shared_ptr<T>& a, const shared_ptr<U>& b)
{
	return a.owner_before(b);
}

template<typename T>
inline void swap(shared_ptr<T>& a, shared_ptr<T>& b)
{
	return a.swap(b);
}

template<typename Y>
std::ostream& operator<< (std::ostream& os, const shared_ptr<Y>& p)
{
	os << p.get();
	return os;
}

template<class T, class U>
shared_ptr<T> static_pointer_cast(shared_ptr<U> const & r)
{
	return shared_ptr<T>(r, detail::static_cast_tag());
}

template<class T, class U>
shared_ptr<T> const_pointer_cast(shared_ptr<U> const & r)
{
	return shared_ptr<T>(r, detail::const_cast_tag());
}

template<class T, class U>
shared_ptr<T> dynamic_pointer_cast(shared_ptr<U> const & r)
{
	return shared_ptr<T>(r, detail::dynamic_cast_tag());
}

///////////////////////////////////////////////////////////////////////////////
// class weak_ptr
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class weak_ptr
{
private:
	T* m_ptr;  // contained pointer
	detail::weak_count  m_count;  // reference counter

	typedef T element_type;
	typedef weak_ptr<T> this_type;

	template<typename Y> friend class weak_ptr;
	template<typename Y> friend class shared_ptr;

public:
	weak_ptr()
		: m_ptr(NULL)
		, m_count()
	{
		// empty
	}

	weak_ptr(const weak_ptr& r)
		: m_ptr(r.m_ptr)
		, m_count(r.m_count)
	{
		// empty
	}

	template<typename Y>
	weak_ptr(const weak_ptr<Y>& r)
		: m_ptr(r.lock().get())
		, m_count(r.m_count)
	{
		// empty
	}

	template<typename Y>
	weak_ptr(const shared_ptr<Y>& r)
		: m_ptr(r.m_ptr)
		, m_count(r.m_count)
	{
		// empty
	}

	weak_ptr& operator= (const weak_ptr& r)
	{
		m_ptr = r.m_ptr;
		m_count = r.m_count;
		return *this;
	}

	template<typename Y>
	weak_ptr& operator= (const weak_ptr<Y>& r)
	{
		m_ptr = r.lock().get();
		m_count = r.m_count;
		return *this;
	}

	template<typename Y>
	weak_ptr& operator= (const shared_ptr<Y>& r)
	{
		m_ptr = r.m_ptr;
		m_count = r.m_count;
		return *this;
	}

	shared_ptr<T> lock() const
	{
		return shared_ptr<element_type>(*this);
	}

	long use_count() const
	{
		return m_count.use_count();
	}

	bool expired() const
	{
		return m_count.use_count() == 0;
	}

	void reset()
	{
		this_type().swap(*this);
	}

	void swap(this_type& r)
	{
		std::swap(m_ptr, r.m_ptr);
		m_count.swap(r.m_count);
	}

	template<typename Y> 
	bool owner_before(const weak_ptr<Y>& r) const
	{
		return m_count < r.m_count;
	}

	template<typename Y>
	bool owner_before(const shared_ptr<Y>& r) const
	{
		return m_count < r.m_count;
	}
};

template<typename T, typename U>
inline bool operator< (const weak_ptr<T>& a, const weak_ptr<U>& b)
{
	return a.owner_before(b);
}

template<typename T>
inline void swap(weak_ptr<T>& a, weak_ptr<T>& b)
{
	a.swap(b);
}

///////////////////////////////////////////////////////////////////////////////
// class shared_array
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class shared_array
{
private:
	T* m_ptr;  // contained pointer
	detail::shared_count m_count; // reference counter

	typedef T element_type;
	typedef shared_array<T> this_type;
	typedef checked_array_deleter<T> deleter;

public:
	explicit shared_array(T* p = NULL)
		: m_ptr(p)
		, m_count(p, deleter())
	{
		// empty
	}

	template<typename D>
	shared_array(T* p, D d)
		: m_ptr(p)
		, m_count(p, d)
	{
		// empty
	}

	shared_array(const shared_array& r)
		: m_ptr(r.m_ptr)
		, m_count(r.m_count)
	{
		// empty
	}

	shared_array& operator=(const shared_array& r)
	{
		this_type(r).swap(*this);
		return *this;
	}

	void reset(T* p = NULL)
	{
		ASSERT(p == NULL || p != m_ptr);
		this_type(p).swap(*this);
	}

	template<typename D>
	void reset(T* p, D d)
	{
		ASSERT(p == NULL || p != m_ptr);
		this_type(p, d).swap(*this);
	}

	T& operator[] (std::ptrdiff_t i) const
	{
		ASSERT(m_ptr != NULL);
		ASSERT(i >= 0);
		return m_ptr[i];
	}

	T* get() const
	{
		return m_ptr;
	}

	bool unique() const
	{
		return m_count.unique();
	}

	long use_count() const
	{
		return m_count.use_count();
	}

	void swap(shared_array<T>& r)
	{
		std::swap(m_ptr, r.m_ptr);
		m_count.swap(r.m_count);
	}
};

template<typename T>
inline bool operator== (const shared_array<T>& a, const shared_array<T>& b)
{
	return a.get() == b.get();
}

template<typename T>
inline bool operator!= (const shared_array<T>& a, const shared_array<T>& b)
{
	return a.get() != b.get();
}

template<typename T>
inline bool operator< (const shared_array<T>& a, const shared_array<T>& b)
{
	return std::less<T*>()(a.get(), b.get());
}

template<typename T>
inline void swap(shared_array<T>& a, shared_array<T>& b)
{
	return a.swap(b);
}

}//namespace xcore

using namespace xcore;

#endif//_XCORE_SMART_PTR_H_

// 2011-07-13
// xcore_clock.h
//

#ifndef _XCORE_CLOCK_H_
#define _XCORE_CLOCK_H_

#include "tconnd_xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XClock
///////////////////////////////////////////////////////////////////////////////
class XClock
{
public:
	XClock();
	XClock(const XClock& from);
	XClock& operator=(const XClock& from);
	~XClock();

	void      reset();
	XTimeSpan peek();
	XTimeSpan diff_last();

private:
	class XClockImpl;
	scoped_ptr<XClockImpl> m_impl;
};

///////////////////////////////////////////////////////////////////////////////

extern XClock* static_clock;

// get time since server starting
XTimeSpan running_time(void);

} // namespace xcore

using namespace xcore;

#endif//_XCORE_CLOCK_H_

// 2008-02-23
// xcore_critical.h
// 
// 临界区类(允许递归锁)


#ifndef _XCORE_CRITICAL_H_
#define _XCORE_CRITICAL_H_

#include "tconnd_xcore_define.h"

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
// 2008-02-23
// xcore_event.h
// 
// windows下的事件概念


#ifndef _XCORE_EVENT_H_
#define _XCORE_EVENT_H_

#include "tconnd_xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XEvent
///////////////////////////////////////////////////////////////////////////////
class XEvent
{
public:
	// default is auto reset
	explicit XEvent(bool manual = false);

	~XEvent();

	static XEvent AutoStaticEvent;

	static XEvent ManualStaticEvent;

public:
	void set();

	void reset();

	void wait();

	bool trywait(int32 msec = 0);

private:
	class XEventImpl;
	scoped_ptr<XEventImpl> m_impl;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_EVENT_H_
// 2008-02-29
// xcore_memory.h
// 
// 自定义内存管理


#ifndef _XCORE_MEMORY_H_
#define _XCORE_MEMORY_H_

#include "tconnd_xcore_define.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// IXBuffer
////////////////////////////////////////////////////////////////////////////////
class IXBuffer
{
protected:
	virtual ~IXBuffer() {};

public:
	// get current usable buffer size
	virtual uint32 size() const = 0;

	// get current data pointer of buffer
	virtual void* data() const = 0;

	// set size() to range of [0, capacity()]
	virtual void resize(uint32 newSize) = 0;
	
	// get prepositive reserve size of buffer
	virtual uint32 size_pre_reserve() const = 0;

	// reset prepositive reserve size of buffer
	virtual void resize_pre_reserve(uint32 newSize) = 0;

	// get the usable max size of buffer
	virtual uint32 capacity() const = 0;

	// restore buffer to original state
	virtual void restore() = 0;

	// clone a new buffer from this buffer
	virtual shared_ptr<IXBuffer> clone() = 0;
};

shared_ptr<IXBuffer> create_buffer(uint32 uSize);

void* memory_alloc(uint32 uSize);

void memory_free(void* pData);

} // namespace xcore

using namespace xcore;

#endif//_XCORE_MEMORY_H_

// 2008-02-23
// xcore_mutex.h
// 


#ifndef _XCORE_MUTEX_H_
#define _XCORE_MUTEX_H_

#include "tconnd_xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XMutex
///////////////////////////////////////////////////////////////////////////////
class XMutex
{
public:
	XMutex();

	~XMutex();

	static XMutex StaticMutex;

public:
	void lock();

	bool trylock();

	void unlock();

private:
	class XMutexImpl;
	XMutexImpl* m_impl;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_MUTEX_H_

// 2008-02-27
// xcore_semaphore.h
// 
// 信号量类


#ifndef _XCORE_SEMAPHORE_H_
#define _XCORE_SEMAPHORE_H_

#include "tconnd_xcore_define.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XSemaphore
////////////////////////////////////////////////////////////////////////////////
class XSemaphore
{
public:
	explicit XSemaphore(uint32 init_count = 0);

	~XSemaphore();

	void wait();

	bool trywait(uint32 msec = 0);

	bool post(uint32 count = 1);

	uint32 get_value();

private:
	class XSemaphoreImpl;
	scoped_ptr<XSemaphoreImpl> m_impl;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_SEMAPHORE_H_

// 20010-09-29
// xcore_sock_addr.h
// 


#ifndef _XCORE_SOCK_ADDR_H_
#define _XCORE_SOCK_ADDR_H_

#include "tconnd_xcore_define.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XSockAddr
////////////////////////////////////////////////////////////////////////////////
class XSockAddr
{
public:
	static const XSockAddr AnyAddr;
	static const XSockAddr NoneAddr;

	XSockAddr(void);
	XSockAddr(const XSockAddr& addr); 
	XSockAddr(const sockaddr_in& addr);
	XSockAddr(const sockaddr& addr);
	XSockAddr(const string& addr); // addr such as "192.168.4.12:80" or "www.sina.com.cn:80"
	XSockAddr(const string& host, uint16 port); // host such as "192.168.4.12" or "www.sina.com.cn"
	XSockAddr(uint32 ip, uint16 port); // ip: 0x12131415 => 18.19.20.21
	~XSockAddr(void);
	
	XSockAddr& operator = (const XSockAddr& addr);
	XSockAddr& operator = (const sockaddr_in& addr);
	XSockAddr& operator = (const sockaddr& addr);
	operator const sockaddr_in *() const;
	operator const sockaddr *() const;
	operator sockaddr_in() const;
	operator sockaddr() const;

	void   set_port(uint16 port);
	void   set_ipaddr(uint32 ip); // ip: 0x12131415 => 18.19.20.21
	bool   set_ipaddr(const string& ip); // ip such as "192.168.5.12"
	bool   set_host(const string& host); // host such as "192.168.4.12" or "www.sina.com.cn"
	uint16 get_port() const;
	uint32 get_ipaddr() const;
	string get_hostname() const;
	string get_hostaddr() const;
	string to_str() const;
	void   reset();

	bool is_any() const;
	bool is_none() const;
	bool is_loopback() const;
	bool is_multicast() const;

	static string    local_net_name();
	static XSockAddr local_mainaddr();
	static bool local_addrs(vector<XSockAddr>& addrs);

	friend bool operator <  (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator <= (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator >  (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator >= (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator == (const XSockAddr& addr1, const XSockAddr& addr2);
	friend bool operator != (const XSockAddr& addr1, const XSockAddr& addr2);

public:
    string m_host;
    uint16 m_port;

private:
	sockaddr_in  m_inaddr;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_SOCK_ADDR_H_
// 2008-12-03
// xcore_socket.h
//


#ifndef _XCORE_SOCKET_H_
#define _XCORE_SOCKET_H_

#include "tconnd_xcore_define.h"

namespace xcore {

#ifdef __WINDOWS__
#pragma comment(lib, "ws2_32.lib")

struct iovec
{
	void*  iov_base;  // Base address
	size_t iov_len;   // Length
};
#endif//__WINDOWS__

////////////////////////////////////////////////////////////////////////////////
// class XSocket
////////////////////////////////////////////////////////////////////////////////
class XSocket : XNoncopyable
{
public:
	explicit XSocket(SOCKET sock = XCORE_INVALID_SOCKET) : m_sock(sock) {
		isstop = false;
    }
	~XSocket() {
    }

	SOCKET get_handle() const { return m_sock; }
	bool   is_open() const;
	void   attach(SOCKET sock);
	SOCKET detach();

	bool open(int type = SOCK_STREAM);
	bool bind(const XSockAddr& addr);
	int connect(const XSockAddr& addr, int timeout_ms = 10000/*10s*/);
	bool listen(const XSockAddr& addr, int backlog = -1);
	bool accept(XSocket &sock, XSockAddr* remote_addr = NULL);
	bool shutdown();
	bool close(int delay = -1);
	bool mutex_close();
    bool reconnect(const XSockAddr& addr);
	bool abort();
	void StopAutoConnect(bool stop) {
		isstop = stop;
	}

	XSockAddr local_addr() const;
	XSockAddr remote_addr() const;

public:
	bool set_reuse_addr(bool bl = true);
	bool set_keep_alive(bool bl = true);
	bool set_nonblock(bool bl = true);
	bool set_tcp_nodelay(bool bl = true);
	bool set_linger(uint16 delay_sec, bool bl = true);
	bool set_send_bufsize(uint32 nSize = 8192);
	bool set_recv_bufsize(uint32 nSize = 8192);
	bool get_send_bufsize(uint32& nSize) const;
	bool get_recv_bufsize(uint32& nSize) const;

public:
	bool can_recv(int timeout_ms = -1);
	bool can_send(int timeout_ms = -1);

	// return >=0: send/recv number of bytes, -1: error or peer host closed
	int send(const void* buf, int len);
	int recv(void* buf, int len);
	int sendto(const void* buf, int len, const XSockAddr& addr);
	int recvfrom(void* buf, int len, XSockAddr& addr);
	int send_n(const void* buf, int len, int timeout_ms = -1);
	int recv_n(void* buf, int len, int timeout_ms = -1);
    int recv_http(char*& buf, int& len,
            char*& last_left, int& left_len,
            char*& http_head, int timeout_ms);
    int recv_one_http(std::string& one_http, int timeout_ms);

	bool send_v(const iovec* iov, int cnt);

	volatile SOCKET m_sock;

private:
	bool _is_can_restore();
	bool _is_already();
	bool _is_would_block();
	bool _is_emfile();

private:
	XMutex mutex_;
	bool isstop;
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_SOCKET_H_
// 2009-02-04
// xcore_str_util.h
// 
// 包装一些方便字符串操作的函数


#ifndef _XCORE_STR_UTIL_H_
#define _XCORE_STR_UTIL_H_

#include "tconnd_xcore_define.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XStrUtil
///////////////////////////////////////////////////////////////////////////////
class XStrUtil
{
public:
	// 去除字符串头(或尾)中在字符集中指定的字符
	static string& chop_head(string &strSrc, const char *pcszCharSet = " \t\r\n");
	static string& chop_tail(string &strSrc, const char *pcszCharSet = " \t\r\n");
	static string& chop(string &strSrc, const char *pcszCharSet = " \t\r\n");

	// 字符串转大写(或小写)
	static void to_upper(char *pszSrc);
	static void to_lower(char *pszSrc);
	static void to_upper(string &strSrc);
	static void to_lower(string &strSrc);

	// 替换
	static void replace(char* str, char oldch, char newch);
	static void replace(char* str, const char* oldCharSet, char newch);

	// 区分大小写比较
	static int compare(const char* pszSrc1, const char* pszSrc2, int length = -1);
	static int compare(const string &str1, const string &str2, int length = -1);

	// 不区分大小写比较
	static int compare_nocase(const char* pszSrc1, const char* pszSrc2, int length = -1);
	static int compare_nocase(const string &str1, const string &str2, int length = -1);

	// 根据字符集中指定的分隔字符分解源字符串,并放置到vector中
	// nMaxCount指定期望得到的行数,解析到maxCount将终止并返回,不会继续解析;设为-1表示解析所有
	static uint32 split(const string &strSrc, vector<string>& vItems, const char *pcszCharSet = " \r\n\t", int nMaxCount = -1);

	// 字符串转整数
	static bool	  to_int(const string &strSrc, int &nValue, int radix = 10);
	static int	  to_int_def(const string &strSrc, int def = -1, int radix = 10);
	static int	  try_to_int_def(const string &strSrc, int def = -1, int radix = 10);
	static bool	  to_uint(const string &strSrc, uint32 &uValue, int radix = 10);
	static uint32 to_uint_def(const string &strSrc, uint32 def = 0, int radix = 10);
	static uint32 try_to_uint_def(const string &strSrc, uint32 def = 0, int radix = 10);

	// 字符串转浮点型数
	static bool   to_float(const string &strSrc, double &value);
	static double to_float_def(const string &strSrc, double def = 0.0);
	static double try_to_float_def(const string &strSrc, double def = 0.0);

	// 数值转字符串
	static string to_str(int nVal, const char* cpszFormat = NULL/*"%d"*/);
	static string to_str(uint32 uVal, const char* cpszFormat = NULL/*"%u"*/);
	static string to_str(int64 nlVal, const char* cpszFormat = NULL/*"%lld"*/);
	static string to_str(uint64 ulVal, const char* cpszFormat = NULL/*"%llu"*/);
	static string to_str(double fVal, const char* cpszFormat = NULL/*"%f"*/);

	// string hash
	static uint32 hash_code(const char* str);
	static uint32 murmur_hash(const void* key, uint32 len);

	// dump data
	static void dump(string& result, const void* pdata, uint32 length);
};

} // namespace xcore

using namespace xcore;

#endif//_XCORE_STR_UTIL_H_
// 2008-02-23
// xcore_thread.h
// 


#ifndef _XCORE_THREAD_H_
#define _XCORE_THREAD_H_

#include "tconnd_xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// Interface XRunnable
///////////////////////////////////////////////////////////////////////////////
class XThread;
class XRunnable
{
public:
	virtual ~XRunnable() {}

	virtual void run(XThread* pThread) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// class XThread
///////////////////////////////////////////////////////////////////////////////
class XThread : private XNoncopyable, public XRunnable
{
public:
	typedef enum { CREATE, START, IDLE, RUNNING, STOP, JOINED } STATE;

	XThread();

	virtual ~XThread();

	virtual bool start(XRunnable* r = NULL, uint32 stack_size = 0);

	virtual void stop();

	virtual void join();

	virtual void kill();

	virtual bool wait_quit(uint32 msec = 0);

	uint64 id() { return m_id; }

	int state() { return m_state; }

protected:
	virtual void run(XThread* pThread) {};

	friend unsigned __stdcall invok_proc_win(void *arg);
	friend void* invok_proc_posix(void *arg);

protected:
	volatile uint64  m_id;     // thread id that be managed
	volatile int     m_state;
	XRunnable*       m_runnable;
	HANDLE		     m_handle; // use at windows platform only
	XEvent		     m_quit;   // manual reset event
};

///////////////////////////////////////////////////////////////////////////////

uint32 thread_id();

void sleep(uint32 msec);

} // namespace xcore

using namespace xcore;

#endif//_XCORE_THREAD_H_
// 2009-07-28
// xcore_timer.h
//
// 定时器实现(精度: 毫秒)


#ifndef _XCORE_TIMER_H_
#define _XCORE_TIMER_H_

#include "tconnd_xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XTimer
////////////////////////////////////////////////////////////////////////////////
class XTimer
{
public:
	class ICallBack
	{
	public:
		virtual ~ICallBack() {}

		virtual void on_timer(XTimer* pTimer, uint32 id, void* ptr) = 0;
	};

	static XTimer StaticTimer;

public:
	XTimer();

	~XTimer();

	uint32 schedule(ICallBack* callback, uint32 interval_ms, int repeat = 1, void* ptr = NULL);

	uint32 cancel(uint32 id);

	uint32 cancel(void* ptr);

	int32  next_timed();

    bool   start();

    bool   stop();

private:
    XThread thread_;
    class XTimerImpl;
	XTimerImpl* m_impl;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_TIMER_H_
// 20010-09-29
// xcore_time_span.h
// 
// 时间段类


#ifndef _XCORE_TIME_SPAN_H_
#define _XCORE_TIME_SPAN_H_

#include "tconnd_xcore_define.h"
#include <string>

using std::string;

namespace xcore {
	
///////////////////////////////////////////////////////////////////////////////
// class XTimeSpan
////////////////////////////////////////////////////////////////////////////////
class XTimeSpan
{
public:
	XTimeSpan();
	XTimeSpan(const XTimeSpan& other);
	XTimeSpan(int64 sec, int64 usec = 0);
	XTimeSpan(const timeval& tv);
	XTimeSpan(double d);
	~XTimeSpan();

	XTimeSpan& set(int64 sec, int64 usec = 0);
	XTimeSpan& set(const timeval& tv);
	XTimeSpan& set(double d);
	XTimeSpan& from_usecs(int64 usec);
	XTimeSpan& from_msecs(int64 msec);

	int64  sec() const { return m_sec; }
	int64  usec() const { return m_usec; }
	int64  to_usecs() const;
	int64  to_msecs() const;
	int64  to_secs() const;
	string to_str() const;

	operator double() const;

	XTimeSpan& operator += (const XTimeSpan& t);
	XTimeSpan& operator -= (const XTimeSpan& t);
	XTimeSpan& operator =  (const XTimeSpan& t);

	friend bool operator <  (const XTimeSpan& t1, const XTimeSpan& t2);
	friend bool operator >  (const XTimeSpan& t1, const XTimeSpan& t2);
	friend bool operator <= (const XTimeSpan& t1, const XTimeSpan& t2);
	friend bool operator >= (const XTimeSpan& t1, const XTimeSpan& t2);
	friend bool operator == (const XTimeSpan& t1, const XTimeSpan& t2);
	friend bool operator != (const XTimeSpan& t1, const XTimeSpan& t2);
	friend XTimeSpan operator + (const XTimeSpan& t1, const XTimeSpan& t2);
	friend XTimeSpan operator - (const XTimeSpan& t1, const XTimeSpan& t2);
	friend XTimeSpan operator * (const XTimeSpan& t, double d);
	friend XTimeSpan operator * (double d, const XTimeSpan& t);
	friend XTimeSpan operator / (const XTimeSpan& t, double d);

private:
	void _normalize();

private:
	volatile int64  m_sec;
	volatile int64  m_usec;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_TIME_SPAN_H_

#ifndef C_COROUTINE_H
#define C_COROUTINE_H

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

#include "tconnd_closure.h"

#include <ucontext.h>
#include <poll.h>
#include <pthread.h>
#include <map>

using std::map;

#define STACK_SIZE (1024*1024*8)
#define DEFAULT_COROUTINE 16

struct schedule {
    char stack[STACK_SIZE];
    ucontext_t main;
    int nco;
    int cap;
    int running;
    struct coroutine **co;
    unsigned threadid;
};

struct coroutine {
    Closure<void>* func;
    ucontext_t ctx;
    struct schedule * sch;
    ptrdiff_t cap;
    ptrdiff_t size;
    int status;
    char *stack;
    bool enable_sys_hook;
};

struct schedule * coroutine_open(void);
struct schedule * coroutine_create(void);
void coroutine_close(struct schedule *);

int coroutine_new(struct schedule *, Closure<void>* closure);
void coroutine_resume(struct schedule *, int id);
int coroutine_status(struct schedule *, int id);
int coroutine_running(struct schedule *);
void coroutine_yield(struct schedule *);


typedef struct schedule * CroMgr;
extern CroMgr singleton;
void SetCoroutineUsedByCurThread();
void CloseUseCoroutine();
CroMgr GetCroMgr();
bool ProcessWithNewCro(Closure<void>* closure);
void co_resume_in_suspend(CroMgr mgr, int croid);

void co_enable_hook_sys();
void co_disable_hook_sys();
bool co_is_enable_sys_hook();

void co_log_err(const char* fmt, ...);

struct MyPollFd {
public:
    MyPollFd() {
        cuid = 0;
        revents = 0;
    }
    unsigned cuid;
    short revents;
};
typedef struct MyPollFd POFD;

class ClosureMap {
public:
    bool Insert(unsigned key, Closure<void>* closure) {
        _mux.lock();
        std::map<unsigned, Closure<void>*>::iterator it = clo_map_.find(key);
        if (it != clo_map_.end()) {
            printf("clo map key conflict\n");
            abort();
        }
        clo_map_[key] = closure;
        _mux.unlock();
        return true;
    }

    Closure<void>* Pop(unsigned key) {
        _mux.lock();
        Closure<void>* ret = NULL;
        std::map<unsigned, Closure<void>*>::iterator it = clo_map_.find(key);
        if (it != clo_map_.end()) {
            ret = it->second;
            clo_map_.erase(it);
        }
        _mux.unlock();
        return ret;
    }
    std::map<unsigned, Closure<void>*> clo_map_;
    XMutex _mux;
};

class PFMutMap {
public:
    bool Insert(unsigned key, POFD& a_pfd) {
        _mux.lock();
        POFD* node = new POFD();
        *node = a_pfd;
        pofd_map_[key] = node;
        _mux.unlock();
        return true;
    }

    POFD Pop(unsigned key) {
        _mux.lock();
        POFD ret;
        std::map<unsigned, POFD*>::iterator it = pofd_map_.find(key);
        if (it != pofd_map_.end()) {
            POFD* node = it->second;
            ret = *node;
            delete node;
            pofd_map_.erase(it);
        }
        _mux.unlock();
        return ret;
    }

    POFD Get(unsigned key) {
        _mux.lock();
        POFD ret;
        std::map<unsigned, POFD*>::iterator it = pofd_map_.find(key);
        if (it != pofd_map_.end()) {
            POFD* node = it->second;
            ret = *node;
        }
        _mux.unlock();
        return ret;
    }

    bool Set(unsigned key, POFD& a_pofd) {
        bool ret = false;
        _mux.lock();
        std::map<unsigned, POFD*>::iterator it = pofd_map_.find(key);
        if (it != pofd_map_.end()) {
            POFD* node = it->second;
            *node = a_pofd;
            ret = true;
        }
        _mux.unlock();
        return ret;
    }

    std::map<unsigned, POFD*> pofd_map_;
    XMutex _mux;
};

extern int hook_epollfd;

void* HookEpollLoop(void *argument);

void StartHookEpoll();

class HookMgr {
public:
    HookMgr();
    ~HookMgr();
    static void Start();
    static void CloseEpoll();
    static bool is_stop;
    static pthread_t epoll_thread;
};

#endif

#ifndef _XCORE_RPC_COMMON_H_
#define _XCORE_RPC_COMMON_H_

class XcoreMgr {
public:
    static void PutOutSideQueue(Closure<void>* done);
    static Closure<void>* GetOutSideQueue(unsigned msec);
    static void RunWithCoroutine(Closure<void>* closure);
    static int Update(unsigned msec);

    static std::deque< Closure<void>* > outside_queue;
    static XSemaphore   m_recvsem;
    static XMutex       m_recvmux;
};

class CroTimer : public xcore::XTimer::ICallBack {
public:
    CroTimer(Closure<void>* a_closure, int a_repeat) {
        closure = a_closure;
        repeat = a_repeat;
    }
    virtual ~CroTimer() {
        if (closure)
            delete closure;
    }
    virtual void on_timer(XTimer* pTimer, uint32 id, void* ptr);
    int repeat;
    Closure<void>* closure;
};

class TimerMgr {
public:
    TimerMgr() {
        is_stop = true;
    }
    void Start() {
        is_stop = false;
        timer.start();
    }
    void Close() {
        if (!is_stop) {
            is_stop = true;
            timer.stop();
        }
    }
    ~TimerMgr() {
        Close();
    }
    unsigned AddJob(unsigned timeout, Closure<void>* closure, int repeat = 1) {
        CroTimer* ct = new CroTimer(closure, repeat);
        return timer.schedule(ct, (uint32)timeout, repeat);
    }
    unsigned DelJob(unsigned uid) {
        return timer.cancel(uid);
    }
    xcore::XTimer timer;
    bool is_stop;
};

#endif
