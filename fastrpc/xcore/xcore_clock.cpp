// 2011-07-13
// xcore_clock.cpp
// 
// 计时器类


#include "xcore_clock.h"

#pragma comment(lib, "winmm.lib")

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XClock
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

static uint32 __getNumberOfProcessors()
{
	SYSTEM_INFO SysInfo; 
	GetSystemInfo(&SysInfo);
	return SysInfo.dwNumberOfProcessors;
}

static LARGE_INTEGER XClock_frequency;
static BOOL XClock_result = QueryPerformanceFrequency(&XClock_frequency);
static uint32 XClock_numberOfProcessors = __getNumberOfProcessors();

// 这套机制在多核条件下会出问题，可参考SetThreadAffinityMas,timeGetTime两个函数
class XClock::XClockImpl
{
public:
	XClockImpl()
	{
		VERIFY(QueryPerformanceCounter(&m_start));
		memcpy(&m_last, &m_start, sizeof(m_last));
		m_start2 = (uint64)timeGetTime();
		m_last2 = m_start2;
	}

	XClockImpl(const XClockImpl& from)
	{
		memcpy(&m_start, &from.m_start, sizeof(m_start));
		memcpy(&m_last, &from.m_last, sizeof(m_last));
		m_start2 = from.m_start2;
		m_last2 = from.m_last2;
	}

	XClockImpl& operator= (const XClockImpl& from)
	{
		if (this != &from)
		{
			memcpy(&m_start, &from.m_start, sizeof(m_start));
			memcpy(&m_last, &from.m_last, sizeof(m_last));
			m_start2 = from.m_start2;
			m_last2 = from.m_last2;
		}
		return *this;
	}

	~XClockImpl()
	{
		// empty
	}

	void reset()
	{
		if (XClock_numberOfProcessors <= 1)
		{
			VERIFY(QueryPerformanceCounter(&m_start));
			memcpy(&m_last, &m_start, sizeof(m_last));
		}
		else
		{
			m_start2 = (uint64)timeGetTime();
			m_last2 = m_start2;
		}
	}

	XTimeSpan peek()
	{
		if (XClock_numberOfProcessors <= 1)
		{
			ASSERT(XClock_result && (XClock_frequency.QuadPart > 0));

			LARGE_INTEGER now;
			VERIFY(QueryPerformanceCounter(&now));
			ASSERT(now.QuadPart >= m_start.QuadPart);

			int64 usec = (int64)((now.QuadPart - m_start.QuadPart) * 1000000 / XClock_frequency.QuadPart);
			return XTimeSpan(usec / 1000000, usec % 1000000);
		}
		else
		{
			uint64 now = (uint64)timeGetTime();
			if (now < m_start2) now = m_start2 + 0X100000000; // timeGetTime计时溢出了

			uint64 msec = now - m_start2;
			return XTimeSpan(msec / 1000, (msec % 1000) * 1000);
		}
	}

	XTimeSpan diff_last()
	{
		if (XClock_numberOfProcessors <= 1)
		{
			ASSERT(XClock_result && (XClock_frequency.QuadPart > 0));

			LARGE_INTEGER now;
			VERIFY(QueryPerformanceCounter(&now));
			ASSERT(now.QuadPart >= m_last.QuadPart);

			int64 usec = (int64)((now.QuadPart - m_last.QuadPart) * 1000000 / XClock_frequency.QuadPart);
			memcpy(&m_last, &now, sizeof(now));
			return XTimeSpan(usec / 1000000, usec % 1000000);
		}
		else
		{
			uint64 now = (uint64)timeGetTime();
			if (now < m_start2) now = m_start2 + 0X100000000; // timeGetTime计时溢出了

			uint64 msec = now - m_last2;
			m_last2 = now;
			return XTimeSpan(msec / 1000, (msec % 1000) * 1000);
		}
	}

private:
	LARGE_INTEGER  m_start;
	LARGE_INTEGER  m_last;
	uint64 m_start2;
	uint64 m_last2;
};

