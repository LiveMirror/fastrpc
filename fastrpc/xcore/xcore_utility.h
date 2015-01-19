// 2011-08-27
// xcore_utility.h
// 
// 实用工具

#ifndef _XCORE_UTILITY_H_
#define _XCORE_UTILITY_H_

#include "xcore_define.h"

namespace xcore {

// errno and describe
uint32  errnum();
string  strerror();

// keyboard hit test
int kbhit();

// computer info
string  computer_name();
string  user_name();

string  os_version();
bool    meminfo(uint32& total_KB, uint32& use_KB, uint32& free_KB);
bool    cpuinfo(vector<string>& infos);
uint32  processnum();

}//namespace xcore

using namespace xcore;

#endif//_XCORE_UTILITY_H_
