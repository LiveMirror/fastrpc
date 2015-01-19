// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_MESSAGE_H
#define IPCS_COMMON_NET_HTTP_HTTP_MESSAGE_H

#include <map>
#include <string>
#include <vector>
#include "deprecate.h"
#include "string_piece.h"
#include "http_headers.h"

// Describes an http message, which is the base class for http request and
// response. It includes the start line, headers and body.
class HttpMessage {
public:
    enum Version {
        VERSION_UNKNOWN = 0,
        VERSION_0_9 = 9,
        VERSION_1_0 = 10,
        VERSION_1_1 = 11,
    };

    enum ErrorType {
        SUCCESS = 0,
        ERROR_NO_START_LINE,
        ERROR_START_LINE_NOT_COMPLETE,
        ERROR_VERSION_UNSUPPORTED,
        ERROR_RESPONSE_STATUS_NOT_FOUND,
        ERROR_FIELD_NOT_COMPLETE,
        ERROR_METHOD_NOT_FOUND,
        ERROR_MESSAGE_NOT_COMPLETE,
    };
    DEPRECATED_BY(SUCCESS) static const ErrorType ERROR_NORMAL = SUCCESS;

    HttpMessage() {
        m_http_version = VERSION_1_1;
    }
    virtual ~HttpMessage() {}
    virtual void Reset();

public:
    // Parse http headers (including the start line) from data.
    // return: error code which is defined as ErrorType.
    virtual bool ParseHeaders(const StringPiece& data, ErrorType* error = NULL);

    std::string StartLine() const {
        std::string result;
        AppendStartLineToString(&result);
        return result;
    }

    int http_version() const { return m_http_version; }
    void set_http_version(int version) {
        m_http_version = version;
    }

    const std::string& http_body() const { return m_http_body; }
    std::string* mutable_http_body() { return &m_http_body; }

    void set_body(const StringPiece& body) {
        m_http_body.assign(body.data(), body.size());
    }

    // string of GNU libstdc++ use reference count to reduce copy
    // keep these overloadings to help it
    void set_body(const std::string& body) {
        m_http_body = body;
    }
    void set_body(const char* body) {
        m_http_body.assign(body);
    }

    int GetContentLength();
    bool IsKeepAlive() const;

    // Get the header value.
    const HttpHeaders& headers() const {
        return m_headers;
    }

    HttpHeaders& headers() {
        return m_headers;
    }

    // Return false if it doesn't exist.
    bool GetHeader(const StringPiece& header_name, std::string** value);
    bool GetHeader(const StringPiece& header_name, const std::string** value) const;
    bool GetHeader(const StringPiece& header_name, std::string* value) const;
    std::string GetHeader(const StringPiece& header_name) const;

    // Used when a http header appears multiple times.
    // return false if it doesn't exist.
    bool GetHeaders(const StringPiece& header_name,
                    std::vector<std::string>* header_values) const;
    // Set a header field. if it exists, overwrite the header value.
    void SetHeader(const StringPiece& header_name,
                   const StringPiece& header_value);
    // Replace the header with those in parameters 'headers'
    void SetHeaders(const HttpHeaders& headers);
    // Add a header field, just append, no overwrite.
    void AddHeader(const StringPiece& header_name,
                   const StringPiece& header_value);
    // Insert the items from 'headers'
    void AddHeaders(const HttpHeaders& headers);
    // Remove an http header field.
    bool RemoveHeader(const StringPiece& header_name);

    // If has a header
    bool HasHeader(const StringPiece& header_name) const;

    // Convert start line and headers to string.
    void AppendHeadersToString(std::string* result) const;
    void HeadersToString(std::string* result) const;
    std::string HeadersToString() const;

    void AppendToString(std::string* result) const;
    void ToString(std::string* result) const;
    std::string ToString() const;

protected:
    static const char* GetVersionString(int version);
    static int  GetVersionNumber(const StringPiece& http_version);

    // append without ending "\r\n"
    virtual void AppendStartLineToString(std::string* result) const = 0;
    virtual bool ParseStartLine(const StringPiece& data, HttpMessage::ErrorType* error) = 0;
    void Swap(HttpMessage* other) {
        using std::swap;
        swap(m_http_version, other->m_http_version);
        m_headers.Swap(&other->m_headers);
        swap(m_http_body, other->m_http_body);
    }

private:
    int m_http_version;
    HttpHeaders m_headers;
    std::string m_http_body;
};

// must include here to make interface compatible
#include "http_request.h"
#include "http_response.h"

#endif // IPCS_COMMON_NET_HTTP_HTTP_MESSAGE_H
