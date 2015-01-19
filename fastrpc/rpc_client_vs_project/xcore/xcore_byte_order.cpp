// 2009-08-03
// xcore_byte_order.cpp
// 
// ×Ö½ÚÐò×ª»»


#include "xcore_byte_order.h"

namespace xcore {

static uint16 __flip_bytes(uint16 val);
static uint32 __flip_bytes(uint32 val);
static uint64 __flip_bytes(uint64 val);
static XByteOrder::TYPE __get_native_byte_order();

////////////////////////////////////////////////////////////////////////////////
// class XByteOrder
////////////////////////////////////////////////////////////////////////////////
XByteOrder::TYPE XByteOrder::native_type()
{
	static TYPE __native_byte_order = __get_native_byte_order();
	return __native_byte_order;
}

uint16 XByteOrder::net2local(uint16 val)
{
	ASSERT(native_type() != ENDIAN_UNKNOWN);
	if (native_type() == ENDIAN_LITTLE)
	{
		return __flip_bytes(val);
	}
	return val;
}

uint32 XByteOrder::net2local(uint32 val)
{
	ASSERT(native_type() != ENDIAN_UNKNOWN);
	if (native_type() == ENDIAN_LITTLE)
	{
		return __flip_bytes(val);
	}
	return val;
}

uint64 XByteOrder::net2local(uint64 val)
{
	ASSERT(native_type() != ENDIAN_UNKNOWN);
	if (native_type() == ENDIAN_LITTLE)
	{
		return __flip_bytes(val);
	}
	return val;
}

uint16 XByteOrder::local2net(uint16 val)
{
	ASSERT(native_type() != ENDIAN_UNKNOWN);
	if (native_type() == ENDIAN_LITTLE)
	{
		return __flip_bytes(val);
	}
	return val;
}

uint32 XByteOrder::local2net(uint32 val)
{
	ASSERT(native_type() != ENDIAN_UNKNOWN);
	if (native_type() == ENDIAN_LITTLE)
	{
		return __flip_bytes(val);
	}
	return val;
}

uint64 XByteOrder::local2net(uint64 val)
{
	ASSERT(native_type() != ENDIAN_UNKNOWN);
	if (native_type() == ENDIAN_LITTLE)
	{
		return __flip_bytes(val);
	}
	return val;
}

///////////////////////////////////////////////////////////////////////////////

XByteOrder::TYPE __get_native_byte_order()
{
	#pragma pack(1)
	union
	{
		uint16 s;
		uint8 c[2];
	} un;
	#pragma pack()

	un.s = 0x0201;
	if ((2 == un.c[0]) && (1 == un.c[1]))
	{
		return XByteOrder::ENDIAN_BIG;
	}
	else if ((1 == un.c[0]) && (2 == un.c[1]))
	{
		return XByteOrder::ENDIAN_LITTLE;
	}
	return XByteOrder::ENDIAN_UNKNOWN;
}

uint16 __flip_bytes(uint16 val)
{
	return (((val >> 8) & 0x00FF) | ((val << 8) & 0xFF00));
}

uint32 __flip_bytes(uint32 val)
{
	return (((val >> 24) & 0x000000FF) | ((val >> 8) & 0x0000FF00)
		| ((val << 8) & 0x00FF0000) | ((val << 24) & 0xFF000000));
}

uint64 __flip_bytes(uint64 val)
{
	#pragma pack(1)
	union
	{
		uint64 s;
		uint8 c[8];
	} un1, un2;
	#pragma pack()
	
	un1.s = val;
	for (int i = 0; i < 8; i++)
	{
		un2.c[i] = un1.c[7 - i];
	}
	return un2.s;
}

}//namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_byte_order()
{
	XByteOrder::TYPE type_ = XByteOrder::native_type();
	if (type_ == XByteOrder::ENDIAN_BIG)
	{
		printf("Native type: ENDIAN_BIG\n");
	}
	else if (type_ == XByteOrder::ENDIAN_LITTLE)
	{
		printf("Native type: ENDIAN_LITTLE\n");
	}
	else
	{
		printf("Native type: ENDIAN_UNKNOWN\n");
		return false;
	}

	uint16 u1 = 0x1234;
	uint32 u2 = 0x12345678;
	uint64 u3 = 0x1234567812345678ULL;
	if (type_ == XByteOrder::ENDIAN_BIG)
	{
		ASSERT(XByteOrder::net2local(u1) == 0x1234);
		ASSERT(XByteOrder::net2local(u2) == 0x12345678);
		ASSERT(XByteOrder::net2local(u3) == 0x1234567812345678ULL);
		ASSERT(XByteOrder::local2net(u1) == 0x1234);
		ASSERT(XByteOrder::local2net(u2) == 0x12345678);
		ASSERT(XByteOrder::local2net(u3) == 0x1234567812345678ULL);
	}
	else
	{
		ASSERT(XByteOrder::net2local(u1) == 0x3412);
		ASSERT(XByteOrder::net2local(u2) == 0x78563412);
		ASSERT(XByteOrder::net2local(u3) == 0x7856341278563412ULL);
		ASSERT(XByteOrder::local2net(u1) == 0x3412);
		ASSERT(XByteOrder::local2net(u2) == 0x78563412);
		ASSERT(XByteOrder::local2net(u3) == 0x7856341278563412ULL);
	}
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
