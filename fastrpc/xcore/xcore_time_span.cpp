// 20010-09-29
// xcore_time_span.cpp
// 
// Ê±¼ä¶ÎÀà


#include "xcore_time_span.h"

namespace xcore {
	
///////////////////////////////////////////////////////////////////////////////
// class XTimeSpan
////////////////////////////////////////////////////////////////////////////////
XTimeSpan::XTimeSpan()
	: m_sec(0)
	, m_usec(0)
{
	// empty
}

XTimeSpan::XTimeSpan(const XTimeSpan& ts)
{
	m_sec = ts.m_sec;
	m_usec = ts.m_usec;
}

XTimeSpan::XTimeSpan(int64 sec, int64 usec)
{
	set(sec, usec);
}

XTimeSpan::XTimeSpan(const timeval& tv)
{
	set(tv.tv_sec, tv.tv_usec);
}

XTimeSpan::XTimeSpan(double d)
{
	set(d);
}

XTimeSpan::~XTimeSpan()
{
	// empty
}

XTimeSpan& XTimeSpan::set(int64 sec, int64 usec)
{
	m_sec = sec;
	m_usec = usec;
	_normalize();
	return *this;
}

XTimeSpan& XTimeSpan::set(double d)
{
	m_sec = (int64)d;
	if (d > 0.0)
		m_usec = (int64)((d - (double)m_sec) * 1000000.0 + 0.5);
	else
		m_usec = (int64)((d - (double)m_sec) * 1000000.0 - 0.5);
	_normalize();
	return *this;
}

XTimeSpan& XTimeSpan::set(const timeval& tv)
{
	set(tv.tv_sec, tv.tv_usec);
	return *this;
}

XTimeSpan& XTimeSpan::from_usecs(int64 usec)
{
	m_sec = usec / 1000000;
	m_usec = usec % 1000000;
	return *this;
}

XTimeSpan& XTimeSpan::from_msecs(int64 msec)
{
	m_sec = msec / 1000;
	m_usec = (msec % 1000) * 1000;
	return *this;
}

int64 XTimeSpan::to_usecs() const
{
	return m_sec * 1000000 + m_usec;
}

int64 XTimeSpan::to_msecs() const
{
	int64 msec = m_sec;
	msec *= 1000;
	if (m_usec > 0)
		msec += ((m_usec + 500) / 1000);
	else
		msec += ((m_usec - 500) / 1000);
	return msec;
}

int64 XTimeSpan::to_secs() const
{
	int64 sec = m_sec;
	if (m_usec > 0)
		sec += ((m_usec + 500000) / 1000000);
	else
		sec += ((m_usec - 500000) / 1000000);
	return sec;
}

string XTimeSpan::to_str() const
{
	char buf[64];
	sprintf(buf, "%.6f", (double)*this);
	return buf;
}

XTimeSpan::operator double() const
{
	double d = (double)m_usec;
	d /= 1000000.0;
	d += (double)m_sec;
	return d;
}

XTimeSpan& XTimeSpan::operator += (const XTimeSpan& t)
{
	this->set(this->m_sec + t.m_sec, this->m_usec + t.m_usec);
	return *this;
}

XTimeSpan& XTimeSpan::operator -= (const XTimeSpan& t)
{
	this->set(this->m_sec - t.m_sec, this->m_usec - t.m_usec);
	return *this;
}

XTimeSpan& XTimeSpan::operator = (const XTimeSpan& t)
{
	if (this != &t)
	{
		m_sec = t.m_sec;
		m_usec = t.m_usec;
	}
	return *this;
}

void XTimeSpan::_normalize()
{
	m_sec += m_usec / 1000000;
	m_usec %= 1000000;
	if (m_sec > 0 && m_usec < 0)
	{
		m_sec--;
		m_usec += 1000000;
	}
	else if (m_sec < 0 && m_usec > 0)
	{
		m_sec++;
		m_usec -= 1000000;
	}
	return;
}


 bool operator < (const XTimeSpan& t1, const XTimeSpan& t2)
 {
 	return ((t1.m_sec < t2.m_sec) ||
 			((t1.m_sec == t2.m_sec) && (t1.m_usec < t2.m_usec)));
 }

bool operator > (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec > t2.m_sec) ||
			((t1.m_sec == t2.m_sec) && (t1.m_usec > t2.m_usec)));
}

bool operator <= (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec < t2.m_sec) ||
			((t1.m_sec == t2.m_sec) && (t1.m_usec <= t2.m_usec)));
}

bool operator >= (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec > t2.m_sec) ||
			((t1.m_sec == t2.m_sec) && (t1.m_usec >= t2.m_usec)));
}

bool operator == (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec == t2.m_sec) && (t1.m_sec == t2.m_sec));
}

bool operator != (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return ((t1.m_sec != t2.m_sec) || (t1.m_sec != t2.m_sec));
}

XTimeSpan operator + (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return XTimeSpan(t1.m_sec + t2.m_sec, t1.m_usec + t2.m_usec);
}

XTimeSpan operator - (const XTimeSpan& t1, const XTimeSpan& t2)
{
	return XTimeSpan(t1.m_sec - t2.m_sec, t1.m_usec - t2.m_usec);
}

XTimeSpan operator * (double d, const XTimeSpan& t)
{
	return XTimeSpan((double)t * d);
}

XTimeSpan operator * (const XTimeSpan& t, double d)
{
	return XTimeSpan((double)t * d);
}

XTimeSpan operator / (const XTimeSpan& t, double d)
{
	ASSERT(!(d >= -0.0001 && d <= 0.0001));
	return XTimeSpan((double)t / d);
}

}//namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_time_span()
{
	timeval val = {23, 34};
	XTimeSpan ts1;
	XTimeSpan ts2(100, 340);
	XTimeSpan ts3(ts2);
	XTimeSpan ts4((double)ts2);
	XTimeSpan ts5(val);
	ASSERT(ts1.to_str() == "0.000000");
	ASSERT(ts2.to_str() == "100.000340");
	ASSERT(ts3.to_str() == "100.000340");
	ASSERT(ts4.to_str() == "100.000340");
	ASSERT(ts5.to_str() == "23.000034");

	ts1.set(234, -574924455);
	ASSERT(ts1.to_str() == "-340.924455");
	ts1.set(val);
	ASSERT(ts1.to_str() == "23.000034");
	ts1.set(345.52462);
	ASSERT(ts1.to_str() == "345.524620");
	ts1.set(345.52462378);
	ASSERT(ts1.to_str() == "345.524624");
	printf("ts1 is %.6f\n", (double)ts1);

	ASSERT(ts1.sec() == 345);
	ASSERT(ts1.usec() == 524624);
	ASSERT(ts1.to_secs() == 346);
	ASSERT(ts1.to_msecs() == 345525);
	ASSERT(ts1.to_usecs() == 345524624);
	ASSERT(ts1.to_msecs() == ts2.from_msecs(ts1.to_msecs()).to_msecs());
	ASSERT(ts1.to_usecs() == ts2.from_usecs(ts1.to_usecs()).to_usecs());

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
