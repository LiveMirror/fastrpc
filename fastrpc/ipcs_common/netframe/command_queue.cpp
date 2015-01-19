// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: command queue

#include "command_queue.h"
#include "command_event.h"
#include "packet.h"

namespace netframe {

CommandQueue::CommandQueue(size_t max_length):
    m_MaxLength(max_length)
{
}

CommandQueue::~CommandQueue()
{
    while (!m_Queue.empty())
    {
        CommandEvent event = m_Queue.front();
        // 发送数据事件带有数据包
        if (event.Type == CommandEvent_SendPacket ||
            event.Type == CommandEvent_SendUrgentPacket)
        {
            Packet* packet = event.Cookie.CastTo<Packet*>();
            delete packet;
        }
        m_Queue.pop_front();
    }
}

bool CommandQueue::GetFront(CommandEvent* event)
{
    if (!m_Queue.empty())
    {
        *event = m_Queue.front();
        return true;
    }
    return false;
}

bool CommandQueue::PopFront()
{
    if (!m_Queue.empty())
    {
        m_Queue.pop_front();
        return true;
    }
    return false;
}

bool CommandQueue::Enqueue(const CommandEvent& event, bool force)
{
    // 超过队列长度,返回
    if (!force && (m_Queue.size() + 1) > m_MaxLength)
        return false;
    m_Queue.push_back(event);
    return true;
}

void CommandQueue::EnqueueUrgent(const CommandEvent& event)
{
    if (!m_Queue.empty())
    {
        // 插入到第二（第一个可能正在处理）
        CommandEvent first = m_Queue.front();
        m_Queue.pop_front();
        m_Queue.push_front(event);
        m_Queue.push_front(first);
    }
    else
    {
        m_Queue.push_front(event);
    }
}

} // namespace netframe
