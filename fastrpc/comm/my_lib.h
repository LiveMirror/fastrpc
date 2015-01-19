/**
************************************ 
* 	Copyright (c) 2012，Baidu
*	All rights reserved.
************************************
*\n
* 	@file mylib.h
* 	@breif 公用基础函数的定义
*\n
* 	@version 0.0.1
* 	@author clarencelei
* 	@date 2012.3.12
*/
#ifndef _SRC_INCLUDE_MY_LIB_H_
#define _SRC_INCLUDE_MY_LIB_H_

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

namespace MyLib
{
	void InitDaemon();

	
	/*
	*	@brief 字符串拷贝
	*	@param[out] char *strDes 要拷贝的目的字符串
	*	@param[in] const char *strSrc 要拷贝的源字符串
	*	@param[in] DWORD dwLen 目的字符串长度
	*	@return char *返回目的字符串指针;
	*/
	char *SafeStrCopy(char *strDes, const char *strSrc, int nLen);


	// 将数字字符串转换为整数
	int SafeAtoi(const char *str);


	int SafeStrCat(char* buf, const char* s1, const char* s2, const int &nBufLen);

	char* SafeStrUpr(char* str);

	bool StringIsAlnum(char* str);

	char *SafeAton(in_addr addr, char *str);
};

#endif
