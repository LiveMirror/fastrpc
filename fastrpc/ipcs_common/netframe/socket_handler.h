// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: socket handler

#ifndef IPCS_COMMON_NETFRAME_SOCKET_HANDLER_H
#define IPCS_COMMON_NETFRAME_SOCKET_HANDLER_H

#include <string>
#include <vector>
#include "netframe.h"
#include "packet.h"

namespace netframe {

/// 事件 Handler 基类
class SocketHandler
{
protected:
    explicit SocketHandler(NetFrame& netframe): m_NetFrame(netframe),
        m_CommandQueueLength(0)
    {
    }
    NetFrame::EndPoint m_EndPoint;

public:
    virtual ~SocketHandler() {}

    void SetEndPointId(SocketId id)
    {
        m_EndPoint.SetEndPointId(id);
    }
    virtual NetFrame::EndPoint& GetEndPoint()
    {
        return m_EndPoint;
    }
    NetFrame& GetNetFrame() const
    {
        return m_NetFrame;
    }

    size_t GetCommandQueueLength()
    {
        return m_CommandQueueLength;
    }
    void SetCommandQueueLength(size_t length)
    {
        m_CommandQueueLength = length;
    }

    /// 连接关闭前执行的函数
    /// @param error_code 失败原因，系统错误码
    /// error_code 的可能值:
    /// EMSGSIZE 消息太长，超出了max_packet_size
    /// 其余为其他系统错误码
    virtual void OnClose(int error_code) = 0;

    /// 发送失败时的消息通知
    /// @param packet 失败的包
    /// @param error_code 失败原因，系统错误码
    /// @return 是否删除包
    /// @retval true 包需要删除
    /// @retval false 包已被用户管理，netframe不要删除。
    /// error_code 的可能值
    /// EMSGSIZE 消息太长
    /// ENOBUFS 到达本连接队列长度上限
    /// ENOMEN 积累的待发消息总长以超出限制
    /// ESHUTDOWN 用户已经主动 Close
    /// 其余为其他系统错误码
    virtual bool OnSendingFailed(Packet* packet, int error_code)
    {
        return true;
    }

private:
    NetFrame& m_NetFrame;
    size_t m_CommandQueueLength;
};

class StreamSocketHandler : public SocketHandler
{
public:
    explicit StreamSocketHandler(NetFrame& netframe):
        SocketHandler(netframe)
    {
    }

    /// 连接成功时执行的函数，主动发起连接和被动连接时都会调用
    virtual void OnConnected() = 0;

    /// 根据已收到的数据探测包的大小，返回0表示未知，大于0返回识别出的包的大小
    /// 小于0表示探测发生错误（如协议错误）
    /// 当包长超过最大限定长度或探测过程发生错误时，连接将被关闭
    virtual int DetectPacketSize(const void* data, size_t length) = 0;

    NetFrame::StreamEndPoint& GetEndPoint() /// override
    {
        return static_cast<NetFrame::StreamEndPoint&>(m_EndPoint);
    }

    /// 接受到数据包之后执行的函数;
    virtual void OnReceived(const Packet& packet) = 0;

    /// 数据包发送完毕之后执行的函数;
    /// @param packet 已被发送的包
    /// @return 是否让netframe删除包
    /// @retval true 包需要删除
    /// @retval false 包已被用户管理，netframe不要删除。
    virtual bool OnSent(Packet* packet)
    {
        return true;
    }

    /// Set/Get remote network address
    void SetRemoteAddress(const SocketAddress& address) {
        m_remote_address = address;
    }

    const SocketAddress& GetRemoteAddress() const {
        return m_remote_address;
    }

private:
    SocketAddressStorage m_remote_address;
};

class DatagramSocketHandler : public SocketHandler
{
public:
    explicit DatagramSocketHandler(NetFrame& netframe):
        SocketHandler(netframe)
    {
    }

    virtual NetFrame::DatagramEndPoint& GetEndPoint() /// override
    {
        return static_cast<NetFrame::DatagramEndPoint&>(m_EndPoint);
    }

    /// 接受到数据包之后执行的函数;
    virtual void OnReceived(const Packet& packet) = 0;

    /// 数据包发送完毕之后执行的函数;
    virtual bool OnSent(Packet* packet)
    {
        return true;
    }
};

class ListenSocketHandler : public SocketHandler
{
public:
    explicit ListenSocketHandler(NetFrame& netframe):
        SocketHandler(netframe)
    {
    }

    /// 对于Listen端口;接受连接成功时执行的函数.
    /// 该函数需要在函数体内进行新连接上处理器的新建
    virtual StreamSocketHandler* OnAccepted(SocketId id) = 0;

    NetFrame::ListenEndPoint& GetEndPoint() /// override
    {
        return static_cast<NetFrame::ListenEndPoint&>(m_EndPoint);
    }
};

/// 每个文本行识别为一条消息
class LineStreamSocketHandler : public StreamSocketHandler
{
public:
    explicit LineStreamSocketHandler(NetFrame& netframe): StreamSocketHandler(netframe)
    {
    }
    int DetectPacketSize(const void* header, size_t size)
    {
        const char* p = (const char*) memchr(header, '\n', size);
        if (p)
            return p - (const char*)header + 1;
        else
            return 0;
    }
};

} // namespace netframe

#endif // COMMON_NETFRAME_SOCKET_HANDLER_H
