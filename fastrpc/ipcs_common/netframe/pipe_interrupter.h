// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: pipe interrupter

#ifndef IPCS_COMMON_NETFRAME_PIPE_INTERRUPTER_H
#define IPCS_COMMON_NETFRAME_PIPE_INTERRUPTER_H

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

#include "interrupter.h"

namespace netframe {

class PipeInterrupter : public Interrupter
{
public:
    PipeInterrupter() : m_ReadFd(-1), m_WriteFd(-1) {}
    bool Create()
    {
        int pipe_fds[2];
        if (pipe(pipe_fds) < 0)
            return false;

        m_ReadFd = pipe_fds[0];
        ::fcntl(m_ReadFd, F_SETFL, O_NONBLOCK);
        ::fcntl(m_ReadFd, F_SETFD, FD_CLOEXEC);

        m_WriteFd = pipe_fds[1];
        ::fcntl(m_WriteFd, F_SETFL, O_NONBLOCK);
        ::fcntl(m_WriteFd, F_SETFD, FD_CLOEXEC);
        return true;
    }

    ~PipeInterrupter()
    {
        if (m_ReadFd != -1)
            ::close(m_ReadFd);
        if (m_WriteFd != -1)
            ::close(m_WriteFd);
    }

    // Interrupt the select call.
    bool Interrupt()
    {
        char byte = 0;
        return ::write(m_WriteFd, &byte, 1) != -1;
    }

    // Reset the select interrupt. Returns true if the call was interrupted.
    bool Reset()
    {
        char data[1024];
        int bytes_read = ::read(m_ReadFd, data, sizeof(data));
        bool was_interrupted = (bytes_read > 0);
        while (bytes_read == sizeof(data))
            bytes_read = ::read(m_ReadFd, data, sizeof(data));
        return was_interrupted;
    }

    // Get the read descriptor to be passed to select.
    int GetReadFd() const
    {
        return m_ReadFd;
    }

private:
    // The read end of a connection used to interrupt the select call. This file
    // descriptor is passed to select such that when it is time to stop, a single
    // byte will be written on the other end of the connection and this
    // descriptor will become readable.
    int m_ReadFd;

    // The write end of a connection used to interrupt the select call. A single
    // byte may be written to this to wake up the select which is waiting for the
    // other end to become readable.
    int m_WriteFd;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_PIPE_INTERRUPTER_H

