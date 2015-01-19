// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: HttpRequest class declaration

#ifndef IPCS_COMMON_NET_HTTP_HTTP_REQUEST_H
#define IPCS_COMMON_NET_HTTP_HTTP_REQUEST_H
#pragma once

#include "http_message.h"
#include <algorithm>
#include <string>

// Describes a http request.
class HttpRequest : public HttpMessage {
public:
    enum MethodType {
        METHOD_UNKNOWN = -1,
        METHOD_HEAD,
        METHOD_GET,
        METHOD_POST,
        METHOD_PUT,
        METHOD_DELETE,
        METHOD_OPTIONS,
        METHOD_TRACE,
        METHOD_CONNECT,
        METHOD_UPPER_BOUND,  // no use, just label the bound.
    };

    HttpRequest() : m_method(METHOD_UNKNOWN) {
        m_uri = "/";
    }
    ~HttpRequest() {}
    virtual void Reset();

public:
    static int GetMethodByName(const char* method_name);
    static const char* GetMethodName(int method);

    int method() const { return m_method; }
    void set_method(int method) {
        m_method = method;
    }

    const std::string& uri() const { return m_uri; }
    void set_uri(const std::string& uri) {
        m_uri = uri;
    }

    void Swap(HttpRequest* other) {
        HttpMessage::Swap(other);
        using std::swap;
        swap(m_method, other->m_method);
        swap(m_uri, other->m_uri);
    }

private:
    virtual void AppendStartLineToString(std::string* result) const;
    virtual bool ParseStartLine(const StringPiece& data, ErrorType* error = NULL);

    int m_method;
    std::string m_uri;
};

// adapt to std::swap
namespace std {

template <>
inline void swap(HttpRequest& lhs, HttpRequest& rhs) {
    lhs.Swap(&rhs);
}

} // namespace std

#endif // IPCS_COMMON_NET_HTTP_HTTP_REQUEST_H
