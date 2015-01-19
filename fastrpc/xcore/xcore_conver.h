// 2011-10-11
// xcore_conver.h
// 
// ×Ö·û¼¯×ª»»

#ifndef _XCORE_CONVER_H_
#define _XCORE_CONVER_H_

#include "xcore_define.h"

namespace xcore {
	
///////////////////////////////////////////////////////////////////////////////
// character set conversion functions
///////////////////////////////////////////////////////////////////////////////

bool gbk_to_utf8(string& result, const string& str);

bool gbk_to_unicode(wstring& result, const string& str);

bool utf8_to_gbk(string& result, const string& str);

bool utf8_to_unicode(wstring& result, const string& str);

bool unicode_to_gbk(string& result, const wstring& str);

bool unicode_to_utf8(string& result, const wstring& str);

}//namespace xcore

using namespace xcore;

#endif//_XCORE_CONVER_H_
