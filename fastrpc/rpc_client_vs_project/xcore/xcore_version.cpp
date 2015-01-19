// 2011-11-14
// xcore_version.cpp
// 
// xcoreø‚∞Ê±æ–≈œ¢


#include "xcore_version.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// get version function
///////////////////////////////////////////////////////////////////////////////

void xcore_version(uint32& majorVer, uint32& subVer, string& info)
{
	static char* ver_info = "xcore library version 1.0.0 build at "__TIME__ " " __DATE__ ;
	majorVer = 1;
	subVer   = 0;
	info = ver_info;
}

}//namespace xcore

////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_version()
{
	uint32 majorVer, subVer;
	string info;
	xcore_version(majorVer, subVer, info);
	printf("majorVer:%u, subVer:%u, info:%s\n", majorVer, subVer, info.c_str());
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
