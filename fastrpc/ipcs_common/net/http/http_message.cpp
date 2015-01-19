// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_message.h"

#include "algorithm.h"
#include "concat.h"
#include "string_number.h"

static const struct {
    int version_number;
    const char* version_string;
} kHttpVersions[] = {
    { HttpMessage::VERSION_0_9, "HTTP/0.9" },
    { HttpMessage::VERSION_1_0, "HTTP/1.0" },
    { HttpMessage::VERSION_1_1, "HTTP/1.1" },
    { HttpMessage::VERSION_UNKNOWN, NULL },
};

void HttpMessage::Reset() {
    m_http_version = VERSION_1_1;
    m_headers.Clear();
    m_http_body.clear();
}

void HttpMessage::AppendHeadersToString(std::string* result) const {
    AppendStartLineToString(result);
    result->append("\r\n");
    m_headers.AppendToString(result);
    result->append("\r\n");
}

void HttpMessage::HeadersToString(std::string* result) const {
    result->clear();
    AppendHeadersToString(result);
}

std::string HttpMessage::HeadersToString() const {
    std::string result;
    AppendHeadersToString(&result);
    return result;
}

void HttpMessage::AppendToString(std::string* result) const
{
    AppendHeadersToString(result);
    result->append(m_http_body);
}

void HttpMessage::ToString(std::string* result) const
{
    result->clear();
    AppendToString(result);
}

std::string HttpMessage::ToString() const
{
    std::string result;
    AppendToString(&result);
    return result;
}

// Get a header value. return false if it does not exist.
// the header name is not case sensitive.
bool HttpMessage::GetHeader(const StringPiece& header_name,
                            std::string** header_value) {
    return m_headers.Get(header_name, header_value);
}

bool HttpMessage::GetHeader(const StringPiece& header_name,
                            const std::string** header_value) const {
    return m_headers.Get(header_name, header_value);
}

bool HttpMessage::GetHeader(
        const StringPiece& header_name,
        std::string* value) const {
    const std::string* pvalue;
    if (GetHeader(header_name, &pvalue)) {
        *value = *pvalue;
        return true;
    }
    return false;
}

std::string HttpMessage::GetHeader(const StringPiece& header_name) const {
    std::string header_value;
    GetHeader(header_name, &header_value);
    return header_value;
}

// Used when a http header appears multiple times.
// return false if it doesn't exist.
bool HttpMessage::GetHeaders(const StringPiece& header_name,
                             std::vector<std::string>* header_values) const {
    return m_headers.Get(header_name, header_values);
}

// Set a header field. if it exists, overwrite the header value.
void HttpMessage::SetHeader(const StringPiece& header_name,
                            const StringPiece& header_value) {
    m_headers.Set(header_name, header_value);
}

void HttpMessage::SetHeaders(const HttpHeaders& headers) {
    m_headers = headers;
}

// Add a header field, just append, no overwrite.
void HttpMessage::AddHeader(const StringPiece& header_name,
                            const StringPiece& header_value) {
    m_headers.Add(header_name, header_value);
}

void HttpMessage::AddHeaders(const HttpHeaders& headers) {
    m_headers.Add(headers);
}

bool HttpMessage::RemoveHeader(const StringPiece& header_name) {
    return m_headers.Remove(header_name);
}

bool HttpMessage::HasHeader(const StringPiece& header_name) const {
    return m_headers.Has(header_name);
}

const char* HttpMessage::GetVersionString(int version) {
    for (int i = 0; ; ++i) {
        if (kHttpVersions[i].version_number == VERSION_UNKNOWN) {
            return NULL;
        }
        if (version == kHttpVersions[i].version_number) {
            return kHttpVersions[i].version_string;
        }
    }
}

int HttpMessage::GetVersionNumber(const StringPiece& http_version) {
    for (int i = 0; ; ++i) {
        if (kHttpVersions[i].version_string == NULL) {
            return HttpMessage::VERSION_UNKNOWN;
        }
        if (http_version.ignore_case_equal(kHttpVersions[i].version_string)) {
            return kHttpVersions[i].version_number;
        }
    }
}

// class HttpMessage
bool HttpMessage::ParseHeaders(const StringPiece& data, HttpMessage::ErrorType* error) {
    HttpMessage::ErrorType error_placeholder;
    if (error == NULL)
        error = &error_placeholder;

    StringPiece::size_type pos = data.find_first_of('\n');
    if (pos == StringPiece::npos) {
        pos = data.size();
    }
    std::string first_line =
        StringTrimRight(data.substr(0, pos), "\r");

    if (first_line.empty()) {
        *error = HttpMessage::ERROR_NO_START_LINE;
        return false;
    }

    if (!ParseStartLine(first_line, error))
        return false;

    int error_code = 0;
    bool result = m_headers.Parse(data.substr(pos + 1), &error_code);
    *error = static_cast<HttpMessage::ErrorType>(error_code);
    return result;
}

int HttpMessage::GetContentLength() {
    std::string content_length;
    if (!GetHeader("Content-Length", &content_length)) {
        return -1;
    }
    int length = 0;
    bool ret = StringToNumber(content_length, &length);
    return (ret && length >= 0) ? length : -1;
};

bool HttpMessage::IsKeepAlive() const {
    const std::string* alive;
    if (!GetHeader("Connection", &alive)) {
        if (m_http_version < VERSION_1_1) {
            return false;
        }
        return true;
    }
    return strcasecmp(alive->c_str(), "keep-alive") == 0;
}
