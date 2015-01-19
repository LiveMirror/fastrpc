// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: work thread

#ifndef IPCS_COMMON_NETFRAME_WORK_THREAD_H
#define IPCS_COMMON_NETFRAME_WORK_THREAD_H

#include <list>
#include <vector>
#include "command_event.h"
#include "socket_context.h"
#include "mutex.h"
#include "spinlock.h"
#include "base_thread.h"

#if defined _WIN32
#include "wsa_event_poller.h"
#elif defined __linux__
#include "epoll_event_poller.h"
#else
#error not supported platform
#endif

namespace netframe {

class NetFrame;

#if defined _WIN32
typedef WsaEventPoller EventPollerType;
#else
typedef EpollEventPoller EventPollerType;
#endif

class WorkThread: public BaseThread,
    private EventPoller::EventHandler // 接受其回调
{
public:
    WorkThread(NetFrame* net_frame, size_t max_fd_value = 0x10000);
    virtual ~WorkThread();

    virtual void Entry();

    /// @brief 向该线程添加一个命令事件
    /// @param event 被添加的命令事件
    void AddCommandEvent(const CommandEvent& event);

private:
    /// 实现基类EventHandler的纯虚函数接口
    virtual bool HandleIoEvent(const IoEvent& event);
    virtual bool HandleInterrupt();

private: /// 对命令队列的处理函数
    /// @brief 处理该线程上的事件
    /// @param event 指向被处理的事件的指针
    void ProcessCommandEvent(const CommandEvent& event);

    /// @brief 获取本线程上收到的命令事件
    /// @param event 接受到的命令事件队列
    /// @retval true 有命令事件到达
    /// @retval false 没有命令事件到达
    bool GetCommandEvents(std::list<CommandEvent>& events);

    /// 把处理过的事件节点放回freelist，用作空间再利用
    void PutCommandEvents(std::list<CommandEvent>& events);

    /// @brief 清除所有的命令事件
    void ClearCommandEvents();

    /// @brief 对单个命令事件进行清理
    void ClearCommandEvent(const CommandEvent& event);
private:
    typedef Mutex LockType;
    LockType m_Lock;
    NetFrame* m_NetFrame;  ///< 调试使用，便于获取workthread对应的netframe对象
    std::vector<SocketContext*> m_SocketContexts;  ///< 线程处理的SocketContext列表
    std::list<CommandEvent> m_CommandEventList;    ///< 事件队列
    std::list<CommandEvent> m_CommandEventFreeList; ///< 事件节点缓存队列
    EventPollerType m_EventPoller;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_WORK_THREAD_H

