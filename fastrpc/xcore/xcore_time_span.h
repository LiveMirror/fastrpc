// 20010-09-29
// xcore_time_span.h
// 
//  ±º‰∂Œ¿‡


#ifndef _XCORE_TIME_SPAN_H_
#define _XCORE_TIME_SPAN_H_

#include "xcore_define.h"

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
