// 2008-02-29
// xcore_memory.cpp
// 
// 自定义内存管理


#include "xcore_memory.h"
#include "xcore_critical.h"


namespace xcore {

#define MEMORY_BLOCK_MARK 'GNIQ'
#define MEMORY_BUFFER_RESERVE_SIZE   8

static uint32 g_uRose[] = 
{
	64,
	1   * 8 + 64,
	2   * 8 + 64,
	4   * 8 + 64,
	8   * 8 + 64,
	16  * 8 + 64,
	32  * 8 + 64,
	64  * 8 + 64,
	128 * 8 + 64, /*1  * 1024*/
	256 * 8 + 64, /*2  * 1024*/
	512 * 8 + 64, /*4  * 1024*/
	1024* 8 + 64, /*8  * 1024*/
	2048* 8 + 64, /*16 * 1024*/
	4096* 8 + 64, /*32 * 1024*/
	8192* 8 + 64  /*64 * 1024*/
};

struct XMemoryBlock;
struct XBlockLinkHead;

#pragma pack(1)
struct XBlockLinkHead
{
	XMemoryBlock *		m_pFreeLink;			// 自由块链
	uint32				m_nFreeCount;			// 自由块数
	XCritical           m_lock;                 // 锁
};

struct XMemoryBlock
{
	XMemoryBlock *			m_pNext;			// 块链
	XBlockLinkHead *		m_pHead;			// 链头
	#ifdef __DEBUG__
	uint32					m_mark;				// 检验字
	#endif//__DEBUG__
	char					m_data[0];			// 有效数据
};
#pragma pack()

////////////////////////////////////////////////////////////////////////////////
// XMemory
////////////////////////////////////////////////////////////////////////////////
class XMemory
{
public:
	XMemory()
	{
		for (size_t i = 0; i < COUNT_OF_ARRAY(m_MainLink); i++)
		{
			m_MainLink[i].m_pFreeLink = NULL;
			m_MainLink[i].m_nFreeCount = 0;
		}
	}

	~XMemory()
	{
		XMemoryBlock *pBlock = NULL;
		XMemoryBlock *pBlock2 = NULL;

		for (size_t i = 0; i < COUNT_OF_ARRAY(g_uRose); i++)
		{
			XLockGuard<XCritical> lock_(m_MainLink[i].m_lock);
			pBlock = m_MainLink[i].m_pFreeLink;
			while (pBlock)
			{
				pBlock2 = pBlock;
				pBlock = pBlock->m_pNext;
				delete[] pBlock2;
			}
		}
	}

	void* alloc(uint32 uSize)
	{
		XBlockLinkHead* pHead = NULL;
		XMemoryBlock *pBlock = NULL;
		uSize += sizeof(XMemoryBlock);

		uint32 count = COUNT_OF_ARRAY(g_uRose);
		if (uSize > g_uRose[count - 1])
		{
			// 超大块由操作系统分配
			pBlock = (XMemoryBlock *)::new(nothrow) uint8[uSize];
		}
		else
		{
			// 查找合适的块链
			uint32 index = 0;
			for (index = 0; index < count; index++)
			{
				if (uSize <= g_uRose[index]) break;
			}
			pHead = &m_MainLink[index];

			XLockGuard<XCritical> lock(pHead->m_lock);
			if (pHead->m_pFreeLink == NULL)
			{
				pBlock = (XMemoryBlock *)new(nothrow) uint8[g_uRose[index]];
			}
			else
			{
				pBlock = pHead->m_pFreeLink;
				pHead->m_pFreeLink = pBlock->m_pNext;
				if (pHead->m_nFreeCount > 0) pHead->m_nFreeCount--;
			}
		}

		ASSERT(pBlock);
		ASSERT((pBlock->m_mark = MEMORY_BLOCK_MARK, true));
		pBlock->m_pHead = pHead;
		pBlock->m_pNext = NULL;

		return (void *)pBlock->m_data;
	}

