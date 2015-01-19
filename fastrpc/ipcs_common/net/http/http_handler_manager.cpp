// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_handler_manager.h"
#include "concat.h"
#include "logging.h"

using namespace ipcs_common;

HttpHandlerManager::HttpHandlerManager() {
}

HttpHandlerManager::~HttpHandlerManager() {
    Clear();
}

void HttpHandlerManager::Clear() {
    RWLock::WriterLocker locker(m_handler_lock);
    std::set<HttpHandler*>::iterator iter;
    for (iter = m_handlers.begin(); iter != m_handlers.end(); ++iter) {
        delete *iter;
    }
    m_handler_map.clear();
    m_handlers.clear();
}

void HttpHandlerManager::RegisterHandler(const std::string& path,
                                         HttpHandler* handler) {
    CHECK_NOTNULL(handler);
    RWLock::WriterLocker locker(m_handler_lock);
    std::map<std::string, HttpHandler*>::iterator iter;
    iter = m_handler_map.find(path);
    if (iter != m_handler_map.end())
        LOG(INFO) << "handler on the specified path ["
                  << path << "] has been replaced";
    m_handler_map[path] = handler;
    m_handlers.insert(handler);
}

void HttpHandlerManager::RegisterPrefixHandler(const std::string& path,
                                               HttpHandler* handler) {
    CHECK_NOTNULL(handler);
    RWLock::WriterLocker locker(m_handler_lock);

    if (m_prefix_map.Remove(path)) {
        LOG(INFO) << "handler on the specified prefix path ["
            << path << "] has been replaced";
    }

    CHECK(m_prefix_map.InsertInclusive(path, handler))
        << "handler on the specified prefix path ["
        << path << "] replace failed";

    m_handlers.insert(handler);
}


HttpHandler* HttpHandlerManager::FindHandler(const std::string& path) {
    RWLock::ReaderLocker locker(m_handler_lock);

    std::map<std::string, HttpHandler*>::iterator iter;
    iter = m_handler_map.find(path);
    if (iter != m_handler_map.end()) {
        return iter->second;
    }

    // Find prefix handler
    HttpHandler* result = NULL;
    if (m_prefix_map.Find(path, &result)) {
        return result;
    }
    return NULL;
}
