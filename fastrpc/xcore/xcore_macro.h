

#ifndef _XCORE_MACRO_H_
#define _XCORE_MACRO_H_

#include "xcore_define.h"

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

///////////////////////////////////////////////////////////////////////////////
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY
#undef SAFE_FREE
#undef SAFE_RELEASE
#ifdef __DEBUG__
#define SAFE_DELETE(x) { try { checked_delete(x); } catch(...) { ASSERT(false && "SAFE_DELETE error\n"); }  x = NULL; }
#define SAFE_DELETE_ARRAY(x) { try { checked_array_delete(x); } catch(...) { ASSERT(false && "SAFE_DELETE error\n"); } x = NULL; }
#define SAFE_FREE(x) { try { free(x); } catch(...) { ASSERT(false && "SAFE_DELETE error\n"); } x = NULL; }
#define SAFE_RELEASE(x) { try { if (x) { x->release(); } } catch(...) { ASSERT(false && "SAFE_DELETE error\n"); } x = NULL; }
#else//__DEBUG__
#define SAFE_DELETE(x) { try { delete x; } catch(...) { } x = NULL; }
#define SAFE_DELETE_ARRAY(x) { try { delete[] x; } catch(...) { } x = NULL; }
#define SAFE_FREE(x) { try { free(x); } catch(...) { } x = NULL; }
#define SAFE_RELEASE(x) { try { if (x) { x->release(); } } catch(...) { } x = NULL; }
#endif//__DEBUG__

///////////////////////////////////////////////////////////////////////////////

#define COUNT_OF_ARRAY(array) (sizeof(array) / sizeof(array[0]))


#define X_PROCESS_ERROR(Condition) \
	do  \
	{   \
		if (!(Condition))       \
		{                       \
			goto Exit0;         \
		}                       \
	} while (false)

#define X_PROCESS_SUCCESS(Condition) \
	do  \
	{   \
		if (Condition)          \
		{                       \
			goto Exit1;         \
		}                       \
	} while (false)

#define X_PROCESS_ERROR_RET(Condition, Code) \
	do  \
	{   \
		if (!(Condition))       \
		{                       \
			nResult = Code;     \
			goto Exit0;         \
		}                       \
	} while (false)

#define X_COM_PROCESS_ERROR(Condition) \
	do  \
	{   \
		if (FAILED(Condition))  \
		{                       \
			goto Exit0;         \
		}                       \
	} while (false)


#define X_COM_PROCESS_SUCCESS(Condition)   \
	do  \
	{   \
		if (SUCCEEDED(Condition))   \
		{                           \
			goto Exit1;             \
		}                           \
	} while (false)


#define X_COM_PROCESS_ERROR_RET(Condition, Code)     \
	do  \
	{   \
		if (FAILED(Condition))      \
		{                           \
			hrResult = Code;        \
			goto Exit0;             \
		}                           \
	} while (false)

}//namespace xcore

using namespace xcore;

#endif//_XCORE_MACRO_H_
