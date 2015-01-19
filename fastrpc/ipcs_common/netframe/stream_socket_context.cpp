// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: stream socket context

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#endif

#include "stream_socket_context.h"

#include "command_event.h"
#include "event_poller.h"
#include "netframe.h"
#include "this_thread.h"

namespace netframe {

#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

const size_t DefaultBufferSize = 0x4000;

StreamSocketContext::StreamSocketContext(
    NetFrame* netframe,
    const SocketAddress* local_address,
    const SocketAddress* remote_address,
    SocketId sock_id,
    StreamSocketHandler* handler,
    size_t max_packet_size,
    const NetFrame::EndPointOptions& options,
    bool connected
) : SocketContext(netframe, local_address, remote_address, sock_id,
    max_packet_size, options),
    m_Connected(connected),
    m_SentLength(0),
    m_MinBufferLength(std::min(max_packet_size, DefaultBufferSize)),
    m_ReceiveBuffer(NULL),
    m_ReceivedLength(0),
    m_DetectedPacketLength(0)
{
    SetEventHandler(handler);
    m_ReceiveBufferLength = m_MinBufferLength;
    m_ReceiveBuffer = static_cast<char*>(MemPool_Allocate(m_ReceiveBufferLength));
    IncreaseReceiveBufferedLength(m_ReceiveBufferLength);
    m_ReceivePacket.SetLocalAddress(m_LocalAddress);
    m_ReceivePacket.SetRemoteAddress(m_RemoteAddress);
}

StreamSocketContext::~StreamSocketContext()
{
    DecreaseReceiveBufferedLength(m_ReceiveBufferLength);
    MemPool_Free(m_ReceiveBuffer);
}

unsigned int StreamSocketContext::GetWantedEventMask() const
{
    unsigned int result = IoEvent_Readable;
    if (m_Connected)
    {
        if (!m_CommandQueue.IsEmpty())
        {
            result |= IoEvent_Writeable;
        }
    }
    else
    {
        result |= IoEvent_Writeable | IoEvent_Connected;
    }
    return result;
}

void StreamSocketContext::ResizeReceiveBuffer(size_t new_size)
{
    char* new_buffer = (char*) MemPool_Allocate(new_size);
    memcpy(new_buffer, m_ReceiveBuffer, m_ReceivedLength);
    MemPool_Free(m_ReceiveBuffer);
    m_ReceiveBuffer = new_buffer;
    if (m_ReceiveBufferLength > new_size) {
        DecreaseReceiveBufferedLength(m_ReceiveBufferLength - new_size);
    } else {
        IncreaseReceiveBufferedLength(new_size - m_ReceiveBufferLength);
    }
    m_ReceiveBufferLength = new_size;
}

void StreamSocketContext::AdjustReceiveBufferSize()
{
    // 计算新的合适的缓冲区大小
    size_t new_size = m_ReceiveBufferLength;
    if (m_DetectedPacketLength > 0)
    {
        // 完整长度已知，调整到此长度
        new_size = m_DetectedPacketLength;
    }
    else
    {
        // 接收缓冲区以填满一半以上
        if (m_ReceivedLength > m_ReceiveBufferLength / 2)
        {
            new_size = 2 * m_ReceiveBufferLength;
        }
    }

    if (new_size > m_MaxPacketLength)
        new_size = m_MaxPacketLength;

    // 不能调整的太小以免波动太大影响性能
    if (new_size < m_MinBufferLength)
        new_size = m_MinBufferLength;

    if (new_size > m_ReceiveBufferLength)
    {
        // 如果是增大，总是允许
        ResizeReceiveBuffer(new_size);
    }
    else
    {
        // 如果是缩小，只有差别较大时才调整，以免频繁调整波动太大
        if (m_ReceiveBufferLength / 2 > new_size)
        {
            ResizeReceiveBuffer(new_size);
        }
    }
}

bool StreamSocketContext::HandleInput()
{
    size_t total_num_packets = 0;
    size_t total_received_length = 0;

    for (;;)
    {
        AdjustReceiveBufferSize();

        char* buffer = m_ReceiveBuffer + m_ReceivedLength;
        size_t buffer_length = m_ReceiveBufferLength - m_ReceivedLength;

        int received_length = Receive(buffer, buffer_length);
        if (received_length < 0) {
            return false;
        }

        if (received_length > 0)
        {
            total_received_length += received_length;
            m_ReceivedLength += received_length;

            int num_packets = SplitAndIndicatePackets();
            if (num_packets < 0) {
                return false;
            }

            if (num_packets == 0 && m_ReceivedLength == m_MaxPacketLength) {
                return false;
            }

            total_num_packets += num_packets;
        }
        // 缓冲区未收满，不必再尝试，退出。
        if ((size_t)received_length < buffer_length)
            break;

        // 每次处理较多包或较长数据之后该 Socket 上的读事件需要进行跳出,
        // 以免在 一个Socket 上面受到 Rush 攻击而不能执行其它操作
        if (total_num_packets > 1000 || total_received_length > 0x10000)
        {
            break;
        }
    }

    return true;
}

int StreamSocketContext::CheckAndIndicatePacket(char*& buffer, int& left_length)
{
    if (m_DetectedPacketLength <= 0) {
        m_DetectedPacketLength = GetEventHandler()->DetectPacketSize(buffer, left_length);
        if (m_DetectedPacketLength == 0) {
            return 0;
        }

        if (m_DetectedPacketLength > static_cast<int>(m_MaxPacketLength))
        {
            // 数据包大小超出限制
            m_LastError = EMSGSIZE;
            return -1;
        }

        if (m_DetectedPacketLength < 0) {
            // 检测到错误的数据包
            m_LastError = EBADMSG;
            return -1;
        }
    }

    if (m_DetectedPacketLength <= left_length)
    {
        // 接收到完整的数据包
        m_ReceivePacket.SetContentPtr(buffer, m_DetectedPacketLength);
        GetEventHandler()->OnReceived(m_ReceivePacket);

        // 移到下一个包头
        buffer += m_DetectedPacketLength;
        left_length -= m_DetectedPacketLength;
        m_DetectedPacketLength = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

int StreamSocketContext::SplitAndIndicatePackets()
{
    char* begin = m_ReceiveBuffer;
    int left_length = static_cast<int>(m_ReceivedLength);

    int num_packets = 0;
    while (left_length > 0) {
        int result = CheckAndIndicatePacket(begin, left_length);
        if (result < 0)
            return -1;

        if (result == 0) {
            break;
        }

        ++num_packets;
    }

    // 剩余部分拷贝到开头
    if (m_ReceiveBuffer != begin && left_length > 0)
        ::memmove(m_ReceiveBuffer, begin, left_length);
    m_ReceivedLength = left_length;

    return num_packets;
}

// 接受当前Socket上到达的所有数据
int StreamSocketContext::Receive(void* buffer, size_t buffer_length)
{
    assert(buffer != NULL);
    assert(buffer_length > 0);

    int received_length = -1;
    int error_code = 0;

    do
    {
        received_length = (int) ::recv(GetFd(), (char*) buffer, buffer_length, 0);
        error_code = ThisThread::GetLastErrorCode();
    } while ((received_length == -1) && (error_code == EINTR));

    if (received_length == -1)
    {
        if ((error_code == EAGAIN) || (error_code == EWOULDBLOCK))
        {
            received_length = 0;
        }
        else
        {
            m_LastError = error_code;
            // XXX: need more handle
            return -1;
        }
    }
    else if (received_length == 0)
    {
        received_length = -1;
    }

    return  received_length;
}

// 在该Socket上处理写事件
bool StreamSocketContext::HandleOutput()
{
    for (;;)
    {
        CommandEvent event;
        if (m_CommandQueue.GetFront(&event))
        {
            if (event.Type == CommandEvent_CloseSocket)
            {
                return false;  // 已关闭，删除socket context
            }
            // 非关闭事件即发送数据包事件
            Packet* packet = event.Cookie.CastTo<Packet*>();
            assert(packet);
            if (packet->Length() <= m_MaxPacketLength)
            {
                int sent_result = SendPacket(packet);
                if (sent_result > 0)
                {
                    m_CommandQueue.PopFront();
                    DecreaseSendBufferedLength(packet->Length());
                    DecreaseSendBufferedPacket();

                    GetEventHandler()->SetCommandQueueLength(m_CommandQueue.Size());
                    if (GetEventHandler()->OnSent(packet))
                        delete packet;

                    m_SentLength = 0;
                }
                else
                {
                    return sent_result == 0;
                }
            }
            else // packet too long
            {
                m_CommandQueue.PopFront();
                HandleSendingFailed(packet, EMSGSIZE);
            }
        }
        else
        {
            break;
        }
    }

    return true;
}

int StreamSocketContext::SendPacket(Packet* packet)
{
    for (;;)
    {
        unsigned char* sending_buffer = packet->Content() + m_SentLength;
        size_t sending_length = packet->Length() - m_SentLength;

        assert(sending_length > 0);
        int sent_length = SendBuffer(sending_buffer, sending_length);
        if (sent_length > 0)
        {
            m_SentLength +=  sent_length;
            if (m_SentLength == packet->Length())
                return 1;
        }
        else
        {
            return sent_length;
        }
    }
}

int StreamSocketContext::SendBuffer(const void* buffer, size_t size)
{
    assert(size > 0);

    int flags = 0;

    flags |= MSG_NOSIGNAL;

#ifdef MSG_MORE
    // 因为开启了 tcp nodelay, 默认会立即发生。
    // 如果还有其他包，设置上 MSG_MORE 以便一起发送。
    if (m_CommandQueue.HasMore())
    {
        flags |= MSG_MORE;
    }
#endif

    for (;;)
    {
        int sent_length = (int) ::send(GetFd(), (char*) buffer, size, flags);
        if (sent_length > 0)
            return sent_length;

        int error_code = ThisThread::GetLastErrorCode();

        if (error_code == EAGAIN)
        {
            return 0;
        }
        else if (error_code != EINTR)
        {
            m_LastError = error_code;
            return -1;
        }
    }
}

int StreamSocketContext::HandleCommandEvent(const CommandEvent& event)
{
    switch (event.Type)
    {
    case CommandEvent_AddSocket:
        return 1;
    case CommandEvent_CloseSocket_Now:
        SocketContext::HandleClose();
        return 1;
    case CommandEvent_CloseSocket:
    {
        m_CommandQueue.Enqueue(event, true); // 关闭事件强制入队
        if (!HandleOutput())
            return -1;
    }
    break;
    case CommandEvent_SendPacket:
    {
        if (!m_CommandQueue.Enqueue(event)) // 队列满, 发送失败
        {
            Packet* packet = event.Cookie.CastTo<Packet*>();
            HandleSendingFailed(packet, ENOBUFS);
        }
        if (!HandleOutput())
            return -1;
    }
    break;
    case CommandEvent_SendUrgentPacket:
    {
        m_CommandQueue.EnqueueUrgent(event);
        if (!HandleOutput())
            return -1;
    }
    break;
    default:
        assert(!"unexpacted event");
        return 0;
    }
    return 1;
}

int StreamSocketContext::HandleIoEvent(const IoEvent& event)
{
    if (event.Mask & IoEvent_Closed)
    {
        if (event.ErrorCode != 0)
            m_LastError = event.ErrorCode;
        else
            HandleInput();
        return -1;
    }

    if (m_Connected)
    {
        if (event.Mask & IoEvent_Writeable)
        {
            if (!HandleOutput())
                return -1;
        }

        if (event.Mask & IoEvent_Readable)
        {
            if (!HandleInput())
                return -1;
        }
    }
    else
    {
        if (event.Mask & (IoEvent_Connected | IoEvent_Readable | IoEvent_Writeable))
        {
            m_Connected = true;
            GetEventHandler()->OnConnected();
        }
    }

    return 1;
}

} // namespace netframe
