// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: epoll event poller

#ifndef IPCS_COMMON_NETFRAME_EPOLL_EVENT_POLLER_H
#define IPCS_COMMON_NETFRAME_EPOLL_EVENT_POLLER_H

#include <sys/epoll.h>
#include <vector>
#include "unique_ptr.h"
#include "event_poller.h"
#include "eventfd_interrupter.h"
#include "pipe_interrupter.h"

namespace netframe {

/// EventPoller using linux epoll here
class EpollEventPoller : public EventPoller
{
public:
    explicit EpollEventPoller(unsigned int max_fds = 0x10000);
    virtual ~EpollEventPoller();

    virtual bool RequestEvent(int fd, unsigned int event_mask);
    virtual bool RerequestEvent(int fd, unsigned int event_mask);
    virtual bool ClearEventRequest(int fd);
    virtual bool PollEvents(EventPoller::EventHandler* event_handler);
    virtual bool Interrupt();
private:
    int m_EpollFd;
    std::vector<epoll_event> m_EpollEvents;  ///epoll event array
    unique_ptr<Interrupter> m_Interrupter;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_EPOLL_EVENT_POLLER_H
