// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 08/01/13
// Description: csv test file

#include "csv_parser.h"
#include "gtest.h"

TEST(CsvTest, BasicTest) {
    using namespace ipcs_common;

    CsvParser csv(true);

    csv.Load("goods.csv");

    std::string price = std::string(csv.GetData(0U, "buy_price"));
    bool eq = false;
    if (price == "1000") {
        eq = true;
    }

    eq = false;
    std::string equip_type = std::string(csv.GetData(110U, "equip_type"));
    if (equip_type == "7") {
        eq = true;
    }

    std::vector<int> basic_property;
    csv.GetMultiInts(1U, "base_property", &basic_property);
    EXPECT_TRUE(basic_property.size() == 2);

    EXPECT_EQ(1, basic_property[0]);
    EXPECT_EQ(73, basic_property[1]);
}
