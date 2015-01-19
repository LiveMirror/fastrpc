// 2009-07-27
// xcore_atomic.cpp
// 
// 原子操作类


#include "xcore_atomic.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XAtomic32
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

XAtomic32::XAtomic32(int32 i)
	: m_counter(i)
{
	ASSERT(sizeof(int32) == sizeof(LONG));
}

XAtomic32::XAtomic32(const XAtomic32& from)
	: m_counter(from.m_counter)
{
	ASSERT(sizeof(int32) == sizeof(LONG));
}

XAtomic32::~XAtomic32()
{
	// empty
}

int32 XAtomic32::get_value() const
{
	return InterlockedCompareExchange((volatile LONG*)&m_counter, 0, 0);
}

int32 XAtomic32::set_value(int32 i)
{
	return InterlockedExchange((LONG*)&m_counter, i);
}

int32 XAtomic32::test_zero_inc()
{
	for ( ; ; )
	{
		LONG tmp = get_value();
		if (tmp <= 0) return tmp;
		if (InterlockedCompareExchange((LONG*)&m_counter, tmp + 1, tmp) == tmp) return tmp + 1;
	}
	return 0;
}

XAtomic32::operator int32()
{
	return get_value();
}

XAtomic32& XAtomic32::operator= (int32 i)
{
	this->set_value(i);
	return *this;
}

XAtomic32& XAtomic32::operator= (const XAtomic32& from)
{
	if (this != &from)
	{
		this->set_value(from.m_counter);
	}
	return *this;
}

int32 XAtomic32::operator+= (int32 i)
{
	return InterlockedExchangeAdd((LONG*)&m_counter, i) + i;
}

int32 XAtomic32::operator-= (int32 i)
{
	return InterlockedExchangeAdd((LONG*)&m_counter, -i) - i;
}

int32 XAtomic32::operator++ (int)
{
	return  InterlockedIncrement((LONG*)&m_counter) - 1;
}

int32 XAtomic32::operator-- (int)
{
	return InterlockedDecrement((LONG*)&m_counter) + 1;
}

int32 XAtomic32::operator++ ()
{
	return InterlockedIncrement((LONG*)&m_counter);
}

int32 XAtomic32::operator-- ()
{
	return InterlockedDecrement((LONG*)&m_counter);
}

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

// 在linux2.6.18之前，多采用以下方法实现原子操作，这种做法与CPU类型相关
//static int32 add_return(int32& counter, int32 i)
//{
//	/* Modern 486+ processor */
//	__asm__ __volatile__(
//		"lock ; xaddl %0, %1"
//		:"+r" (i), "+m" (counter)
//		: : "memory");
//	return i; // 返回原有值
//}
//
// 在linux2.6.18之后，删除了<asm/atomic.h>和<asm/bitops.h>头文件，编译器提供内建(built-in)原子操作函数。
// 需要在gcc编译选项中指明CPU类型。如gcc -marth=i686 -o hello hello.c

XAtomic32::XAtomic32(int32 i)
	: m_counter(i)
{
	ASSERT(sizeof(int32) == sizeof(int32_t));
}

XAtomic32::XAtomic32(const XAtomic32& from)
	: m_counter(from.m_counter)
{
	ASSERT(sizeof(int32) == sizeof(int32_t));
}

XAtomic32::~XAtomic32()
{
	// empty
}

int32 XAtomic32::get_value() const
{
	return __sync_val_compare_and_swap((volatile int*)&m_counter, 0, 0);
}

int32 XAtomic32::set_value(int32 i)
{
	// set m_counter = i and return old value of m_counter
	return __sync_lock_test_and_set(&m_counter, i);
}

int32 XAtomic32::test_zero_inc()
{
	// if (m_counter != 0) ++m_counter; return m_counter;

	for( ; ; )
	{
		int32 tmp = get_value();
		if(tmp <= 0) return tmp;
		if (__sync_val_compare_and_swap(&m_counter, tmp, tmp + 1) == tmp) return tmp + 1;
	}
	return 0;
}

XAtomic32::operator int32()
{
	return get_value();
}

