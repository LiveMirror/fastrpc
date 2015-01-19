// 2011-06-21
// 
// xcore_test.h
// ”√”⁄∏®÷˙≤‚ ‘


#ifndef _XCORE_TEST_H_
#define _XCORE_TEST_H_

#ifdef _XCORE_NEED_TEST

#include "xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// test class
///////////////////////////////////////////////////////////////////////////////
struct XTestNode
{
	typedef bool (*xcore_test_func)();
	string          m_name;
	xcore_test_func m_func;
};

bool xcore_test_version();
bool xcore_test_atomic();
bool xcore_test_critical();
bool xcore_test_mutex();
bool xcore_test_event();
bool xcore_test_thread();
bool xcore_test_semaphore();
bool xcore_test_file_util();
bool xcore_test_str_util();
bool xcore_test_log();
bool xcore_test_str_parser();
bool xcore_test_thread_array();
bool xcore_test_thread_pool();
bool xcore_test_time();
bool xcore_test_time_span();
bool xcore_test_timer();
bool xcore_test_base64();
bool xcore_test_crc();
bool xcore_test_md5();
bool xcore_test_markup();
bool xcore_test_memory();
bool xcore_test_rwlock();
bool xcore_test_sock_addr();
bool xcore_test_socket();
bool xcore_test_byte_order();
bool xcore_test_clock();
bool xcore_test_utility();
bool xcore_test_conver();
bool xcore_test_json();

}//namespace xcore

using namespace xcore;

#endif//_XCORE_NEED_TEST

#endif//_XCORE_TEST_H_
