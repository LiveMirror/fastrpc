// 2011-07-13
// xcore_clock.h
// 
// 计时器类(修改系统时间不受影响)


#ifndef _XCORE_CLOCK_H_
#define _XCORE_CLOCK_H_

#include "xcore_define.h"
#include "xcore_time_span.h"
#include "xcore_smart_ptr.h"

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

// get time since server starting
XTimeSpan running_time(void);

} // namespace xcore

using namespace xcore;

#endif//_XCORE_CLOCK_H_