	void free(void *pData)
	{
		if (pData == NULL) return;
		ASSERT(((uint32)((char *)pData - (char *)NULL) > (uint32)sizeof(XMemoryBlock)) && "Invalid Memory Block!");
		XMemoryBlock* pBlock = (XMemoryBlock *)((char *)pData - sizeof(XMemoryBlock));
		ASSERT((pBlock->m_mark == MEMORY_BLOCK_MARK) && "Corrupt Memory Block!");

		XBlockLinkHead* pHead = pBlock->m_pHead;
		if ((pHead == NULL) || (pHead->m_nFreeCount > 10000))
		{
			// 1. 操作系统管理的块
			// 2. 自由链上的自由块过多
			delete[] pBlock;
		}
		else
		{
			XLockGuard<XCritical> lock_(pHead->m_lock);
			pBlock->m_pNext = (XMemoryBlock *)pHead->m_pFreeLink;
			pHead->m_pFreeLink = pBlock;
			pHead->m_nFreeCount++;
		}
		return;
	}

private:
	XBlockLinkHead m_MainLink[COUNT_OF_ARRAY(g_uRose)];
};

static XMemory g_Memory_instance;

////////////////////////////////////////////////////////////////////////////////
// class XBuffer
////////////////////////////////////////////////////////////////////////////////
class XBuffer : public IXBuffer
{
public:
	struct XBuffer_Deleter
	{
		void operator()(XBuffer* pBuffer) const
		{
			if (pBuffer == NULL) return;
			pBuffer->~XBuffer();
			g_Memory_instance.free((void *)pBuffer);
		}
	};

public:
	XBuffer(uint32 uSize, void *pvData)
	  : m_orignSize(uSize)
	  , m_orignData((char*)pvData)
	  , m_size(uSize)
	  , m_pData((char*)pvData)
	{
		ASSERT(pvData);
	}

	virtual uint32 size() const
	{
		return m_size;
	}

	virtual void* data() const
	{
		return m_pData;
	}

	virtual void resize(uint32 newSize)
	{
		ASSERT(newSize <= capacity());
		m_size = newSize;
	}

	virtual uint32 size_pre_reserve() const
	{
		return m_pData - (m_orignData - MEMORY_BUFFER_RESERVE_SIZE);
	}

	virtual void resize_pre_reserve(uint32 newSize)
	{
		uint32 reserveSize = size_pre_reserve();
		ASSERT(newSize <= reserveSize + m_size);
		m_pData = (char*)m_orignData - MEMORY_BUFFER_RESERVE_SIZE + newSize;
		m_size = reserveSize + m_size - newSize;
	}

	virtual uint32 capacity() const
	{
		return (m_orignData + m_orignSize) - m_pData;
	}

	virtual void restore()
	{
		m_size = m_orignSize;
		m_pData = (char *)m_orignData;
	}

	virtual shared_ptr<IXBuffer> clone()
	{
		shared_ptr<XBuffer> ptrBuffer = dynamic_pointer_cast<XBuffer, IXBuffer>(create_buffer(m_orignSize));
		ASSERT(ptrBuffer.get() != NULL);
		ptrBuffer->resize_pre_reserve(this->size_pre_reserve());
		ptrBuffer->resize(this->size());
		memcpy((char *)ptrBuffer->m_orignData - MEMORY_BUFFER_RESERVE_SIZE,
			   m_orignData - MEMORY_BUFFER_RESERVE_SIZE,
			   m_orignSize + MEMORY_BUFFER_RESERVE_SIZE);
		return shared_ptr<IXBuffer>(ptrBuffer);
	}

private:
	const uint32	m_orignSize;
	const char*     m_orignData;
	uint32			m_size;
	char*           m_pData;
};

///////////////////////////////////////////////////////////////////////
// function create_buffer
///////////////////////////////////////////////////////////////////////
shared_ptr<IXBuffer> create_buffer(uint32 uSize)
{
	void* pvBuffer = g_Memory_instance.alloc(uSize + sizeof(XBuffer) + MEMORY_BUFFER_RESERVE_SIZE);
	ASSERT(pvBuffer);

	char* pvData = (char *)pvBuffer + sizeof(XBuffer) + MEMORY_BUFFER_RESERVE_SIZE;
	XBuffer* pBuffer = new(pvBuffer)XBuffer(uSize, pvData);  // placement operator new
	ASSERT(pBuffer);

	return shared_ptr<IXBuffer>(pBuffer, XBuffer::XBuffer_Deleter());
}

void* memory_alloc(uint32 uSize)
{
	return g_Memory_instance.alloc(uSize);
}

void memory_free(void* pData)
{
	return g_Memory_instance.free(pData);
}

} // namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"
#include "xcore_log.h"
#include "xcore_thread.h"

namespace xcore
{

bool xcore_test_memory()
{
	shared_ptr<IXBuffer> ptr = xcore::create_buffer(12);
	ptr = xcore::create_buffer(24);
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST

