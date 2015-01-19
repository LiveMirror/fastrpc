// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_HANDLER_MANAGER_H
#define IPCS_COMMON_NET_HTTP_HTTP_HANDLER_MANAGER_H

#include <map>
#include <set>
#include <string>
#include "string_piece.h"
#include "prefix_map.h"
#include "http_handler.h"
#include "http_message.h"
#include "rwlock.h"

class HttpHandlerManager {
public:
    HttpHandlerManager();
    ~HttpHandlerManager();

    // Clear all handlers.
    void Clear();

    // Register a handler on a specified path.
    // Return false if another handler already has been registered.
    // If a handler has been registered successfully, it will be taken over
    // by the handler manager and CANNOT be unregistered.
    void RegisterHandler(const std::string& path, HttpHandler* handler);
    // Register a handler on a prefix path.
    void RegisterPrefixHandler(const std::string& path, HttpHandler* handler);
    // Find a handler registered on a  path.
    // Fist find specific path, if no handler exist then find a prefix path.
    // If no handler is registered, NULL will be returned.
    HttpHandler* FindHandler(const std::string& path);

private:
    ipcs_common::RWLock m_handler_lock;
    std::set<HttpHandler*> m_handlers;
    std::map<std::string, HttpHandler*>  m_handler_map;
    ipcs_common::PrefixMap<HttpHandler*> m_prefix_map;
};

#endif // IPCS_COMMON_NET_HTTP_HTTP_HANDLER_MANAGER_H
