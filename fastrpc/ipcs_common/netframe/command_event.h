// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: command event

#ifndef IPCS_COMMON_NETFRAME_COMMAND_EVENT_H
#define IPCS_COMMON_NETFRAME_COMMAND_EVENT_H

#include <stdint.h>
#include "any_ptr.h"

namespace netframe {

///< 标征每个操作对应的socket标识
union SocketId
{
    explicit SocketId(int64_t i = -1)
    {
        Id = i;
    }
    struct
    {
        int32_t SockFd;     ///< 32位sock fd
        int32_t SequenceId; ///< 32位序列号
    };
    bool operator == (const SocketId& sock_id) const
    {
        return Id == sock_id.Id;
    }
    int GetFd() const
    {
        return SockFd;
    }
    int64_t GetId() const
    {
        return Id;
    }
    int64_t Id;
};

enum CommandEventType
{
    ///< 占位符
    CommandEvent_Null = 0,
    ///< 添加一个Socket的命令事件，Cookie为SocketContext指针
    CommandEvent_AddSocket,
    ///< 关闭一个连接，等待之前的数据包发完
    CommandEvent_CloseSocket,
    ///< 立即关闭一个socket
    CommandEvent_CloseSocket_Now,
    ///< Socket发送包命令事件，Cookie中为Packet指针
    CommandEvent_SendPacket,
    ///< Socket发送紧急包命令事件，Cookie中为Packet指针
    CommandEvent_SendUrgentPacket,
};

struct CommandEvent
{
public:
    CommandEventType Type;  ///< Event的类型
    SocketId SockId;        ///< 事件要发给的Socket Id
    ipcs_common::AnyPtr Cookie;          ///< 事件的附带信息

    CommandEvent() : Type(CommandEvent_Null), SockId(-1), Cookie()
    {
    }

    template <typename T>
    CommandEvent(CommandEventType type, SocketId id, T* cookie) :
        Type(type), SockId(id), Cookie(cookie)
    {
    }

    CommandEvent(CommandEventType type, SocketId id) :
        Type(type), SockId(id)
    {
    }

    int32_t GetFd() const
    {
        return SockId.SockFd;
    }
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_COMMAND_EVENT_H
