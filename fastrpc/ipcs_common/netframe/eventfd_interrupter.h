// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: interrupter

#ifndef IPCS_COMMON_NETFRAME_EVENTFD_INTERRUPTER_H
#define IPCS_COMMON_NETFRAME_EVENTFD_INTERRUPTER_H

#include "eventfd.h"
#include <stdexcept>
#include "interrupter.h"

namespace netframe {

class EventFdInterrupter : public Interrupter
{
public:
    EventFdInterrupter() : m_Fd(-1) {}

    bool Create()
    {
        m_Fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        return m_Fd >= 0;
    }

    ~EventFdInterrupter()
    {
        if (m_Fd >= 0)
        {
            ::close(m_Fd);
            m_Fd = -1;
        }
    }

    // Interrupt the select call.
    bool Interrupt()
    {
        return eventfd_write(m_Fd, 1) == 0;
    }

    // Reset the select interrupt. Returns true if the call was interrupted.
    bool Reset()
    {
        eventfd_t data;
        return eventfd_read(m_Fd, &data) == 0;
    }

    // Get the read descriptor to be passed to select.
    int GetReadFd() const
    {
        return m_Fd;
    }

private:
    int m_Fd;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_EVENTFD_INTERRUPTER_H

