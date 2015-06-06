// 2009-07-27
// xcore_atomic.h
// 
// 原子操作类


#ifndef _XCORE_ATOMIC_H_
#define _XCORE_ATOMIC_H_

#include "xcore_define.h"

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
	
	int32 get_value() const;  // 返回当前值
	int32 set_value(int32 i); // 返回原值
	int32 test_zero_inc(); // 返回增量后的值

	operator int32();
	XAtomic32& operator= (int32 i);
	XAtomic32& operator= (const XAtomic32& from);

	int32 operator+= (int32 i); // 返回增量后的值
	int32 operator-= (int32 i); // 返回减量后的值
	int32 operator++ (int); // 返回原值
	int32 operator-- (int); // 返回原值
	int32 operator++ (); // 返回增量后的值
	int32 operator-- (); // 返回减量后的值

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

	int64 get_value() const;  // 返回当前值
	int64 set_value(int64 i); // 返回原值
	int64 test_zero_inc(); // 返回增量后的值

	operator int64();
	XAtomic64& operator= (int64 i);
	XAtomic64& operator= (const XAtomic64& from);

	int64 operator+= (int64 i); // 返回增量后的值
	int64 operator-= (int64 i); // 返回减量后的值
	int64 operator++ (int); // 返回原值
	int64 operator-- (int); // 返回原值
	int64 operator++ (); // 返回增量后的值
	int64 operator-- (); // 返回减量后的值

private:
	volatile int64 m_counter;
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_ATOMIC_H_
