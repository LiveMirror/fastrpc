// 2008-02-23
// xcore_mutex.h
// 
// »¥³âËø(²»ÔÊÐíµÝ¹éËø)


#ifndef _XCORE_MUTEX_H_
#define _XCORE_MUTEX_H_

#include "xcore_define.h"
#include "xcore_smart_ptr.h"

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
