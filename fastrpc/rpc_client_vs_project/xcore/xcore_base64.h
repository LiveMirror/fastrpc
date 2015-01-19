// 2009-06-11
// xcore_base64.h
// 
// base64 ±à½âÂë


#ifndef _XCORE_BASE_H_
#define _XCORE_BASE_H_

#include "xcore_define.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XBase64
////////////////////////////////////////////////////////////////////////////////
class XBase64
{
public:
	// not contain '\0' char
	static uint32 cacl_encode_len(uint32 origin_len);
	static uint32 cacl_decode_len(uint32 base64code_len);

	// return: > 0 encode or decode result byte size; <= 0 is error
	static int encode(string& base64code, const void* origin, uint32 origin_len);
	static int encode(char* base64code, uint32 base64code_len, const void* origin, uint32 origin_len);

	static int decode(void* origin, uint32 origin_len, const string& base64code);
	static int decode(void* origin, uint32 origin_len, const char* base64code, uint32 base64code_len);
}; 

}//namespace xcore

using namespace xcore;

#endif//_XCORE_BASE_H_
