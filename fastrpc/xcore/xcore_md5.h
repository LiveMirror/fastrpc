// 2007-08-01
// xcore_md5.h
// 
// MD5÷ß≥÷¿‡


#ifndef _XCORE_MD5_H_
#define _XCORE_MD5_H_

#include "xcore_define.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// class XMD5
////////////////////////////////////////////////////////////////////////////////
class XMD5
{
public:
	XMD5();

	void initialize(void);

	void update(const uint8 *input, uint32 inputLen);

	void final(uint8 (&digest)[16]);

	string final();

private:
	void _transform(const uint8 *block);

	void _encode(uint8 *output, const uint32 *input, uint32 inputLen);

	void _decode(uint32 *output, const uint8 *input, uint32 inputLen);

private:
	uint32 m_count[2];
	uint32 m_state[4];
	uint8  m_buffer[64];
};

///////////////////////////////////////////////////////////////////////////////

void md5(const void* src, uint32 size, uint8 (&digest)[16]);

string md5(const void* src, uint32 size);

} // namespace xcore

using namespace xcore;

#endif//_XCORE_MD5_H_
