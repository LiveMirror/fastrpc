// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: HttpHeaders class

#ifndef IPCS_COMMON_NET_HTTP_HTTP_HEADERS_H
#define IPCS_COMMON_NET_HTTP_HTTP_HEADERS_H
#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "string_piece.h"

// Store http headers information
class HttpHeaders
{
public:
    // Return false if it doesn't exist.
    bool Get(const StringPiece& header_name, std::string** value);
    bool Get(const StringPiece& header_name, const std::string** value) const;
    bool Get(const StringPiece& header_name, std::string* value) const;

    // Used when a http header appears multiple times.
    // return false if it doesn't exist.
    bool Get(const StringPiece& header_name,
             std::vector<std::string>* header_values) const;
    // Set a header field. if it exists, overwrite the header value.
    HttpHeaders& Set(const StringPiece& header_name,
                     const StringPiece& header_value);
    // Add a header field, just append, no overwrite.
    HttpHeaders& Add(const StringPiece& header_name,
                     const StringPiece& header_value);
    // Add all the header fields in rhs into this. no overwrite.
    HttpHeaders& Add(const HttpHeaders& rhs);

    // Remove an http header field.
    bool Remove(const StringPiece& header_name);

    // Get count of header
    size_t Count() const;

    // Get header by index
    bool GetAt(int index, std::pair<std::string, std::string>* header) const;

    // If has a header
    bool Has(const StringPiece& header_name) const;

    // Convert start line and headers to string.
    void AppendToString(std::string* result) const;
    void ToString(std::string* result) const;
    std::string ToString() const;

    bool Parse(const StringPiece& data, int* error = NULL);

    void Clear();

    void Swap(HttpHeaders* rhs);

private:
    std::vector<std::pair<std::string, std::string> > m_headers;
};

// adapt to std::swap
namespace std {
template <>
inline void swap(HttpHeaders& lhs, HttpHeaders& rhs) {
    lhs.Swap(&rhs);
}
} // namespace std

#endif // IPCS_COMMON_NET_HTTP_HTTP_HEADERS_H
