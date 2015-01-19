// 2008-06-10
// xcore_crc.h
// 
// CRCºÏ—È¿‡


#ifndef _XCORE_CRC_H_
#define _XCORE_CRC_H_

#include "xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// crc functions
///////////////////////////////////////////////////////////////////////////////
//uint32 crc_32(const void* src, uint32 len, uint32 checksum = 0);  // check

//uint16 crc_16(const void* src, uint32 len, uint16 checksum = 0);  // check

uint16 crc_ccitt(const void* src, uint32 len, uint16 checksum = 0);  // ok

// ∑¥–Úccitt
//uint16 crc_ccitt_rev(const void* src, uint32 len, uint16 checksum = 0);  // check

} // namespace xcore

using namespace xcore;

#endif//_XCORE_CRC_H_
