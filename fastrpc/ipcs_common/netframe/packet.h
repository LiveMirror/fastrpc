// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: netframe packet

#ifndef IPCS_COMMON_NETFRAME_PACKET_H
#define IPCS_COMMON_NETFRAME_PACKET_H

#include <stdlib.h>
#include <string.h>
#include "mempool.h"
#include "socket.h"

namespace netframe {

using namespace ipcs_common;

class Packet
{
public:
    Packet():
        m_OwnBuffer(false),
        m_Content(NULL),
        m_Length(0)
    {
    }

    virtual ~Packet()
    {
        DeleteContent();
    }

    /// @brief 设置packet数据，packet分配内存，拷贝数据内容
    void  SetContent(const void* content, size_t length)
    {
        DeleteContent();
        m_Content = MemPool_Allocate(length);
        ::memcpy(m_Content, content, length);
        m_Length = length;
        m_OwnBuffer = true;
    }

    /// @brief 设置packet内容指针，packet本身不持有内存
    void  SetContentPtr(const void* content, size_t length)
    {
        DeleteContent();
        m_Content = const_cast<void*>(content);
        m_Length = length;
        m_OwnBuffer = false;
    }

    void  DeleteContent()
    {
        if (m_Content != NULL)
        {
            if (m_OwnBuffer)
                MemPool_Free(m_Content);
            m_Content = NULL;
        }
        m_OwnBuffer = false;
        m_Length = 0;
    }

    unsigned char* Content()
    {
        if (m_Content != NULL)
        {
            return (unsigned char*)(m_Content);
        }
        return NULL;
    }

    const unsigned char* Content() const
    {
        if (m_Content != NULL)
        {
            return (const unsigned char*)(m_Content);
        }
        return NULL;
    }

    size_t Length() const
    {
        return m_Length;
    }

    void SetLocalAddress(const SocketAddress& address)
    {
        m_LocalAddress = address;
    }

    void GetLocalAddress(SocketAddress& address) const
    {
        address = m_LocalAddress;
    }

    const SocketAddressStorage& GetLocalAddress() const
    {
        return m_LocalAddress;
    }

    void SetRemoteAddress(const SocketAddress& address)
    {
        m_RemoteAddress = address;
    }

    void GetRemoteAddress(SocketAddress& address) const
    {
        address = m_RemoteAddress;
    }

    const SocketAddressStorage& GetRemoteAddress() const
    {
        return m_RemoteAddress;
    }

private:
    Packet(const Packet&);
    Packet& operator=(const Packet&);
private:
    bool m_OwnBuffer; ///< 是否持有缓冲区
    void* m_Content;  ///< 保存包的内容的MemUnit
    size_t m_Length;  ///< 数据包的总长度
    SocketAddressStorage m_LocalAddress;
    SocketAddressStorage m_RemoteAddress;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_PACKET_H

