// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: HttpRequest implementation

#include "http_message.h"
#include "algorithm.h"
#include "concat.h"
#include "logging.h"

// NOTE: The order must be consistent with enum values because GetMethodName
// access this table by method_type enum as index
static const struct {
    int method;
    const char* method_name;
} kValidMethodNames[] = {
    { HttpRequest::METHOD_HEAD, "HEAD" },
    { HttpRequest::METHOD_GET, "GET" },
    { HttpRequest::METHOD_POST, "POST" },
    { HttpRequest::METHOD_PUT, "PUT" },
    { HttpRequest::METHOD_DELETE, "DELETE" },
    { HttpRequest::METHOD_OPTIONS, "OPTIONS" },
    { HttpRequest::METHOD_TRACE, "TRACE" },
    { HttpRequest::METHOD_CONNECT, "CONNECT" },
    { HttpRequest::METHOD_UNKNOWN, NULL },
};

void HttpRequest::Reset() {
    HttpMessage::Reset();
    m_method = METHOD_UNKNOWN;
    m_uri = "/";
}

// static
int HttpRequest::GetMethodByName(const char* method_name) {
    for (int i = 0; ; ++i) {
        if (kValidMethodNames[i].method_name == NULL) {
            return HttpRequest::METHOD_UNKNOWN;
        }
        // Method is case sensitive.
        if (strcmp(method_name, kValidMethodNames[i].method_name) == 0) {
            return kValidMethodNames[i].method;
        }
    }
}

// static
const char* HttpRequest::GetMethodName(int method) {
    if (method <= METHOD_UNKNOWN || method >= METHOD_UPPER_BOUND) {
        return NULL;
    }
    return kValidMethodNames[method].method_name;
}

bool HttpRequest::ParseStartLine(const StringPiece& data, HttpMessage::ErrorType* error) {
    ErrorType error_placeholder;
    if (error == NULL)
        error = &error_placeholder;

    std::vector<std::string> fields;
    SplitString(data, " ", &fields);
    if (fields.size() != 2 && fields.size() != 3) {
        *error = ERROR_START_LINE_NOT_COMPLETE;
        return false;
    }

    m_method = GetMethodByName(fields[0].c_str());
    if (m_method == METHOD_UNKNOWN) {
        *error = ERROR_METHOD_NOT_FOUND;
        return false;
    }
    m_uri = fields[1];

    if (fields.size() == 3) {
        int http_version = GetVersionNumber(fields[2]);
        if (http_version == HttpMessage::VERSION_UNKNOWN) {
            *error = ERROR_VERSION_UNSUPPORTED;
            return false;
        }
        set_http_version(http_version);
    }

    return true;
}

void HttpRequest::AppendStartLineToString(std::string* result) const {
    CHECK_NE(m_method, METHOD_UNKNOWN);
    StringAppend(result,
        GetMethodName(m_method),
        " ",
        m_uri,
        " ",
        GetVersionString(http_version())
    );
}

void HttpResponse::Reset() {
    HttpMessage::Reset();
    m_status = -1;
}
