// 2008-03-06
// xcore_time.h
// 
//  ±º‰¿‡


#ifndef _XCORE_TIME_H_
#define _XCORE_TIME_H_

#include "xcore_define.h"
#include "xcore_time_span.h"
#include <time.h>

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XTime
///////////////////////////////////////////////////////////////////////////////
class XTime
{
public:
	enum Style
	{
		YMDHMS_,		/* 2010-9-29 9:12:8 */
		YMDHMS_0,		/* 2010-09-29 09:12:08 */
		YMD_,			/* 2010-9-29 */
		YMD_0,			/* 2010-09-29 */
		HMS_,			/* 9:12:8 */
		HMS_0,			/* 09:12:08 */
		YMDHMS,			/* 20100929091208 */
		YMD,			/* 20100929 */
		HMS,			/* 091208 */
		MD,				/* 0929 */
		YMDWHMS			/* 2010092903091208 */
	};

	static const XTime ZeroTime;
	static const XTime StartTime; // the process start time
	static const XTime MaxTime;
	static const XTime ErrorTime;

	// support two time format
	// format 1: 2008-03-12 15:01:12
	// format 2: 20080312150112
	static XTime try_parse(const string& strTime);

public:
	XTime(void);
	XTime(const XTime& other);
	XTime(time_t sec, long usec = 0);
	XTime(const timeval& tv);
	XTime(const tm& tm_);
	XTime(const XTimeSpan& ts);
	XTime(double d);
	~XTime(void);

	void set(time_t sec, long usec = 0);
	void set(const timeval& tv);
	void set(const tm& t);
	void set(const XTimeSpan& t);
	void set(double d);

	// seconds since 1970-01-01 00:00:00
	time_t sec(void) const { return m_sec; }
	// microseconds since the second
	long usec(void) const { return m_usec; }
	// seconds after the minute - [0,59]
	int local_sec(void) const { return m_tm.tm_sec; }
	// minutes after the hour - [0,59]
	int local_min(void) const { return m_tm.tm_min; }
	// hours since midnight - [0,23]
	int local_hour(void) const { return m_tm.tm_hour; }
	// day of the month - [1,31]
	int local_mday(void) const { return m_tm.tm_mday; }
	// months since January - [1,12]
	int local_mon(void) const { return m_tm.tm_mon + 1; }
	// years - [1900, ]
	int local_year(void) const { return m_tm.tm_year + 1900; }
	// days since Sunday - [0,6]
	int local_wday(void) const { return m_tm.tm_wday; }
	// days since January 1 - [0,365]
	int local_yday(void) const { return m_tm.tm_yday; }

	bool   has_error() const;
	uint32 to_date() const;  // such as 20110915
	string to_str(int style = YMDHMS_0) const;

public:
	operator tm() const { return m_tm; }
	operator XTimeSpan() const { return XTimeSpan(m_sec, m_usec); }
	operator const tm*() const { return &m_tm; }
	operator double() const { return (double)XTimeSpan(m_sec, m_usec); }

	XTime& operator = (const XTime& t);
	XTime& operator = (const timeval& tv);
	XTime& operator = (const tm& t);
	XTime& operator = (const time_t& t);
	XTime& operator = (const XTimeSpan& ts);
	XTime& operator = (double d);
	
	XTime& operator += (const XTimeSpan& ts);
	XTime& operator -= (const XTimeSpan& ts);

	friend bool operator <  (const XTime& t1, const XTime& t2);
	friend bool operator >  (const XTime& t1, const XTime& t2);
	friend bool operator <= (const XTime& t1, const XTime& t2);
	friend bool operator >= (const XTime& t1, const XTime& t2);
	friend bool operator == (const XTime& t1, const XTime& t2);
	friend bool operator != (const XTime& t1, const XTime& t2);
	friend XTimeSpan operator - (const XTime& t1, const XTime& t2);

private:
	void _fill_error_time();

private:
	time_t           m_sec;  // local UTC time(second)
	long             m_usec; // local UTC time(microsecond)
	struct tm        m_tm;   // local time calendar
};

///////////////////////////////////////////////////////////////////////////////

// get local timezone(-12 ~ 13)
int timezone();

} // namespace xcore

using namespace xcore;

#endif//_XCORE_TIME_H_

