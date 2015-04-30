// Copyright (c) 2012, feimat Inc.
// All rights reserved.
//
// Author: jack <jack@qq.com>
// Created: 12/29/12
// Description: string_hash test

#include <fstream>
#include <iostream>
#include <map>
#include "string_hash.h"
#include "gtest.h"

TEST(Shuffle, Simple)
{
    unsigned int disks = 11;
    unsigned int dirs_per_level = 16;
    unsigned int levels = 3;

    unsigned int total_dirs_per_disk = 1;
    for (unsigned int i = 0; i < levels; ++i) {
        total_dirs_per_disk *= dirs_per_level;
    }
    unsigned int buckets = disks * total_dirs_per_disk;

    std::map<unsigned int, unsigned int> files_allocated_container;
    std::map<unsigned int, unsigned int> disk_files;

    for (unsigned int i = 0; i < buckets; ++i) {
        files_allocated_container.insert(
            std::make_pair<unsigned int, unsigned int>(i, 0));
    }

    for (unsigned int j = 0; j < disks; ++j) {
        disk_files.insert(std::make_pair<unsigned int, unsigned int>(j, 0));
    }

    std::string data_file = "/home/pengchong/tars/storemd5.log";

    std::ifstream fin(data_file.c_str());

    std::string one_file_hash;

    std::map<unsigned int, unsigned int>::iterator it;
    std::map<unsigned int, unsigned int>::iterator disk_it;
    while (getline(fin, one_file_hash)) {
        unsigned int place = ipcs_common::ELFhash(one_file_hash.c_str(), buckets);
        it = files_allocated_container.find(place);
        if (it != files_allocated_container.end()) {
            ++it->second;
        }
    }

    unsigned int step = 0;
    unsigned int which_disk = 0;
    unsigned int last_disk = 0;
    unsigned int dirs = 0;
    for (it = files_allocated_container.begin();
         it != files_allocated_container.end();
         ++it) {
        which_disk = step++ / total_dirs_per_disk;
        if (which_disk != last_disk) {
            disk_it = disk_files.find(last_disk);
            std::cout << step << std::endl;
            disk_it->second += dirs;
            last_disk = which_disk;
            dirs = 0;
        } else {
            dirs += it->second;
        }
        std::cout << "bucket " << it->first << " : " << it->second << std::endl;
    }
    disk_it = disk_files.find(disks-1);
    disk_it->second +=dirs;

    for (disk_it = disk_files.begin();
         disk_it != disk_files.end();
         ++disk_it) {
        std::cout << "disk " << (disk_it->first + 1) << " : " << disk_it->second << std::endl;
    }
}
