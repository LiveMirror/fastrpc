// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "http_stats.h"
#include "gtest.h"

TEST(HttpTrafficStats, Dump)
{
    HttpTrafficStats stats;
    TrafficStatsEntry entry1, entry2;
    entry1.SetDescription("entry1");
    entry2.SetDescription("entry2");
    TrafficStatsItem item1, item2;
    std::vector<TrafficStatsItem> stats_result;
    EXPECT_TRUE(stats.RegisterStatsEntry(&entry1));
    EXPECT_TRUE(stats.RegisterStatsEntry(&entry2));
    // register entry1 again should return false
    EXPECT_FALSE(stats.RegisterStatsEntry(&entry1));
    entry1.AddTxBytes(10);
    entry1.AddTxBytes(20);
    entry1.AddRxBytes(10);
    entry2.AddTxBytes(20);
    entry2.AddRxBytes(20);
    entry2.AddRxBytes(40);
    stats.Dump(&stats_result);
    EXPECT_EQ(2U, stats_result.size());
    for (size_t i = 0; i < stats_result.size(); ++i) {
        if (stats_result[i].client_address == "entry1") {
            EXPECT_EQ(30, stats_result[i].tx_stats_result.total_count);
            EXPECT_EQ(10, stats_result[i].rx_stats_result.total_count);
        } else if (stats_result[i].client_address == "entry2") {
            EXPECT_EQ(20, stats_result[i].tx_stats_result.total_count);
            EXPECT_EQ(60, stats_result[i].rx_stats_result.total_count);
        }
    }
    stats.UnregisterStatsEntry(&entry1);
    stats.UnregisterStatsEntry(&entry2);
}
