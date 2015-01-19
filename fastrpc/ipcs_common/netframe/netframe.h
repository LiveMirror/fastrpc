// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: netframe

#ifndef IPCS_COMMON_NETFRAME_NETFRAME_H
#define IPCS_COMMON_NETFRAME_NETFRAME_H

#include <string>
#include <vector>
#include "annotation.h"
#include "command_event.h"
#include "packet.h"
#include "mutex.h"
#include "socket.h"

namespace netframe {

class WorkThread;
class SocketContext;
class ListenSocketContext;

class SocketHandler;
class ListenSocketHandler;
class DatagramSocketHandler;
class StreamSocketHandler;

/// 异步网络通讯库.
/// 支持通过调用 SendPacket 等主动发送，通过 handler 被动接收。
class NetFrame
{
    friend class SocketContext;
    friend class WorkThread;
    friend class ListenSocketContext;

public:
    static const size_t DEFAULT_MAX_COMMAND_QUEUE_LENGTH = 100000;
    static const size_t MAX_BUFFERED_SIZE = 1 * 1024 * 1024 * 1024L;

    enum ErrorCode
    {
        ErrorCode_None = 0,
        ErrorCode_QueueFull,
        ErrorCode_SocketClosed,
        ErrorCode_SystemError
    };

    class EndPoint
    {
    public:
        friend class NetFrame;
        friend class SocketHandler;
        EndPoint() : m_SockId(-1)
        {
        }
        SocketId GetSockId() const
        {
            return m_SockId;
        }
        int64_t GetId() const
        {
            return m_SockId.Id;
        }
        int GetFd() const
        {
            return m_SockId.SockFd;
        }
        bool IsValid() const
        {
            return GetFd() >= 0;
        }
        bool operator == (const EndPoint& rhs)
        {
            return m_SockId == rhs.m_SockId;
        }
        bool operator != (const EndPoint& rhs)
        {
            return !(m_SockId == rhs.m_SockId);
        }

        explicit EndPoint(SocketId id): m_SockId(id) {}
        explicit EndPoint(int64_t id)
        {
            m_SockId.Id = id;
        }

    private:
        void SetEndPointId(SocketId id)
        {
            m_SockId = id;
        }
        SocketId m_SockId;
    };

    class ListenEndPoint : public EndPoint
    {
        friend class ListenSocketHandler;
    public:
        ListenEndPoint() {}
    private:
        ListenEndPoint(SocketId id) : EndPoint(id) {}
    };

    class StreamEndPoint : public EndPoint
    {
        friend class StreamSocketHandler;
    public:
        StreamEndPoint() {}
    private:
        StreamEndPoint(SocketId id) : EndPoint(id) {}
    };

    class DatagramEndPoint : public EndPoint
    {
        friend class DatagramSocketHandler;
    public:
        DatagramEndPoint() {}
    private:
        DatagramEndPoint(SocketId id) : EndPoint(id) {}
    };

    class EndPointOptions
    {
    public:
        EndPointOptions() :
            m_MaxCommandQueueLength(DEFAULT_MAX_COMMAND_QUEUE_LENGTH),
            m_SendBufferSize(32 * 1024),
            m_ReceiveBufferSize(32 * 1024),
            m_Priority(0)
        {
        }

        size_t MaxCommandQueueLength() const
        {
            return m_MaxCommandQueueLength;
        }
        EndPointOptions& MaxCommandQueueLength(size_t value)
        {
            m_MaxCommandQueueLength = value;
            return *this;
        }
        size_t SendBufferSize() const
        {
            return m_SendBufferSize;
        }
        EndPointOptions& SendBufferSize(size_t value)
        {
            m_SendBufferSize = value;
            return *this;
        }
        size_t ReceiveBufferSize() const
        {
            return m_ReceiveBufferSize;
        }
        EndPointOptions& ReceiveBufferSize(size_t value)
        {
            m_ReceiveBufferSize = value;
            return *this;
        }

        // QoS priority, should be 0~6
        int Priority() const
        {
            return m_Priority;
        }

        EndPointOptions& Priority(int value)
        {
            m_Priority = value;
            return *this;
        }
    private:
        size_t m_MaxCommandQueueLength;
        size_t m_SendBufferSize;
        size_t m_ReceiveBufferSize;
        int m_Priority;
    };

    /// @param num_workthreads 工作线程数目, 0 代表系统逻辑 CPU 数目
    /// @param max_bufferd_size 支持的最大待发送的包的总字节大小
    NetFrame(unsigned int num_workthreads = 0, size_t max_bufferd_size = MAX_BUFFERED_SIZE);
    ~NetFrame();

    /// @retval >0 socket id
    /// @retval <0 -errno
    int64_t AsyncListen(
        const SocketAddress& address,   ///< 要监听的本地地址
        ListenSocketHandler* handler,   ///< Socket上的处理器
        size_t max_packet_size,         ///< Socket上所传输的最大数据包的大小
        const EndPointOptions& options = EndPointOptions()
    );

