// 2008-02-27
// xcore_semaphore.h
// 
// –≈∫≈¡ø¿‡


#ifndef _XCORE_SEMAPHORE_H_
#define _XCORE_SEMAPHORE_H_

#include "xcore_define.h"
#include "xcore_smart_ptr.h"

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