XAtomic32& XAtomic32::operator= (int32 i)
{
	this->set_value(i);
	return *this;
}

XAtomic32& XAtomic32::operator= (const XAtomic32& from)
{
	if (this != &from)
	{
		this->set_value(from.get_value());
	}
	return *this;
}

int32 XAtomic32::operator+= (int32 i)
{
	return __sync_add_and_fetch(&m_counter, i);
}

int32 XAtomic32::operator-= (int32 i)
{
	return __sync_sub_and_fetch(&m_counter, i);
}

int32 XAtomic32::operator++ (int)
{
	return __sync_fetch_and_add(&m_counter, 1);
}

int32 XAtomic32::operator-- (int)
{
	return __sync_fetch_and_sub(&m_counter, 1);
}

int32 XAtomic32::operator++ ()
{
	return __sync_add_and_fetch(&m_counter, 1);
}

int32 XAtomic32::operator-- ()
{
	return __sync_sub_and_fetch(&m_counter, 1);
}

#endif//__GNUC__


///////////////////////////////////////////////////////////////////////////////
// class XAtomic64
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

XAtomic64::XAtomic64(int64 i)
	: m_counter(0)
{
#if (_WIN32_WINNT >= 0x0502)
	m_counter = i;
#else
	*(volatile LONG*)&m_counter = (LONG)i;
#endif

	ASSERT(sizeof(int64) == sizeof(LONGLONG));
}

XAtomic64::XAtomic64(const XAtomic64& from)
	: m_counter(0)
{
#if (_WIN32_WINNT >= 0x0502)
	m_counter = from.m_counter;
#else
	*(volatile LONG*)&m_counter = (LONG)from.m_counter;
#endif

	ASSERT(sizeof(int64) == sizeof(LONGLONG));
}

XAtomic64::~XAtomic64()
{
	// empty
}

int64 XAtomic64::get_value() const
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedCompareExchange64((volatile LONGLONG*)&m_counter, 0, 0);
#else
	return (int64)InterlockedCompareExchange((volatile LONG*)&m_counter, 0, 0);
#endif
}

int64 XAtomic64::set_value(int64 i)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedExchange64((volatile LONGLONG*)&m_counter, i);
#else
	return (int64)InterlockedExchange((volatile LONG*)&m_counter, (LONG)i);
#endif
}

int64 XAtomic64::test_zero_inc()
{
#if (_WIN32_WINNT >= 0x0502)
	for ( ; ; )
	{
		LONGLONG tmp = get_value();
		if (tmp <= 0) return tmp;
		if (InterlockedCompareExchange64((volatile LONGLONG*)&m_counter, tmp + 1, tmp) == tmp) return tmp + 1;
	}
	return 0;
#else
	for ( ; ; )
	{
		LONG tmp = (LONG)get_value();
		if (tmp <= 0) return tmp;
		if (InterlockedCompareExchange((volatile LONG*)&m_counter, tmp + 1, tmp) == tmp) return (int64)tmp + 1;
	}
	return 0;
#endif
}

XAtomic64::operator int64()
{
	return get_value();
}

XAtomic64& XAtomic64::operator= (int64 i)
{
	this->set_value(i);
	return *this;
}

XAtomic64& XAtomic64::operator= (const XAtomic64& from)
{
	if (this != &from)
	{
		this->set_value(from.m_counter);
	}
	return *this;
}

int64 XAtomic64::operator+= (int64 i)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedExchangeAdd64((volatile LONGLONG*)&m_counter, i) + i;
#else
	return (int64)InterlockedExchangeAdd((volatile LONG*)&m_counter, i) + i;
#endif
}

int64 XAtomic64::operator-= (int64 i)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedExchangeAdd64((volatile LONGLONG*)&m_counter, -i) - i;
#else
	return (int64)InterlockedExchangeAdd((volatile LONG*)&m_counter, -i) - i;
#endif
}

int64 XAtomic64::operator++ (int)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedIncrement64((volatile LONGLONG*)&m_counter) - 1;
#else
	return (int64)InterlockedIncrement((volatile LONG*)&m_counter) - 1;
