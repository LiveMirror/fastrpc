// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: command queue

#ifndef IPCS_COMMON_NETFRAME_COMMAND_QUEUE_H
#define IPCS_COMMON_NETFRAME_COMMAND_QUEUE_H

#include <deque>
#include "uncopyable.h"
#include "command_event.h"

namespace netframe {

struct CommandEvent;

class CommandQueue
{
public:
    explicit CommandQueue(size_t max_length);
    ~CommandQueue();
    bool IsEmpty() const;
    size_t Size() const;
    bool HasMore() const; ///< 队列不空
    bool GetFront(CommandEvent* event);
    bool PopFront();

    /// @brief 插入队列
    /// @param event 入队事件
    /// @param force 是否强制入队，若强制入队，忽略容量限制
    /// @retval true 成功
    bool Enqueue(const CommandEvent& event, bool force = false);

    /// @brief 紧急优先插队，忽略容量限制
    void EnqueueUrgent(const CommandEvent& event);

    DECLARE_UNCOPYABLE(CommandQueue);

private:
    std::deque<CommandEvent> m_Queue;
    size_t m_MaxLength;
};

inline bool CommandQueue::IsEmpty() const
{
    return m_Queue.empty();
}

inline size_t CommandQueue::Size() const
{
    return m_Queue.size();
}

inline bool CommandQueue::HasMore() const
{
    return m_Queue.size() > 1;
}

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_COMMAND_QUEUE_H
