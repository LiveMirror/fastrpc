// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: event poller

#ifndef IPCS_COMMON_NETFRAME_WSA_EVENT_POLLER_H
#define IPCS_COMMON_NETFRAME_WSA_EVENT_POLLER_H

#ifndef _WIN32
#error for Windows only
#endif

#define WIN32_LEAN_AND_MEAN

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <winsock2.h>
#include "common_windows.h"
#include "event_poller.h"

namespace netframe {

class WsaEventPoller : public EventPoller
{
public:
    WsaEventPoller();
    ~WsaEventPoller();

    /// @brief 在一个Fd上请求事件
    /// @param fd Fd的文件描述符
    /// @param event_mask 请求的事件掩码
    /// @return 成功或者失败
    virtual bool RequestEvent(int fd, unsigned int event_mask);

    /// @brief 在一个Socket上重新请求事件
    /// @param fd Fd的文件描述符
    /// @param event_mask 请求的事件掩码
    /// @return 成功或者失败
    virtual bool RerequestEvent(int fd, unsigned int event_mask);

    /// @brief 清除一个Fd上的所有事件请求
    /// @param fd 被清除的Fd
    virtual bool ClearEventRequest(int fd);

    /// @brief 等待事件的触发，该函数诸塞
    /// @param events 已经触发了的事件列表
    /// @retval true 正常
    /// @retval false 出现致命错误
    bool PollEvents(EventPoller::EventHandler* event_handler);

    bool Interrupt();
private:
    static HWND CreateHiddenWindow();
private:
    static ATOM s_WndClassAtom;
    HWND m_hWndMessage;  ///< 消息窗口的句柄
    HANDLE m_hEvent;
};

} // namespace netframe

#endif // IPCS_COMMON_NETFRAME_WSA_EVENT_POLLER_H
