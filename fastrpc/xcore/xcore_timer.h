// 2009-07-28
// xcore_timer.h
//
// 定时器实现(精度: 毫秒)


#ifndef _XCORE_TIMER_H_
#define _XCORE_TIMER_H_

#include "xcore_define.h"
#include "xcore_smart_ptr.h"
#include "xcore_thread.h"

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
