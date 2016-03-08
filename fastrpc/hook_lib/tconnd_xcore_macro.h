

#ifndef _TCONND_XCORE_MACRO_H_
#define _TCONND_XCORE_MACRO_H_

#include "tconnd_xcore_define.h"
#include <errno.h>

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
template<typename T> 
inline void checked_delete(T * x)
{
	// intentionally complex - simplification causes regressions
	typedef char type_must_be_complete[ sizeof(T) ? 1: -1 ];
	(void) sizeof(type_must_be_complete);
	delete x;
}

template<typename T> 
inline void checked_array_delete(T * x)
{
	typedef char type_must_be_complete[ sizeof(T) ? 1: -1 ];
	(void) sizeof(type_must_be_complete);
	delete [] x;
}

template<class T>
struct checked_deleter
{
	typedef void result_type;
	typedef T * argument_type;

	void operator()(T * x) const
	{
		checked_delete(x);
	}
};

template<class T>
struct checked_array_deleter
{
	typedef void result_type;
	typedef T * argument_type;

	void operator()(T * x) const
	{
		checked_array_delete(x);
	}
};

///////////////////////////////////////////////////////////////////////////////
#undef ASSERT
#undef VERIFY
#ifdef __DEBUG__
#ifdef __WINDOWS__
#define ASSERT(x) \
	do { \
		if (!(x)) { \
			fprintf(stderr, "Assertion ("#x") failed at %s(..) in %s:%d, LastError:%u\r\n", __FUNCTION__, __FILE__, __LINE__, ::GetLastError()); \
			fflush(stderr); \
			abort(); \
		} \
	} while (0)

#define VERIFY(x) \
	do { \
		if (!(x)) { \
			fprintf(stderr, "Verification ("#x") failed at %s(..) in %s:%d, LastError:%u\r\n", __FUNCTION__, __FILE__, __LINE__, ::GetLastError()); \
			fflush(stderr); \
			abort(); \
		} \
	} while (0)

#else//__WINDOWS__
#define ASSERT(x) \
	do { \
		if (!(x)) { \
			fprintf(stderr, "Assertion ("#x") failed at %s(..) in %s:%d, errno:%d\r\n", __FUNCTION__, __FILE__, __LINE__, errno); \
			fflush(stderr); \
			abort(); \
		} \
	} while (0)

#define VERIFY(x) \
	do { \
		if (!(x)) { \
			fprintf(stderr, "Verification ("#x") failed at %s(..) in %s:%d, errno:%d\r\n", __FUNCTION__, __FILE__, __LINE__, errno); \
			fflush(stderr); \
			abort(); \
		} \
	} while (0)
#endif//__WINDOWS__
#else//__DEBUG__
#define ASSERT(x)
#define VERIFY(x) (x)
#endif//__DEBUG__


#define COUNT_OF_ARRAY(array) (sizeof(array) / sizeof(array[0]))

}//namespace xcore

using namespace xcore;

#endif//_XCORE_MACRO_H_