#endif
}

int64 XAtomic64::operator-- (int)
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedDecrement64((volatile LONGLONG*)&m_counter) + 1;
#else
	return (int64)InterlockedDecrement((volatile LONG*)&m_counter) + 1;
#endif
}

int64 XAtomic64::operator++ ()
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedIncrement64((volatile LONGLONG*)&m_counter);
#else
	return (int64)InterlockedIncrement((volatile LONG*)&m_counter);
#endif
}

int64 XAtomic64::operator-- ()
{
#if (_WIN32_WINNT >= 0x0502)
	return InterlockedDecrement64((volatile LONGLONG*)&m_counter);
#else
	return (int64)InterlockedDecrement((volatile LONG*)&m_counter);
#endif
}

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

XAtomic64::XAtomic64(int64 i)
	: m_counter(i)
{
	ASSERT(sizeof(int64) == sizeof(int64_t));
}

XAtomic64::XAtomic64(const XAtomic64& from)
	: m_counter(from.m_counter)
{
	ASSERT(sizeof(int64) == sizeof(int64_t));
}

XAtomic64::~XAtomic64()
{
	// empty
}

int64 XAtomic64::get_value() const
{
	return __sync_val_compare_and_swap((volatile int64*)&m_counter, 0, 0);
}

int64 XAtomic64::set_value(int64 i)
{
	// set m_counter = i and return old value of m_counter
	return __sync_lock_test_and_set(&m_counter, i);
}

int64 XAtomic64::test_zero_inc()
{
	// if (m_counter != 0) ++m_counter; return m_counter;

	for( ; ; )
	{
		int64 tmp = get_value();
		if(tmp <= 0) return tmp;
		if (__sync_val_compare_and_swap(&m_counter, tmp, tmp + 1) == tmp) return tmp + 1;
	}
	return 0;
}

XAtomic64::operator int64()
{
	return get_value();
}

XAtomic64& XAtomic64::operator= (int64 i)
{
	this->set_value(i);
	return *this;
}

XAtomic64& XAtomic64::operator= (const XAtomic64& from)
{
	if (this != &from)
	{
		this->set_value(from.get_value());
	}
	return *this;
}

int64 XAtomic64::operator+= (int64 i)
{
	return __sync_add_and_fetch(&m_counter, i);
}

int64 XAtomic64::operator-= (int64 i)
{
	return __sync_sub_and_fetch(&m_counter, i);
}

int64 XAtomic64::operator++ (int)
{
	return __sync_fetch_and_add (&m_counter, 1);
}

int64 XAtomic64::operator-- (int)
{
	return __sync_fetch_and_sub(&m_counter, 1);
}

int64 XAtomic64::operator++ ()
{
	return __sync_add_and_fetch(&m_counter, 1);
}

int64 XAtomic64::operator-- ()
{
	return __sync_sub_and_fetch(&m_counter, 1);
}

#endif//__GNUC__

}//namespace xcore

////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore {

bool xcore_test_atomic()
{
	XAtomic32 a1 = 2;
	ASSERT(a1.get_value() == 2);
	ASSERT((int)a1 == 2);
	a1.set_value(2);
	ASSERT(a1.get_value() == 2);
	a1 = 3;
	ASSERT(a1.get_value() == 3);
	VERIFY(a1++ == 3);
	VERIFY(++a1 == 5);
	VERIFY(a1-- == 5);
	VERIFY(--a1 == 3);
	VERIFY(5 == (a1 += 2));
	VERIFY(2 == (a1 -= 3));

	//XAtomic32 a2 = a1;
	//XAtomic32 a3;
	//a3 = a1;
	//XAtomic32 a4(a1);
	XAtomic32 a5 = a1.get_value();
	XAtomic32 a6;
	a6 = a1.get_value();
	XAtomic32 a7(a1.get_value());

	vector<XAtomic32> v;
	//v.push_back(XAtomic32(0));
	//v.push_back(a1);
	//v.push_back(a1.get_value());
	//v.push_back(4);

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
