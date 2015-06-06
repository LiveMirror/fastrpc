// 2008-02-29
// xcore_memory.h
// 
// 自定义内存管理


#ifndef _XCORE_MEMORY_H_
#define _XCORE_MEMORY_H_

#include "xcore_define.h"
#include "xcore_smart_ptr.h"

namespace xcore {

////////////////////////////////////////////////////////////////////////////////
// IXBuffer
////////////////////////////////////////////////////////////////////////////////
class IXBuffer
{
protected:
	virtual ~IXBuffer() {};

public:
	// get current usable buffer size
	virtual uint32 size() const = 0;

	// get current data pointer of buffer
	virtual void* data() const = 0;

	// set size() to range of [0, capacity()]
	virtual void resize(uint32 newSize) = 0;
	
	// get prepositive reserve size of buffer
	virtual uint32 size_pre_reserve() const = 0;

	// reset prepositive reserve size of buffer
	virtual void resize_pre_reserve(uint32 newSize) = 0;

	// get the usable max size of buffer
	virtual uint32 capacity() const = 0;

	// restore buffer to original state
	virtual void restore() = 0;

	// clone a new buffer from this buffer
	virtual shared_ptr<IXBuffer> clone() = 0;
};

shared_ptr<IXBuffer> create_buffer(uint32 uSize);

void* memory_alloc(uint32 uSize);

void memory_free(void* pData);

} // namespace xcore

using namespace xcore;

#endif//_XCORE_MEMORY_H_

