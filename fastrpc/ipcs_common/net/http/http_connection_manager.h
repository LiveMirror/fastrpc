// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_CONNECTION_MANAGER_H
#define IPCS_COMMON_NET_HTTP_HTTP_CONNECTION_MANAGER_H

#include <set>
#include "netframe.h"
#include "condition_variable.h"
#include "mutex.h"

class ConnectionManager {
public:
    explicit ConnectionManager(netframe::NetFrame* net_frame);
    ~ConnectionManager();

    // Add a connection.
    bool AddConnection(int64_t connection_id);

    // Close a connection by id.
    void CloseConnection(int64_t connection_id, bool immediate);

    // Close all connections.
    void CloseAllConnections();

    // Remove a connection.
    void RemoveConnection(int64_t connection_id);

private:
    netframe::NetFrame* m_net_frame;
    std::set<int64_t> m_connections;

    ipcs_common::Mutex m_mutex;
    ipcs_common::ConditionVariable m_cond;
};

#endif // IPCS_COMMON_NET_HTTP_HTTP_CONNECTION_MANAGER_H
