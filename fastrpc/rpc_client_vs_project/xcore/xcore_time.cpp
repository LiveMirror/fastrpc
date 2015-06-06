// 2008-03-06
// xcore_time.cpp
// 
// Ê±¼äÀà


#include "xcore_time.h"

namespace xcore {

// time_t --> struct tm
static bool XTime_LocalTime(struct tm &_Tm, const time_t &_Time)
{
	#ifdef __WINDOWS__
	return (0 == localtime_s(&_Tm, &_Time));
	#endif//__WINDOWS__

	#ifdef __GNUC__
	return (NULL != localtime_r(&_Time, &_Tm));
	#endif// __GNUC__
}

///////////////////////////////////////////////////////////////////////////////
// class XTime
///////////////////////////////////////////////////////////////////////////////
const XTime XTime::ZeroTime(0, 0);

const XTime XTime::StartTime;

const XTime XTime::MaxTime((time_t)0x7fffffff, 999999);  

const XTime XTime::ErrorTime(-1.1);

XTime::XTime(void)
{
	#ifdef __WINDOWS__
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	m_tm.tm_year  = st.wYear - 1900;
	m_tm.tm_mon   = st.wMonth - 1;
	m_tm.tm_mday  = st.wDay;
	m_tm.tm_hour  = st.wHour;
	m_tm.tm_min   = st.wMinute;
	m_tm.tm_sec   = st.wSecond;
	m_tm.tm_isdst = -1;
	m_sec         = mktime(&m_tm);
	m_usec        = st.wMilliseconds * 1000;
	#endif//__WINDOWS__

	#ifdef __GNUC__
	struct timeval tv_ = {};
	int ret = gettimeofday(&tv_, NULL);
	ASSERT(0 == ret);
	m_sec = tv_.tv_sec;
	m_usec = tv_.tv_usec;
	bool bRet = XTime_LocalTime(m_tm, m_sec);
	ASSERT(bRet);
	#endif//__GNUC__
}

XTime::XTime(const XTime &other)
{
	memcpy(this, &other, sizeof(XTime));
};

XTime::XTime(time_t sec, long usec)
{
	set(sec, usec);
}

XTime::XTime(const timeval& tv)
{
	set(tv.tv_sec, tv.tv_usec);
}

XTime::XTime(const tm &tm_)
{
	set(tm_);
}

XTime::XTime(const XTimeSpan& ts)
{
	set(ts);
}

XTime::XTime(double d)
{
	set(d);
}

XTime::~XTime(void)
{
	// empty
}

void XTime::set(time_t sec, long usec)
{
	sec += usec / 1000000;
	usec %= 1000000;
	if (sec > 0 && usec < 0)
	{
		sec--;
		usec += 1000000;
	}
	else if (sec < 0 && usec > 0)
	{
		sec++;
		usec -= 1000000;
	}
	if (usec < 0 || !XTime_LocalTime(m_tm, sec))
	{
		_fill_error_time();
		return;
	}

	m_sec = sec;
	m_usec = usec;
	return;
}

void XTime::set(const timeval& tv)
{
	set(tv.tv_sec, tv.tv_usec);
}

void XTime::set(const tm& tm_)
{
	m_sec = mktime((struct tm *)&tm_);
	m_usec = 0;
	m_tm = tm_;
	return;
}

void XTime::set(const XTimeSpan& ts)
{
	set((time_t)ts.sec(), (long)ts.usec());
	return;
}

void XTime::set(double d)
{
	set(XTimeSpan(d));
	return;
}

XTime& XTime::operator = (const XTime& t)
{
	if (this != &t)
	{
		memcpy(this, &t, sizeof(t));
	}
	return *this;
}

XTime& XTime::operator = (const timeval& tv)
{
	set(tv.tv_sec, tv.tv_usec);
	return *this;
}

XTime& XTime::operator = (const tm& t)
{
	set(t);
	return *this;
}

XTime& XTime::operator = (const time_t& t)
{
	set(t, 0);
	return *this;
}

XTime& XTime::operator = (const XTimeSpan& ts)
{
	set(ts);
	return *this;
}

XTime& XTime::operator = (double d)
{
	set(d);
	return *this;
}

XTime& XTime::operator += (const XTimeSpan& ts)
{
	XTimeSpan t(m_sec, m_usec);
	t += ts;
	set(t);
	return *this;
}

XTime& XTime::operator -= (const XTimeSpan& ts)
{
	XTimeSpan t(m_sec, m_usec);
	t -= ts;
	set(t);
	return *this;
}

bool XTime::has_error() const
{
	return (m_sec == (time_t)-1);
}

uint32 XTime::to_date() const
{
	if (has_error()) return 0;
	return local_year() * 10000 + local_mon() * 100 + local_mday();
}

string XTime::to_str(int style) const
{
	char str[64];

	if (has_error())
	{
		return "0000-00-00 00:00:00";
	}

	switch (style)
	{
	case YMDHMS_:
		sprintf(str, "%d-%d-%d %d:%d:%d", 
			m_tm.tm_year + 1900, m_tm.tm_mon + 1, m_tm.tm_mday, 
			m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
		break;
	case YMD_:
		sprintf(str, "%d-%d-%d", 
			m_tm.tm_year + 1900, m_tm.tm_mon + 1, m_tm.tm_mday);
		break;
	case YMD_0:
		sprintf(str, "%04d-%02d-%02d", 
			m_tm.tm_year + 1900, m_tm.tm_mon + 1, m_tm.tm_mday);
		break;
	case HMS_:
		sprintf(str, "%d:%d:%d", 
			m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
		break;
	case HMS_0:
		sprintf(str, "%02d:%02d:%02d", 
			m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
		break;
	case YMDHMS:
		sprintf(str, "%04d%02d%02d%02d%02d%02d", 
			m_tm.tm_year + 1900, m_tm.tm_mon + 1, m_tm.tm_mday, 
			m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
		break;
	case YMD:
		sprintf(str, "%04d%02d%02d", 
			m_tm.tm_year + 1900, m_tm.tm_mon + 1, m_tm.tm_mday);
		break;
	case HMS:
		sprintf(str, "%02d%02d%02d", 
			m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
		break;
	case MD:
		sprintf(str, "%02d%02d", 
			m_tm.tm_mon + 1, m_tm.tm_mday);
		break;
	case YMDWHMS:
		sprintf(str, "%04d%02d%02d%02d%02d%02d%02d", 
			m_tm.tm_year + 1900, m_tm.tm_mon + 1, m_tm.tm_mday, 
			m_tm.tm_wday == 0 ? 7 : m_tm.tm_wday,
			m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
		break;
	case YMDHMS_0:
	default:
		sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", 
			m_tm.tm_year + 1900, m_tm.tm_mon + 1, m_tm.tm_mday, 
			m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
		break;
	}
	return str;
}

XTime XTime::try_parse(const string& strTime)
{
	struct tm tm_ = {};
	int ret = sscanf(strTime.c_str(), " %d - %d - %d %d : %d : %d", 
					&tm_.tm_year, &tm_.tm_mon, &tm_.tm_mday, &tm_.tm_hour, &tm_.tm_min, &tm_.tm_sec);
	if (ret != 6)
	{
		ret = sscanf(strTime.c_str(), "%4d%2d%2d%2d%2d%2d", 
					&tm_.tm_year, &tm_.tm_mon, &tm_.tm_mday, &tm_.tm_hour, &tm_.tm_min, &tm_.tm_sec);
		if (ret != 6)
		{
			return ErrorTime;
		}
	}

	if ((tm_.tm_year < 1900) ||
		(tm_.tm_mon == 0 || tm_.tm_mon > 12) ||
		(tm_.tm_mday == 0 || tm_.tm_mday > 31) ||
		(tm_.tm_hour > 23 || tm_.tm_min > 59 || tm_.tm_sec > 59))
	{
		return ErrorTime;
	}

	tm_.tm_year -= 1900;
	tm_.tm_mon -= 1;
	return XTime(tm_);
}

void XTime::_fill_error_time()
{
	m_sec = (time_t)-1;
	m_usec = 0;
	memset(&m_tm, 0, sizeof(m_tm));
	return;
}

bool operator < (const XTime& t1, const XTime& t2)
{
	return ((t1.m_sec < t2.m_sec) || 
			((t1.m_sec == t2.m_sec) && (t1.m_usec < t2.m_usec)));
}

bool operator > (const XTime& t1, const XTime& t2)
{
	return ((t1.m_sec > t2.m_sec) || 
			((t1.m_sec == t2.m_sec) && (t1.m_usec > t2.m_usec)));
}

bool operator <= (const XTime& t1, const XTime& t2)
{
	return ((t1.m_sec < t2.m_sec) || 
		((t1.m_sec == t2.m_sec) && (t1.m_usec <= t2.m_usec)));
}

bool operator >= (const XTime& t1, const XTime& t2)
{
	return ((t1.m_sec > t2.m_sec) || 
		((t1.m_sec == t2.m_sec) && (t1.m_usec >= t2.m_usec)));
}

bool operator == (const XTime& t1, const XTime& t2)
{
	return ((t1.m_sec == t2.m_sec) && (t1.m_usec == t2.m_usec));
}

bool operator != (const XTime& t1, const XTime& t2)
{
	return ((t1.m_sec != t2.m_sec) || (t1.m_usec != t2.m_usec));
}

XTimeSpan operator - (const XTime& t1, const XTime& t2)
{
	XTimeSpan ts1(t1.m_sec, t1.m_usec);
	XTimeSpan ts2(t2.m_sec, t2.m_usec);
	return ts1 - ts2;
}

///////////////////////////////////////////////////////////////////////////////

int timezone()
{
	struct tm tm1 = {};
	time_t t_time = 24 * 3600;

	XTime_LocalTime(tm1, t_time);
	return (tm1.tm_mday - 1) * 24 + tm1.tm_hour - 24;
}

} // namespace xcore

////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_time()
{
	printf("Local time zone is:%d\n", xcore::timezone());
	printf("ZeroTime is:%s\n", XTime::ZeroTime.to_str().c_str());
	printf("MaxTime is:%s\n", XTime::MaxTime.to_str().c_str());
	printf("ErrorTime is:%s\n", XTime::ErrorTime.to_str().c_str());
	XTime t = XTime::try_parse("2011-09-16 12:23:59");
	printf("time:%s, yday:%d, mday:%d, wday:%d\n", t.to_str().c_str(), t.local_yday(), t.local_mday(), t.local_wday());
	ASSERT(t.local_wday() == 5);
	ASSERT(t.local_yday() == 258);

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
