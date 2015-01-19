// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_connection_manager.h"
#include "netframe.h"
#include "logging.h"

ConnectionManager::ConnectionManager(netframe::NetFrame* net_frame) {
    m_net_frame = net_frame;
}

// Connection manager destructor
ConnectionManager::~ConnectionManager() {
    CloseAllConnections();
}

// Close all connections.
void ConnectionManager::CloseAllConnections() {
    std::set<int64_t>::iterator iter;
    MutexLocker locker(m_mutex);
    for (iter = m_connections.begin(); iter != m_connections.end();
            ++iter) {
        // close after data was sent.
        netframe::NetFrame::EndPoint endpoint(*iter);
        m_net_frame->CloseEndPoint(endpoint, false);
    }
    // Wait until all connections are closed.
    while (m_connections.size() > 0) {
        m_cond.Wait(&m_mutex);
    }
}

// Add a connection
bool ConnectionManager::AddConnection(int64_t connection_id) {
    MutexLocker locker(m_mutex);
    return m_connections.insert(connection_id).second;
}

// Remove a connection
void ConnectionManager::RemoveConnection(int64_t connection_id) {
    std::set<int64_t>::iterator iter;
    MutexLocker locker(m_mutex);
    iter = m_connections.find(connection_id);
    if (iter == m_connections.end()) {
        return;
    }
    m_connections.erase(iter);
    m_cond.Signal();
}

// Close a connection
void ConnectionManager::CloseConnection(int64_t connection_id, bool immediate) {
    std::set<int64_t>::iterator iter;
    MutexLocker locker(m_mutex);
    iter = m_connections.find(connection_id);
    if (iter == m_connections.end()) {
        return;
    }
    netframe::NetFrame::EndPoint endpoint(connection_id);
    m_net_frame->CloseEndPoint(endpoint, immediate);
}
