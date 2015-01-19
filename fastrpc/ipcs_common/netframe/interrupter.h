// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: interrupter

#ifndef IPCS_COMMON_NETFRAME_INTERRUPTER_H
#define IPCS_COMMON_NETFRAME_INTERRUPTER_H

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

namespace netframe {

class Interrupter
{
protected:
    Interrupter() {}
public:
    /// Destructor.
    virtual ~Interrupter() { }

    virtual bool Create() = 0;

    /// Interrupt the select call.
    virtual bool Interrupt() = 0;

    /// Reset the select interrupt. Returns true if the call was interrupted.
    virtual bool Reset() = 0;

    /// Get the read descriptor to be passed to select.
    virtual int GetReadFd() const = 0;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_INTERRUPTER_H
