// Copyright (c) 2013, tencent Inc.
// All rights reserved.
//
// Author: jack
// Created: 01/26/13
// Description:  A resource pool for coroutine

#ifndef _XCORE_RESOURCE_POOL_H
#define _XCORE_RESOURCE_POOL_H
#pragma once

#include <deque>
#include "coroutine.h"
#include "xcore_rpc_common.h"
#include "xcore_mutex.h"

template <typename T>
class ResourcePool
{
public:
    ResourcePool(){}
    ~ResourcePool(){}

    T* Get() {
        T* t = GetResQue();
        if (t) {
            return t;
        } else {
            CroMgr mgr = GetCroMgr();
            int croid = coroutine_running(mgr);
            ASSERT(-1 != croid);
            PutCroQue(croid);
            coroutine_yield(mgr);
            return GetResQue();
        }
    }

    bool Release(T* t) {
        PutResQue(t);
        int croid = GetCroQue();
        if (-1 != croid) {
            CroMgr mgr = GetCroMgr();
            PbClosure* cro_done =
                ::google::protobuf::NewCallback(&coroutine_resume, mgr, croid);
            RpcMgr::PutOutSideQueue(cro_done);
        }
        return true;
    }

    void PutResQue(T* t) {
        m_res_mut.lock();
        m_res_que.push_front(t);
        m_res_mut.unlock();
    }

    T* GetResQue() {
        T* ret = NULL;
        m_res_mut.lock();
        if (!m_res_que.empty()) {
            ret = m_res_que.front();
            m_res_que.pop_front();
        }
        m_res_mut.unlock();
        return ret;
    }

private:
    void PutCroQue(int croid) {
        m_cro_mut.lock();
        m_wait_cro_que.push_back(croid);
        m_cro_mut.unlock();
    }

    int GetCroQue() {
        int ret = -1;
        m_cro_mut.lock();
        if (!m_wait_cro_que.empty()) {
            ret = m_wait_cro_que.front();
            m_wait_cro_que.pop_front();
        }
        m_cro_mut.unlock();
        return ret;
    }

    std::deque<T*> m_res_que;
    std::deque<int> m_wait_cro_que;
    XMutex m_res_mut;
    XMutex m_cro_mut;
};

#endif // _XCORE_RESOURCE_POOL_H
