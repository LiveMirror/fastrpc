// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: event poller

#ifdef _WIN32
#include "wsa_event_poller.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <time.h>

namespace netframe {

static const unsigned int WM_SOCKET_EVENT = WM_USER + 1;
static const wchar_t* const WINDOW_CLASS_NAME = L"NetFreame::WsaEventPollerWindow";

ATOM WsaEventPoller::s_WndClassAtom = 0;

static LRESULT CALLBACK WSAAsyncSelect_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg < WM_USER)
        return DefWindowProc(hWnd, msg, wParam, lParam);
    else
        return 0;
}

//////////////////////////////////////////////////////////////////////////
WsaEventPoller::WsaEventPoller():
    m_hWndMessage(NULL)
{
    if (s_WndClassAtom == 0)
    {
        WNDCLASSEXW theWndClass;
        theWndClass.cbSize = sizeof(theWndClass);
        theWndClass.style = 0;
        theWndClass.lpfnWndProc = &WSAAsyncSelect_WndProc;
        theWndClass.cbClsExtra = 0;
        theWndClass.cbWndExtra = 0;
        theWndClass.hInstance = NULL;
        theWndClass.hIcon = NULL;
        theWndClass.hCursor = NULL;
        theWndClass.hbrBackground = NULL;
        theWndClass.lpszMenuName = NULL;
        theWndClass.lpszClassName = WINDOW_CLASS_NAME;
        theWndClass.hIconSm = NULL;
        s_WndClassAtom = ::RegisterClassExW(&theWndClass);
    }
    assert(s_WndClassAtom != NULL);
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

WsaEventPoller::~WsaEventPoller()
{
    if (m_hWndMessage)
        DestroyWindow(m_hWndMessage);
    CloseHandle(m_hEvent);
}

bool WsaEventPoller::RequestEvent(int fd, unsigned int event_mask)
{
    assert(event_mask != 0);

    // 转换事件掩码
    int wsa_event_mask      =   0;

    if (event_mask & IoEvent_Connected)
    {
        wsa_event_mask |= FD_CONNECT;
    }

    if (event_mask & IoEvent_Readable)
    {
        wsa_event_mask |= FD_READ;
    }

    if (event_mask & IoEvent_Acceptable)
    {
        wsa_event_mask |= FD_ACCEPT;
    }

    if (event_mask & IoEvent_Writeable)
    {
        wsa_event_mask |= FD_WRITE;
    }

    wsa_event_mask  |= FD_CLOSE;

    return ::WSAAsyncSelect(fd, m_hWndMessage, WM_SOCKET_EVENT, wsa_event_mask) == 0;
}

bool WsaEventPoller::RerequestEvent(int fd, unsigned int event_mask)
{
    /// WsaAsyncSelect 没有处女情结
    return RequestEvent(fd, event_mask);
}

bool WsaEventPoller::ClearEventRequest(int fd)
{
    assert(m_hWndMessage != NULL);
    return ::WSAAsyncSelect(fd, m_hWndMessage, 0, 0) == 0;
}

HWND WsaEventPoller::CreateHiddenWindow()
{
    return CreateWindowW(
            WINDOW_CLASS_NAME, WINDOW_CLASS_NAME,
            WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            HWND_MESSAGE,
            NULL, NULL, NULL
    );
}

/// translate WSAAsyncSelect events to our events
static unsigned int TranslateWinSockEvent(unsigned int wsa_event_mask)
{
    unsigned int result = 0;

    if (wsa_event_mask & FD_ACCEPT)
    {
        result |= IoEvent_Acceptable;
    }

    if (wsa_event_mask & FD_CONNECT)
    {
        result |= IoEvent_Connected;
    }

    if (wsa_event_mask & FD_READ)
    {
        result |= IoEvent_Readable;
    }

    if (wsa_event_mask & FD_WRITE)
    {
        result |= IoEvent_Writeable;
    }

    if (wsa_event_mask & FD_CLOSE)
    {
        result |= IoEvent_Closed;
    }

    return result;
}

bool WsaEventPoller::PollEvents(EventPoller::EventHandler* event_handler)
{
    if (m_hWndMessage == NULL)
    {
        m_hWndMessage = CreateHiddenWindow();
        if (m_hWndMessage == NULL)
            return false;
    }

    // 等待本线程有消息到达或者 m_hEvent 被 SetEvent
    DWORD n = MsgWaitForMultipleObjectsEx(1, &m_hEvent, INFINITE, QS_ALLEVENTS, 0);
    if (n == WAIT_OBJECT_0)
    {
        return event_handler->HandleInterrupt();
    }

    if (n == 0xFFFFFFFF)
        return false;

    MSG msg;
    while (::PeekMessage(&msg, m_hWndMessage, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
        case WM_SOCKET_EVENT:
        {
            int fd = (int) msg.wParam;
            if (fd >= 0)
            {
                IoEvent event;
                event.Mask = 0;
                event.Fd = fd;

                unsigned int error = WSAGETSELECTERROR(msg.lParam);

                if (error != 0)
                {
                    event.Mask = IoEvent_Closed;
                    event.ErrorCode = error;
                }
                else
                {
                    unsigned int wsa_event  = WSAGETSELECTEVENT(msg.lParam);
                    event.Mask = TranslateWinSockEvent(wsa_event);
                    event.ErrorCode = 0;
                }
                if (!event_handler->HandleIoEvent(event))
                    return false;
            }
        }
        break;
        default:
            break;
        }
    }

    return true;
}

bool WsaEventPoller::Interrupt()
{
    return SetEvent(m_hEvent) != FALSE;
}

} // namespace netframe

#endif // _WIN32
