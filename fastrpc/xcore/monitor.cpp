/*
 * Copyright(C) feimat hemingzhe
 */
#include "monitor.h"

Monitor::Monitor() {
    m_has_notify = false;
}

Monitor::~Monitor() {
}

void Monitor::Wait() {
    xsem.wait();
}

void Monitor::TimeWait(uint64 timeout_ms) {
    if (timeout_ms <= 0)
        return;
    xsem.trywait((uint32)timeout_ms);
}

void Monitor::Notify() {
    xsem.post();
    m_has_notify = true;
}

