// 2009-06-11
// xcore_base64.cpp
// 
// base64 ±à½âÂë


#include "xcore_base64.h"

namespace xcore {

#define B0(a) (a & 0xFF)
#define B1(a) ((a >> 8) & 0xFF)
#define B2(a) ((a >> 16) & 0xFF)
#define B3(a) ((a >> 24) & 0xFF)

static char __base64_get_char(uint32 index)
{
	static char szBase64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	if ((index >= 0) && (index < 64)) return szBase64Table[index];
	return '=';
}

static uint8 __base64_get_index(char ch)
{
	static uint8 indextable[128] = {
		0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
		0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
		0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
		0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
		0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
		0XFF, 0XFF, 0XFF, 0X3E, 0XFF, 0XFF, 0XFF, 0X3F,  // +, /
		0X34, 0X35, 0X36, 0X37, 0X38, 0X39, 0X3A, 0X3B,  // 0~9
		0X3C, 0X3D, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
		0XFF, 0X00, 0X01, 0X02, 0X03, 0X04, 0X05, 0X06,  // A~Z
		0X07, 0X08, 0X09, 0X0A, 0X0B, 0X0C, 0X0D, 0X0E,
		0X0F, 0X10, 0X11, 0X12, 0X13, 0X14, 0X15, 0X16,
		0X17, 0X18, 0X19, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
		0XFF, 0X1A, 0X1B, 0X1C, 0X1D, 0X1E, 0X1F, 0X20,  // a~z
		0X21, 0X22, 0X23, 0X24, 0X25, 0X26, 0X27, 0X28,
		0X29, 0X2A, 0X2B, 0X2C, 0X2D, 0X2E, 0X2F, 0X30,
		0X31, 0X32, 0X33, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF
	};

	if (ch < 0) return 0XFF;
	return indextable[(int)ch];
}

uint32 XBase64::cacl_encode_len(uint32 origin_len)
{
	return ((origin_len + 2) / 3) * 4;
}

uint32 XBase64::cacl_decode_len(uint32 base64code_len)
{
	return ((base64code_len + 3) / 4) * 3;
}

int XBase64::encode(string& base64code, const void* origin, uint32 origin_len)
{
	base64code.clear();
	if (origin == NULL || origin_len == 0) return 0;
	base64code.assign(cacl_encode_len(origin_len), '=');
	int ret = encode((char *)base64code.c_str(), (uint32)base64code.size() + 1, origin, origin_len);
	if (ret <= 0) base64code.clear();
	return ret;
}

int XBase64::encode(char* base64code, uint32 base64code_len, const void* origin, uint32 origin_len)
{
	if (base64code == NULL || origin == NULL || origin_len == 0) return 0;
	if (base64code_len < cacl_encode_len(origin_len) + 1) return -1;

	uint8 *psrc = (uint8 *)origin;
	uint8 *pdst = (uint8 *)base64code;
	uint32 rest = origin_len;
	for ( ; rest >= 3; rest -= 3)
	{
		uint32 ulTmp = *(uint32 *)psrc;
		*pdst++ = __base64_get_char((B0(ulTmp) >> 2) & 0x3F);
		*pdst++ = __base64_get_char((B0(ulTmp) << 4 | B1(ulTmp) >> 4) & 0x3F);
		*pdst++ = __base64_get_char((B1(ulTmp) << 2 | B2(ulTmp) >> 6) & 0x3F);
		*pdst++ = __base64_get_char(B2(ulTmp) & 0x3F);
		psrc += 3;
	}

	if (rest > 0)
	{
		uint32 ulTmp = *(uint32 *)psrc;
		uint32 step = 32 - rest * 8;
		ulTmp = (ulTmp << step >> step);

		*pdst++ = __base64_get_char((B0(ulTmp) >> 2) & 0x3F);
		*pdst++ = __base64_get_char((B0(ulTmp) << 4 | B1(ulTmp) >> 4) & 0x3F);
		*pdst++ = (rest == 2) ? __base64_get_char((B1(ulTmp) << 2 | B2(ulTmp) >> 6) & 0x3F) : '=';
		*pdst++ = '=';
	}
	*pdst = '\0';
	return cacl_encode_len(origin_len);
}

int XBase64::decode(void* origin, uint32 origin_len, const string& base64code)
{
	return decode(origin, origin_len, (const char*)base64code.c_str(), (uint32)base64code.size());
}

int XBase64::decode(void* origin, uint32 origin_len, const char* base64code, uint32 base64code_len)
{
	if (origin == NULL || base64code == NULL) return 0;
	if (base64code_len == 0) base64code_len = (uint32)strlen(base64code);
	if (origin_len < cacl_decode_len(base64code_len)) return -1;

	uint32 *psrc = (uint32 *)base64code;
	uint8 *pdst = (uint8 *)origin;
	uint32 rest = base64code_len;
	for ( ; rest > 4; rest -= 4)
	{
		uint32 ulTmp = *psrc++;
		*pdst++ = (__base64_get_index((char)B0(ulTmp)) << 2 | __base64_get_index((char)B1(ulTmp)) >> 4);
		*pdst++ = (__base64_get_index((char)B1(ulTmp)) << 4 | __base64_get_index((char)B2(ulTmp)) >> 2);
		*pdst++ = (__base64_get_index((char)B2(ulTmp)) << 6 | __base64_get_index((char)B3(ulTmp)));
	}

	if (rest > 0)
	{
		uint32 ulTmp = *psrc;
		uint32 step = 32 - rest * 8;
		ulTmp = (ulTmp << step >> step);

		if ((char)B0(ulTmp) == '\0' || (char)B0(ulTmp) == '=') return -1;
		if ((char)B1(ulTmp) == '\0' || (char)B1(ulTmp) == '=') return -1;
		*pdst++ = (__base64_get_index((char)B0(ulTmp)) << 2 | __base64_get_index((char)B1(ulTmp)) >> 4);
		if ((char)B2(ulTmp) == '\0' || (char)B2(ulTmp) == '=') return cacl_decode_len(base64code_len) - 2;
		*pdst++ = (__base64_get_index((char)B1(ulTmp)) << 4 | __base64_get_index((char)B2(ulTmp)) >> 2);
		if ((char)B3(ulTmp) == '\0' || (char)B3(ulTmp) == '=') return cacl_decode_len(base64code_len) - 1;
		*pdst++ = (__base64_get_index((char)B2(ulTmp)) << 6 | __base64_get_index((char)B3(ulTmp)));
	}
	return cacl_decode_len(base64code_len);
}

}//namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_base64()
{
	int ret = 0;
	string str1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	string result1;
	ret = XBase64::encode(result1, (const void*)str1.c_str(), (uint32)str1.size());
	ASSERT(ret >= 0);
	ASSERT(ret == (int)result1.size());
	printf("result1 size:%d, base64code: %s\n", ret, result1.c_str());
	char *origin1 = new char[XBase64::cacl_decode_len((uint32)result1.size()) + 1];
	ret = XBase64::decode(origin1, XBase64::cacl_decode_len((uint32)result1.size()), result1);
	ASSERT(ret > 0);
	origin1[ret] = '\0';
	ASSERT(str1 == origin1);

	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
