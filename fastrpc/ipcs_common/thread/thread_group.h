/// Jack
#ifndef IPCS_COMMON_SYSTEM_CONCURRENCY_THREAD_GROUP_H
#define IPCS_COMMON_SYSTEM_CONCURRENCY_THREAD_GROUP_H
#pragma once

#include <vector>
#include "function.h"
#include "thread.h"

namespace ipcs_common {

class ThreadGroup
{
    DECLARE_UNCOPYABLE(ThreadGroup);
public:
    ThreadGroup();
    ThreadGroup(const Function<void ()>& callback, size_t count);
    ~ThreadGroup();
    void Add(const Function<void ()>& callback, size_t count = 1);
    void Start();
    void Join();
    size_t Size() const;
private:
    std::vector<Thread*> m_threads;
};

} // namespace ipcs_common

#endif // IPCS_COMMON_SYSTEM_CONCURRENCY_THREAD_GROUP_H
