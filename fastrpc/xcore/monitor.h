/*
 * Copyright(C) feimat hemingzhe
 */
#ifndef __ARPC_MONITOR_H__
#define __ARPC_MONITOR_H__
#include "xcore_semaphore.h"

class Monitor {
public:
    Monitor();

    ~Monitor();

    void TimeWait(uint64 timeout_ms);

    void Wait();

    void Notify();

    bool m_has_notify;

private:
    XSemaphore xsem;
};

#endif
