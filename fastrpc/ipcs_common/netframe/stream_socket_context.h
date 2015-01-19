// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: stream socket context

#ifndef IPCS_COMMON_NETFRAME_STREAM_SOCKET_CONTEXT_H
#define IPCS_COMMON_NETFRAME_STREAM_SOCKET_CONTEXT_H

#include "socket_context.h"

namespace netframe {

class StreamSocketContext : public SocketContext
{
public:
    StreamSocketContext(
        NetFrame* netframe,
        const SocketAddress* local_address,
        const SocketAddress* remote_address,
        SocketId sock_id,
        StreamSocketHandler* handler,
        size_t max_packet_size,
        const NetFrame::EndPointOptions& options,
        bool connected
    );
    ~StreamSocketContext();

private:
    virtual int HandleIoEvent(const IoEvent& event); // override
    virtual int HandleCommandEvent(const CommandEvent& event); // override
    virtual unsigned int GetWantedEventMask() const; // override

    /// 在该Socket上处理读事件
    /// @retval true 成功
    /// @retval false 出现错误
    bool HandleInput();

    /// 在该Socket上处理写事件
    /// @retval true 成功
    /// @retval false 出现错误
    bool HandleOutput();

    /// 尝试发送一个包，直到阻塞或者错误
    /// @param packet 要发送的包
    /// @retval >0 已全部发送
    /// @retval =0 阻塞
    /// @retval <0 出现错误
    int SendPacket(Packet* packet);

    /// 尝试发送缓冲区，直到阻塞或者错误
    /// @retval >=0 发出去的长度
    /// @retval =0 阻塞
    /// @retval <0 出现错误
    int SendBuffer(const void* buffer, size_t size);

    /// 读取Socket上已经到达的数据
    int Receive(void* buffer, size_t buffer_length);

    /// 从已接受的缓冲区中探测包边界，识别并通知给上层
    /// @return 识别到的包的个数
    /// @retval <0 出现错误
    int SplitAndIndicatePackets();

    int CheckAndIndicatePacket(char*& buffer, int& left_length);

    void AdjustReceiveBufferSize();
    void ResizeReceiveBuffer(size_t size);
private:
    virtual StreamSocketHandler* GetEventHandler() const // override
    {
        return static_cast<StreamSocketHandler*>(SocketContext::GetEventHandler());
    }
private:
    bool   m_Connected;            ///< 是否已连接
    size_t m_SentLength;           ///< 已经发送出去了的包的长度
    size_t m_MinBufferLength;      ///< 最小缓冲区长度
    char*  m_ReceiveBuffer;        ///< 接收到的数据包
    size_t m_ReceiveBufferLength;  ///< 当前接收缓冲区的长度
    size_t m_ReceivedLength;       ///< 已经接受到了的数据长度
    int    m_DetectedPacketLength; ///< 经检测已知的包长度
    Packet m_ReceivePacket;        ///< 当前所接收到的数据包
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_STREAM_SOCKET_CONTEXT_H
