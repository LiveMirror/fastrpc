// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: work thread

#include <assert.h>
#include <string>
#include <vector>
#include "work_thread.h"

namespace netframe {

using namespace std;

WorkThread::WorkThread(NetFrame* net_frame, size_t max_fd_value):
    m_NetFrame(net_frame), m_SocketContexts(max_fd_value + 1)
{
}

WorkThread::~WorkThread()
{
    SendStopRequest();
    m_EventPoller.Interrupt();
    Join();
    m_NetFrame = NULL;
}

void WorkThread::Entry()
{
    while (m_EventPoller.PollEvents(static_cast<EventPoller::EventHandler*>(this)))
    {
        if (IsStopRequested())
            break;
    }

    // 将线程上所有尚未处理的命令清空
    ClearCommandEvents();

    for (size_t i = 0 ; i < m_SocketContexts.size(); i++)
    {
        if (m_SocketContexts[i] != NULL)
        {
            m_SocketContexts[i]->Close();
        }
        delete m_SocketContexts[i];
    }
    m_SocketContexts.clear();
}

void WorkThread::AddCommandEvent(const CommandEvent& event)
{
    LockType::Locker locker(m_Lock);
    if (!m_CommandEventFreeList.empty())
    {
        m_CommandEventList.splice(m_CommandEventList.end(),
                m_CommandEventFreeList, m_CommandEventFreeList.begin());
        m_CommandEventList.back() = event;
    }
    else
    {
        m_CommandEventList.push_back(event);
    }
    m_EventPoller.Interrupt();
}

bool WorkThread::GetCommandEvents(std::list<CommandEvent>& events)
{
    LockType::Locker locker(m_Lock);
    if (!m_CommandEventList.empty())
    {
        std::swap(events, m_CommandEventList);
        return true;
    }
    return false;
}

void WorkThread::PutCommandEvents(std::list<CommandEvent>& events)
{
    LockType::Locker locker(m_Lock);
    m_CommandEventFreeList.splice(m_CommandEventFreeList.end(), events);
}

bool WorkThread::HandleIoEvent(const IoEvent& event)
{
    if (m_SocketContexts[event.Fd])
    {
        if (m_SocketContexts[event.Fd]->ProcessIoEvent(event) < 0)
        {
            delete m_SocketContexts[event.Fd];
            m_SocketContexts[event.Fd] = NULL;
        }
    }
    return true;
}

bool WorkThread::HandleInterrupt()
{
    std::list<CommandEvent> events;
    if (GetCommandEvents(events))
    {
        std::list<CommandEvent>::iterator i;
        for (i = events.begin(); i != events.end(); ++i)
        {
            ProcessCommandEvent(*i);
        }
        PutCommandEvents(events);
    }
    return true;
}

void WorkThread::ProcessCommandEvent(const CommandEvent& event)
{
    int fd = event.GetFd();
    int result = 0;

    switch (event.Type)
    {
    case CommandEvent_AddSocket:
        assert(fd >= 0 && fd < (int)m_SocketContexts.size());
        assert(m_SocketContexts[fd] == NULL);
        m_SocketContexts[fd] = event.Cookie.CastTo<SocketContext*>();
        m_SocketContexts[fd]->SetEventPoller(&m_EventPoller);
        result = m_SocketContexts[fd]->ProcessCommandEvent(event);
        break;
    case CommandEvent_CloseSocket_Now:
        if (m_SocketContexts[fd] != NULL)
        {
            m_SocketContexts[fd]->Close();
            delete m_SocketContexts[fd];
            m_SocketContexts[fd] = NULL;
            result = 1;
        }
        break;
    default:
        // 判断端口上下文存在，且前后序列号一致
        if (m_SocketContexts[fd] != NULL &&
            event.SockId == m_SocketContexts[fd]->GetSockId())
        {
            result = m_SocketContexts[fd]->ProcessCommandEvent(event);
        }
        break;
    }

    if (result < 0)
    {
        delete m_SocketContexts[fd];
        m_SocketContexts[fd] = NULL;
    }
    else if (result == 0)
    {
        ClearCommandEvent(event);
    }
}

void WorkThread::ClearCommandEvents()
{
    LockType::Locker locker(m_Lock);
    while (!m_CommandEventList.empty())
    {
        ClearCommandEvent(m_CommandEventList.front());
        m_CommandEventList.pop_front();
    }
}

void WorkThread::ClearCommandEvent(const CommandEvent& event)
{
    switch (event.Type)
    {
    case CommandEvent_SendPacket:
    case CommandEvent_SendUrgentPacket:
        assert(!event.Cookie.IsNull());
        m_NetFrame->DecreaseSendBufferedLength(event.Cookie.CastTo<Packet*>()->Length());
        m_NetFrame->DecreaseSendBufferedPacket();
        delete event.Cookie.CastTo<Packet*>();
        break;
    case CommandEvent_AddSocket:
        {
            assert(!event.Cookie.IsNull());
            SocketContext* socket_context = event.Cookie.CastTo<SocketContext*>();
            if (socket_context) {
                socket_context->Close();
                delete socket_context;
            }
        }
        break;
    default:
        break;
    }
}

} // namespace netframe