#endif//__WINDOWS__

///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

class XClock::XClockImpl
{
public:
	XClockImpl()
	{
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &m_start));
		memcpy(&m_last, &m_start, sizeof(m_last));
	}

	XClockImpl(const XClockImpl& from)
	{
		memcpy(&m_start, &from.m_start, sizeof(m_start));
		memcpy(&m_last, &from.m_last, sizeof(m_last));
	}

	XClockImpl& operator= (const XClockImpl& from)
	{
		if (this != &from)
		{
			memcpy(&m_start, &from.m_start, sizeof(m_start));
			memcpy(&m_last, &from.m_last, sizeof(m_last));
		}
		return *this;
	}

	~XClockImpl()
	{
		// empty
	}

	void reset()
	{
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &m_start));
		memcpy(&m_last, &m_start, sizeof(m_last));
	}

	XTimeSpan peek()
	{
		timespec now;
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &now));
		ASSERT(now.tv_sec >= m_start.tv_sec);

		int64 nsec = now.tv_sec - m_start.tv_sec;
		nsec = nsec * 1000000000 + now.tv_nsec - m_start.tv_nsec;
		return XTimeSpan(nsec / 1000000000, (nsec % 1000000000) / 1000);
	}

	XTimeSpan diff_last()
	{
		timespec now;
		VERIFY(!clock_gettime(CLOCK_MONOTONIC, &now));
		ASSERT(now.tv_sec >= m_last.tv_sec);

		int64 nsec = now.tv_sec - m_last.tv_sec;
		nsec = nsec * 1000000000 + now.tv_nsec - m_last.tv_nsec;
		memcpy(&m_last, &now, sizeof(now));
		return XTimeSpan(nsec / 1000000000, (nsec % 1000000000) / 1000);
	}

private:
	timespec  m_start;
	timespec  m_last;
};

#endif//__GNUC__

///////////////////////////////////////////////////////////////////////////////
XClock::XClock()
	: m_impl(new(nothrow) XClockImpl)
{
	// empty
}

XClock::XClock(const XClock& from)
	: m_impl(new(nothrow) XClockImpl(*from.m_impl))
{
	// empty
}

XClock& XClock::operator=(const XClock& from)
{
	if (this != &from)
	{
		*m_impl = *from.m_impl;
	}
	return *this;
}

XClock::~XClock()
{
}

void XClock::reset()
{
	m_impl->reset();
}

XTimeSpan XClock::peek()
{
	return m_impl->peek();
}

XTimeSpan XClock::diff_last()
{
	return m_impl->diff_last();
}

///////////////////////////////////////////////////////////////////////////////

static XClock __ServerStartClock;

XTimeSpan running_time(void)
{
	return __ServerStartClock.peek();
}

} // namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"
#include "xcore_thread.h"

namespace xcore
{

bool xcore_test_clock()
{
	XClock clock1;
	printf("xcore running time is:%lld usecs.\n", (long long)xcore::running_time().to_usecs());
	xcore::sleep(1000);
	printf("sleep 1 second, clock1 record is:%lld usecs.\n", (long long)clock1.peek().to_usecs());
	printf("xcore running time is:%lld usecs.\n", (long long)xcore::running_time().to_usecs());
	clock1.reset();
	printf("clock1 reset, clock record is:%lld usecs.\n", (long long)clock1.peek().to_usecs());

	XClock clock2(clock1);
	printf("clock2 record is: %lld usecs.\n", (long long)clock2.peek().to_usecs());
	clock2 = clock1;
	printf("clock2 record is: %lld usecs.\n", (long long)clock2.peek().to_usecs());

	long long start = xcore::running_time().to_usecs();
	printf("server running is %lld usecs\n", (long long)start);
	xcore::sleep(1000);
	long long now = xcore::running_time().to_usecs();
	printf("server running is %lld usecs\n", (long long)now);
	printf("server running diff is %lld usecs\n", now - start);
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
