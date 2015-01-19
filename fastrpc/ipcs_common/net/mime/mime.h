// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_MIME_MIME_H
#define IPCS_COMMON_NET_MIME_MIME_H
#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include "once.h"

namespace net {

class MimeType
{
public:
    MimeType() {}

    MimeType(const std::string& type, const std::string& subtype)
        :m_type(type), m_subtype(subtype) {}

    explicit MimeType(const std::string& mime)
    {
        if (!Set(mime))
            throw std::runtime_error("Invalid MIME: " + mime);
    }

    void Set(const std::string& type, const std::string& subtype)
    {
        m_type = type;
        m_subtype = subtype;
    }

    bool Set(const std::string& mime);

    const std::string& Type() const
    {
        return m_type;
    }

    const std::string& SubType() const
    {
        return m_subtype;
    }

    bool Match(const MimeType& mime) const;

    bool Match(const std::string& mime) const;

    bool Empty()
    {
        return (m_type.empty() && m_subtype.empty());
    }
    /// convert mime to string
    std::string ToString() const
    {
        return m_type + "/" + m_subtype;
    }

public:
    // .xml -> text/xml
    //  /etc/mime.types
    static MimeType FromFileExtension(const std::string& ext);

private:
    typedef std::map<std::string, std::string> MapType;
    // Initialize mime map
    static MapType& GetMap();
    static MapType& DoGetMap();

    std::string m_type;
    std::string m_subtype;
    static Once s_once;
};

} // end namespace net

#endif // IPCS_COMMON_NET_MIME_MIME_H
