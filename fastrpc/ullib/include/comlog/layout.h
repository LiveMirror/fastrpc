/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: layout.h,v 1.3.14.1 2010/03/25 08:18:08 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file layout.h
 * @author feimat(feimat@baidu.com)
 * @date 2008/01/30 18:54:26
 * @version $Revision: 1.3.14.1 $ 
 * @brief 
 *  
 **/


#ifndef  __LAYOUT_H_
#define  __LAYOUT_H_

namespace comspace
{

class Event;


class Layout
{
	static const int MAXPATTERNSIZE = 1024;
//	static const int FORMATSIZE=128;
//	static char _time_format[FORMATSIZE];
protected:
	char _pattern[MAXPATTERNSIZE];	//模板参数
	char _ndc_pattern[MAXPATTERNSIZE];	//key:value 对的模板
public:
	virtual int format(Event *evt);	//将msg里面的数据根据模式format到render里面	
	int setPattern(const char *pattern, const char *ndc);	//设置显示格式
	int setPattern(const char *pattern);
	int setNdcPattern(const char *ndc);
	/**
	 * @brief 自定义时间格式，在Layout中以%Y打印的时间
	 * 			请使用者确保传入的格式是正确可用的
	 * 			内部调用strftime，不能打印毫秒，要用毫秒需在Layout中使用%U
	 *
	 * 			非线程安全函数，需要在openlog之前被调用
	 *
	 * @param [in] fmt   : const char* 能被strftime识别的时间格式
	 * @return  int 0成功，其余失败
	 * @retval   
	 * @see 
	 * @author feimat
	 * @date 2009/03/26 23:27:40
	**/
	static int setTimeFormat(const char * fmt);
public:
	Layout();

private:
	virtual int fmtpattern(Event *evt, char *buf, int siz);

public:
	virtual ~Layout();
};


//extern Layout g_def_layout;
Layout * comlog_get_def_layout();
void comlog_del_def_layout();

};
#endif  //__LAYOUT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