    /// @retval >0 socket id
    /// @retval <0 -errno
    int64_t AsyncConnect(
        const SocketAddress& local_address,     ///< 本地地址
        const SocketAddress& remote_address,    ///< 远端地址
        StreamSocketHandler* handler,           ///< Socket上的处理器
        size_t max_packet_size,                 ///< Socket上所传输的最大数据包的大小
        const EndPointOptions& options = EndPointOptions()
    );

    /// @retval >0 socket id
    /// @retval <0 -errno
    int64_t AsyncConnect(
        const SocketAddress& remote_address,    ///< 远端地址
        StreamSocketHandler* handler,           ///< Socket上的处理器
        size_t max_packet_size,                 ///< Socket上所传输的最大数据包的大小
        const EndPointOptions& options = EndPointOptions()
    );

    /// @retval >0 socket id
    /// @retval <0 -errno
    int64_t AsyncDatagramBind(
        const SocketAddress& address,   ///< 要绑定的本地地址
        DatagramSocketHandler* handler, ///< Socket上的处理器
        size_t max_packet_size,         ///< Socket上所传输的最大数据包的大小
        const EndPointOptions& options = EndPointOptions()
    );

    /// @brief 关闭一个Socket
    /// @param endpoint  Socket 的端点号
    /// @param immediate 默认为立即关闭，若为否，则等待之前的数据包发完后关闭
    /// @retval 0 成功，其他值失败
    int CloseEndPoint(EndPoint& endpoint, bool immediate = true);

    /// @brief 发送一个流式连接上的包
    /// @param endpoint  Socket 的端点号
    /// @param packet   指向被发送包的指针，包体内存由netframe负责销毁
    /// @param urgent   是否是紧急的数据包
    /// @retval 0 成功，其他值失败
    WARN_UNUSED_RESULT int SendPacket(const StreamEndPoint& endpoint,
                                      Packet* packet,
                                      bool urgent = false);

    /// @brief 发送一个流式连接上的包
    /// @param endpoint  Socket 的端点号
    /// @param data      要发送的数据的缓冲区，缓冲区的内容将被拷贝进队列后发送
    /// @param size      要发送的数据的长度
    /// @param urgent    是否是紧急的数据包
    /// @retval 0 成功，其他值失败
    int SendPacket(const StreamEndPoint& endpoint, const void* data, size_t size,
                   bool urgent = false);

    /// @brief 发送UDP数据包
    /// @param endpoint  Socket 的端点号
    /// @param address 远端的地址
    /// @param packet 发送包的指针，包体内存由netframe负责销毁
    /// @retval 0 成功
    WARN_UNUSED_RESULT int SendPacket(const DatagramEndPoint& endpoint,
                                      const SocketAddress& address,
                                      Packet* packet);

    /// @brief 发送UDP数据包
    /// @param endpoint Socket 的端点号
    /// @param address  远端的地址
    /// @param data     要发送的数据的缓冲区，缓冲区的内容将被拷贝进队列后发送
    /// @param size     要发送的数据的长度
    /// @retval 0 成功
    int SendPacket(const DatagramEndPoint& endpoint, const SocketAddress& address,
                   const void* data, size_t size);

    /// @brief 等待所有的数据包发送完毕
    /// @param timeout 指定时间，单位：毫秒
    /// @retval false 等待指定时间内尚未发完
    /// @retval true 已发完，立即返回
    bool WaitForSendingComplete(int timeout = 10000);

    /// @brief 得到当前发送缓冲区的大小
    /// @retval size 当前发送缓冲区的长度
    size_t GetCurrentSendBufferedLength() const;

    /// @brief 得到当前接收缓冲区的大小
    /// @retval size 当前接收缓冲区的长度
    size_t GetCurrentReceiveBufferedLength() const;

private:
    ///< @param local_address 本地地址，NULL 表示不指定
    ///< @param remote_address 远端地址
    ///< @param handler Socket上的处理器
    ///< @param max_packet_size Socket上所传输的最大数据包的
    ///< 大小，如果超过这个上限，
    ///< OnClose 的 error_code 为 EMSGSSIZE
    int64_t AsyncConnect(
        const SocketAddress* local_address,
        const SocketAddress& remote_address,
        StreamSocketHandler* handler,
        size_t max_packet_size,
        const EndPointOptions& options = EndPointOptions()
    );
    WorkThread* GetWorkThreadOfFd(int fd);

    /// add a new command to system
    bool AddCommandEvent(const CommandEvent& event);

    static int CloseFd(int fd);

    void IncreaseSendBufferedLength(size_t length);
    void DecreaseSendBufferedLength(size_t length);
    size_t GetMaxSendBufferedLength() const;

    void IncreaseSendBufferedPacket();
    void DecreaseSendBufferedPacket();
    size_t GetSendBufferedPacketNumber();

    void IncreaseReceiveBufferedLength(size_t length);
    void DecreaseReceiveBufferedLength(size_t length);

    /// 由sock fd得到新的SocketId
    static SocketId GenerateSocketId(int32_t sock_fd);

private:
    static uint32_t m_SequenceId;
    size_t m_MaxSendBufferedMemorySize;
    size_t m_CurrentSendBufferedMemorySize;
    size_t m_SendBufferedPacketNumber;
    size_t m_CurrentReceiveBufferedMemorySize;
    std::vector<WorkThread*> m_WorkThreads;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_NETFRAME_H
