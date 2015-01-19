// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: epoll event poller

#ifdef __linux__

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <new>
#include <stdexcept>
#include "epoll_event_poller.h"

namespace netframe {

EpollEventPoller::EpollEventPoller(unsigned int max_fds):
    m_EpollEvents(max_fds)
{
    m_Interrupter.reset(new EventFdInterrupter());
    if (!m_Interrupter->Create())
    {
        m_Interrupter.reset(new PipeInterrupter());
        if (!m_Interrupter->Create())
            throw std::runtime_error("Can't create interrupter");
    }

    m_EpollFd = epoll_create(max_fds);
    if (m_EpollFd >= 0)
    {
        ::fcntl(m_EpollFd, F_SETFD, FD_CLOEXEC);
        RequestEvent(m_Interrupter->GetReadFd(), IoEvent_Readable);
    }
    else
    {
        int error = errno;
        std::string message = "EventPoller: can't create epoll fd: ";
        message += strerror(error);
        throw std::runtime_error(message);
    }
}

EpollEventPoller::~EpollEventPoller()
{
    close(m_EpollFd);
}

static unsigned int IoEventMaskToEpollMask(unsigned int epoll_event_mask)
{
    unsigned int result = 0;

    if (epoll_event_mask & IoEvent_Acceptable)
    {
        result |= EPOLLIN | EPOLLHUP;
    }

    if (epoll_event_mask & IoEvent_Readable)
    {
        result |= EPOLLIN | EPOLLHUP;
    }

    if (epoll_event_mask & IoEvent_Writeable)
    {
        result |= EPOLLOUT;
    }

    return result;
}

bool EpollEventPoller::RequestEvent(int fd, unsigned int event_mask)
{
    assert(event_mask != 0);

    struct epoll_event event = {};
    event.data.fd = fd;
    event.events = IoEventMaskToEpollMask(event_mask);

    return (epoll_ctl(m_EpollFd, EPOLL_CTL_ADD, fd, &event) == 0);
}

bool EpollEventPoller::RerequestEvent(int fd, unsigned int event_mask)
{
    struct epoll_event event = {};
    event.data.fd = fd;
    event.events = IoEventMaskToEpollMask(event_mask);
    return epoll_ctl(m_EpollFd, EPOLL_CTL_MOD, fd, &event) == 0;
}

bool EpollEventPoller::ClearEventRequest(int fd)
{
    struct epoll_event event = {};
    return epoll_ctl(m_EpollFd, EPOLL_CTL_DEL, fd, &event) == 0;
}

bool EpollEventPoller::PollEvents(EventPoller::EventHandler* event_handler)
{
    // 等待有Socket事件的触发，阻塞
    int num_events;
    int interrupter_fd = m_Interrupter->GetReadFd();

    do
    {
        num_events = epoll_wait(m_EpollFd, &m_EpollEvents[0], m_EpollEvents.size(), -1);
    } while (num_events == -1 && errno == EINTR);

    if (num_events < 0)
        return false;

    // 对Epoll上的所有IO事件进行处理
    for (int i = 0; i < num_events; i++)
    {
        int fd = m_EpollEvents[i].data.fd;
        if (fd == interrupter_fd) // is from interrupter
        {
            m_Interrupter->Reset();
            if (event_handler->HandleInterrupt() < 0)
                return false;
        }
        else
        {
            IoEvent event = { fd, 0 };

            if (m_EpollEvents[i].events & (EPOLLHUP | EPOLLERR))
            {
                // 若事件为出错事件，不管其他事件
                event.Mask |=  IoEvent_Closed;
            }
            else
            {
                if (m_EpollEvents[i].events & (EPOLLIN | EPOLLRDBAND | EPOLLRDNORM))
                {
                    event.Mask |= IoEvent_Readable;
                }

                if (m_EpollEvents[i].events & (EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND))
                {
                    event.Mask |= IoEvent_Writeable;
                }
            }
            if (event_handler->HandleIoEvent(event) < 0)
                return false;
        }
    }

    return true;
}

bool EpollEventPoller::Interrupt()
{
    return m_Interrupter->Interrupt();
}

} // namespace netframe

#endif // __linux__
