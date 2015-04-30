/***************************************************************************
 * 
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 * $Id: xdebug.h,v 1.4 2009/09/18 11:02:57 sun_xiao Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file xdebug.h
 * @author feimat(feimat@feimat.com)
 * @date 2008/03/10 15:44:19
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __XDEBUG_H_
#define  __XDEBUG_H_


#ifdef __XDEBUG__
#define _com_debug(fmt, arg...) \
{ \
	fprintf(stderr, "[----------debug--------][%s:%d]"fmt"\n", __FILE__, __LINE__, ##arg); \
}
#define _com_bin_debug(buf, size) \
{ \
	fwrite(buf, 1, size, stderr); \
}

#define _debug_time_def timeval s, e;
#define _debug_getstart gettimeofday(&s, NULL)
#define _debug_getend gettimeofday(&e, NULL)
#define _debug_time ((int)(((e.tv_sec - s.tv_sec) * 1000 + (e.tv_usec - s.tv_usec) / 1000)))

#else
#define _com_debug(fmt, arg...) {}
#define _com_bin_debug(buf, size) {}

#define _debug_time_def {}
#define _debug_getstart {}
#define _debug_getend {}
#define _debug_time 0

#endif

#define _com_error(fmt, arg...) \
{ \
	fprintf(stderr, "[%ld][%ld][%s:%d]"fmt"\n", (long)getpid(), (long)pthread_self(), __FILE__, __LINE__, ##arg); \
    fflush(stderr);\
}

#ifdef __YDEBUG__
#define _core_debug(fmt, arg...) \
{ \
	fprintf(stderr, "[----------core--------][%s:%d]"fmt"\n", __FILE__, __LINE__, ##arg); \
}
#else

//#define _core_debug(fmt, arg...) {}
#define _core_debug _com_debug

#endif



#endif  //__XDEBUG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
