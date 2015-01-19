// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_STRING_PACKET_H

#include <string>
#include "packet.h"

class StringPacket : public netframe::Packet
{
public:
    explicit StringPacket(std::string* message)
    {
        m_buff_str.swap(*message);
        SetContentPtr(m_buff_str.data(), m_buff_str.size());
    }

    virtual ~StringPacket()
    {
    }

private:
    std::string m_buff_str;
};

#define IPCS_COMMON_NET_HTTP_STRING_PACKET_H
#pragma once

#endif // IPCS_COMMON_NET_HTTP_STRING_PACKET_H
