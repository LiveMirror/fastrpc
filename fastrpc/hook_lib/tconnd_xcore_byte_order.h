// 
// ×Ö½ÚÐò×ª»»


#ifndef _TCONND_XCORE_BYTE_ORDER_H_
#define _TCONND_XCORE_BYTE_ORDER_H_

#include "tconnd_xcore_define.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XByteOrder
////////////////////////////////////////////////////////////////////////////////
class XByteOrder
{
public:
	enum TYPE
    {
		ENDIAN_UNKNOWN = 0,
		ENDIAN_BIG = 1,
		ENDIAN_LITTLE = 2
	};

	static TYPE native_type();

	static uint16 net2local(uint16 val);
	static uint32 net2local(uint32 val);
	static uint64 net2local(uint64 val);

	static uint16 local2net(uint16 val);
	static uint32 local2net(uint32 val);
	static uint64 local2net(uint64 val);
};

}//namespace xcore

using namespace xcore;

#endif//_XCORE_BYTE_ORDER_H_
