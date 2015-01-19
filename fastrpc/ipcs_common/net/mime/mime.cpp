// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "algorithm.h"
#include "mime.h"
#include "logging.h"

namespace net {

MimeType::MapType& MimeType::DoGetMap()
{
    static MapType mime_map;
    // Load common mime types
    mime_map.insert(std::make_pair("xml", "text/xml"));
    mime_map.insert(std::make_pair("html", "text/html"));
    mime_map.insert(std::make_pair("htm", "text/html"));
    mime_map.insert(std::make_pair("txt", "text/plain"));
    mime_map.insert(std::make_pair("js", "application/x-javascript"));
    mime_map.insert(std::make_pair("ico", "image/x-icon"));

    std::ifstream file("/etc/mime.types");
    if (!file) {
        LOG(WARNING) << "error open mime.types!";
        return mime_map;
    }
    std::string line;
    std::vector<std::string> result;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        SplitStringByAnyOf(line, " \t", &result);
        for (std::vector<std::string>::iterator iter = result.begin() + 1;
             iter != result.end(); ++iter) {
            std::string ext = mime_map[*iter];
            if (ext.empty()) {
                mime_map[*iter] = result[0];
            }
        }
        line.clear();
        result.clear();
    }
    return mime_map;
}

MimeType::MapType& MimeType::GetMap()
{
    static MapType& mime_map = DoGetMap();
    return mime_map;
}

bool MimeType::Set(const std::string& mime)
{
    std::string::size_type pos = mime.find('/');
    if (pos != std::string::npos) {
        m_type = mime.substr(0, pos);
        m_subtype = mime.substr(pos + 1, mime.size() + 1);
        return true;
    }
    return false;
}

bool MimeType::Match(const MimeType& mime) const
{
    return ((m_type == mime.m_type) || (m_type == "*") || (mime.m_type == "*"))
            && ((m_subtype == mime.m_subtype) || (m_subtype == "*") || (mime.m_subtype == "*"));
}

bool MimeType::Match(const std::string& mime) const
{
    std::string::size_type pos = mime.find('/');
    if (pos != std::string::npos) {
        std::string left = mime.substr(0, pos);
        std::string right = mime.substr(pos + 1, mime.size());
        return ((m_type == left) || (m_type == "*") || (left == "*"))
            && ((m_subtype == right) || (m_subtype == "*") || (right == "*"));
    }
    return false;
}

MimeType MimeType::FromFileExtension(const std::string& ext)
{
    MapType& mime_map = GetMap();
    MapType::iterator it = mime_map.find(ext);
    if (it == mime_map.end()) {
        LOG(ERROR) << "Unknown extension name:" << ext;
        return MimeType("", "");
    }
    return MimeType(it->second);
}

} // end of namespace net
