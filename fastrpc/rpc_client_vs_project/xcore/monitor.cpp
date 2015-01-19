/*
 * Copyright(C) feimat hemingzhe
 */
#include "monitor.h"

Monitor::Monitor() {
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
}

