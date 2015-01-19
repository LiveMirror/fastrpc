// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_base.h"
#include "logging.h"

HttpBase::HttpBase(netframe::NetFrame* net_frame, bool own_net_frame, int threads_number)
    : m_own_net_frame(own_net_frame),
      m_net_frame(net_frame ? net_frame : new netframe::NetFrame(threads_number)),
      m_connection_manager(m_net_frame)
{
    if (!net_frame) {
        CHECK(!own_net_frame);
        // already constructed in initialize list
        m_own_net_frame = true;
    }
}

HttpBase::HttpBase(int threads_number)
    : m_own_net_frame(true),
    m_net_frame(new netframe::NetFrame(threads_number)),
    m_connection_manager(m_net_frame) {
}

HttpBase::~HttpBase() {
    Stop();

    if (m_own_net_frame) {
        delete m_net_frame;
    }
}

void HttpBase::Stop() {
    m_connection_manager.CloseAllConnections();
}
