// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_NET_HTTP_HTTP_STATS_H
#define IPCS_COMMON_NET_HTTP_HTTP_STATS_H

#include <set>
#include <string>
#include <vector>
#include "timed_stats.h"
#include "uncopyable.h"
#include "mutex.h"

class HttpTrafficStats
{
    DECLARE_UNCOPYABLE(HttpTrafficStats);

public:
    struct Item
    {
        std::string client_address;
        CountingStatsResult<int64_t> rx_stats_result;
        CountingStatsResult<int64_t> tx_stats_result;
    };

    class Entry
    {
    public:
        Entry(): m_rx_bytes_stats("RX bytes", "bytes"),
                 m_tx_bytes_stats("TX bytes", "bytes") {}

        void SetDescription(const std::string& description);
        void AddRxBytes(int rx_bytes);
        void AddTxBytes(int tx_bytes);
        void Dump(Item* item) const;

    private:
        mutable ipcs_common::Mutex m_mutex;
        std::string m_description;
        CountingStats<int64_t> m_rx_bytes_stats;
        CountingStats<int64_t> m_tx_bytes_stats;
    };

    HttpTrafficStats() {}
    bool RegisterStatsEntry(Entry* stats_entry);
    void UnregisterStatsEntry(Entry* stats_entry);
    void Dump(std::vector<Item>* traffic_stats_result) const;

private:
    mutable ipcs_common::Mutex m_mutex;
    std::set<const Entry*> m_stats_set;
};

typedef HttpTrafficStats::Entry TrafficStatsEntry;
typedef HttpTrafficStats::Item TrafficStatsItem;

#endif // IPCS_COMMON_NET_HTTP_HTTP_STATS_H
