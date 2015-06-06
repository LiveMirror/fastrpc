/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: logswitcher.h,v 1.2.22.1 2010/03/25 08:18:08 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file logswitcher.h
 * @author feimat(com@baidu.com)
 * @date 2008/09/12 15:35:42
 * @version $Revision: 1.2.22.1 $ 
 * @brief 
 *  
 **/


#ifndef  __LOGSWITCHER_H_
#define  __LOGSWITCHER_H_

namespace comspace{

//用于ul_log和com_log之间切换的全局变量
//extern int __log_switcher__;
extern int com_get_log_switcher();
extern void com_set_log_switcher(int swt);
enum{
	__LOG_NOT_DEFINED__ = 0,
	__USING_ULLOG__ ,
	__USING_COMLOG__ 
};

};













#endif  //__LOGSWITCHER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
