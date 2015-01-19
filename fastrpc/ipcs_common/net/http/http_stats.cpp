// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_stats.h"

void HttpTrafficStats::Entry::SetDescription(
    const std::string& description)
{
    MutexLocker locker(m_mutex);
    m_description = description;
}

void HttpTrafficStats::Entry::AddRxBytes(int rx_bytes)
{
    MutexLocker locker(m_mutex);
    m_rx_bytes_stats.Add(rx_bytes);
}

void HttpTrafficStats::Entry::AddTxBytes(int tx_bytes)
{
    MutexLocker locker(m_mutex);
    m_tx_bytes_stats.Add(tx_bytes);
}

void HttpTrafficStats::Entry::Dump(Item* item) const
{
    MutexLocker locker(m_mutex);
    item->client_address = m_description;
    m_rx_bytes_stats.GetStatsResult(&(item->rx_stats_result));
    m_tx_bytes_stats.GetStatsResult(&(item->tx_stats_result));
}

bool HttpTrafficStats::RegisterStatsEntry(Entry* stats_entry)
{
    MutexLocker locker(m_mutex);
    std::set<const Entry*>::const_iterator it = m_stats_set.find(stats_entry);
    if (it == m_stats_set.end()) {
        m_stats_set.insert(stats_entry);
        return true;
    }

    return false;
}

void HttpTrafficStats::UnregisterStatsEntry(Entry* stats_entry)
{
    MutexLocker locker(m_mutex);
    std::set<const Entry*>::iterator it = m_stats_set.find(stats_entry);
    m_stats_set.erase(it);
}

void HttpTrafficStats::Dump(std::vector<Item>* traffic_stats_result) const
{
    traffic_stats_result->clear();
    MutexLocker locker(m_mutex);
    for (std::set<const Entry*>::const_iterator it = m_stats_set.begin();
        it != m_stats_set.end(); ++it) {
        Item item;
        (*it)->Dump(&item);
        traffic_stats_result->push_back(item);
    }
}
