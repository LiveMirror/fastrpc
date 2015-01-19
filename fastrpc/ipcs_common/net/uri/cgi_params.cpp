// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "cgi_params.h"
#include <iso646.h>
#include <stddef.h>
#include "algorithm.h"
#include "string_number.h"
#include "percent.h"
#include "uri.h"

bool CgiParams::Parse(const std::string& params)
{
    Clear();
    std::vector<std::string> splited;
    SplitString(params, "&", &splited);
    for (size_t i = 0; i < splited.size(); ++i)
    {
        m_params.push_back(CgiParam());
        size_t pos = splited[i].find('=');
        if (pos != std::string::npos)
        {
            m_params.back().name.assign(splited[i], 0, pos);
            m_params.back().value.assign(splited[i], pos + 1, std::string::npos);
            if (not PercentEncoding::Decode(&m_params.back().value))
                return false;
        }
        else
        {
            m_params.back().name = splited[i];
        }
    }
    return true;
}

bool CgiParams::ParseFromUrl(const std::string& url)
{
    net::URI uri;
    if (!uri.Parse(url) || !uri.HasQuery())
        return false;
    return Parse(uri.Query());
}

void CgiParams::AppendToString(std::string* target) const
{
    for (size_t i = 0; i < m_params.size(); ++i)
    {
        if (not m_params[i].name.empty())
        {
            const CgiParam& param = m_params[i];
            target->append(param.name);
            target->push_back('=');
            target->append(PercentEncoding::Encode(param.value));
            if (i != m_params.size() - 1)
                target->push_back('&');
        }
    }
}

void CgiParams::WriteToString(std::string* target) const
{
    target->clear();
    AppendToString(target);
}

std::string CgiParams::ToString() const
{
    std::string result;
    AppendToString(&result);
    return result;
}

CgiParam* CgiParams::Find(const std::string& name)
{
    for (size_t i = 0; i < m_params.size(); ++i)
    {
        if (m_params[i].name == name)
            return &m_params[i];
    }
    return NULL;
}

const CgiParam* CgiParams::Find(const std::string& name) const
{
    return const_cast<CgiParams*>(this)->Find(name);
}

CgiParam& CgiParams::Get(size_t index)
{
    return m_params.at(index);
}

const CgiParam& CgiParams::Get(size_t index) const
{
    return m_params.at(index);
}

bool CgiParams::GetValue(const std::string& name, std::string* value) const
{
    const CgiParam* param = Find(name);
    if (param)
    {
        *value = param->value;
        return true;
    }
    return false;
}

bool CgiParams::GetValue(const std::string& name, int32_t* value) const
{
    const CgiParam* param = Find(name);
    if (param)
    {
        return StringToNumber(param->value, value);
    }
    return false;
}

const std::string& CgiParams::GetOrDefaultValue(
    const std::string& name,
    const std::string& default_value) const
{
    const CgiParam* param = Find(name);
    if (param)
    {
        return param->value;
    }
    return default_value;
}

size_t CgiParams::Count() const
{
    return m_params.size();
}

void CgiParams::Clear()
{
    m_params.clear();
}

void CgiParams::Add(const CgiParam& param)
{
    m_params.push_back(param);
}

void CgiParams::Add(const std::string& name, const std::string& value)
{
    m_params.push_back(CgiParam());
    m_params.back().name = name;
    m_params.back().value = value;
}

void CgiParams::Set(const std::string& name, const std::string& value)
{
    CgiParam* param = Find(name);
    if (param)
        param->value = value;
    else
        Add(name, value);
}

bool CgiParams::Remove(const std::string& name)
{
    std::vector<CgiParam>::iterator iter;
    for (iter = m_params.begin(); iter != m_params.end(); ++iter) {
        if ((*iter).name == name) {
            m_params.erase(iter);
            return true;
        }
    }
    return false;
}
