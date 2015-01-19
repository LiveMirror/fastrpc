// Copyright (c) 2013, Baidu Inc.
// All rights reserved.
//
// Author: feimat <feimat@qq.com>
// Created: 01/31/13
// Description: This is UnCopyable details

#ifndef IPCS_COMMON_BASE_UNCOPYABLE_H
#define IPCS_COMMON_BASE_UNCOPYABLE_H
#pragma once

namespace  ipcs_common
{

class UnCopyable
{
protected:
    UnCopyable() {}
    virtual ~UnCopyable() {}

private:
    UnCopyable(const UnCopyable& u);
    UnCopyable& operator=(const UnCopyable& u);
};

} // namespace ipcs_common

// convenient approach
typedef ipcs_common::UnCopyable UnCopyable;

/// the macro way
#define DECLARE_UNCOPYABLE(Class) \
private: \
    Class(const Class&); \
    Class& operator=(const Class&)

#endif // IPCS_COMMON_BASE_UNCOPYABLE_H
