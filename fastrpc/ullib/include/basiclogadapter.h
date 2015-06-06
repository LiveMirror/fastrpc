/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * $Id: basiclogadapter.h,v 1.4 2009/09/18 11:02:57 sun_xiao Exp $ 
 * 
 **************************************************************************/



/**
 * @file basiclogadapter.h
 * @author feimat(com@baidu.com)
 * @date 2009/03/05 16:46:36
 * @version $Revision: 1.4 $ 
 * @brief 
 *  
 **/


#ifndef  __BASICLOGADAPTER_H_
#define  __BASICLOGADAPTER_H_

#include "com_log.h"

//ÎªmaskÌí¼ÓÒ»¸öÈÕÖ¾µÈ¼¶,maskÓ¦¸ÃÊÇÒ»¸öuint32_t
#define COMLOG_MASK(mask, level) do{\
	(mask) |= (unsigned int)(1) << (level);\
}while(0)

//ÎªmaskÌí¼Ó5¸öÄ¬ÈÏµÄÏµÍ³ÈÕÖ¾µÈ¼¶,maskÊÇÒ»¸öuint32_t
#define COMLOG_DEFAULT_MASK(mask) do{\
	COMLOG_MASK(mask, COMLOG_FATAL);\
	COMLOG_MASK(mask, COMLOG_WARNING);\
	COMLOG_MASK(mask, COMLOG_NOTICE);\
	COMLOG_MASK(mask, COMLOG_TRACE);\
	COMLOG_MASK(mask, COMLOG_DEBUG);\
}while(0)

/**
 * @brief ÈÕÖ¾½Ó¿Ú£¬ÏòÏÂµ÷ÓÃcomlog»òÕßullog´òÓ¡ÈÕÖ¾
 *        BasicLogAdapter²»»áÊÍ·ÅÈÎºÎ×ÊÔ´£¨°üÀ¨×Ô¼º´´½¨µÄ×ÊÔ´£©
 *        Ò»°ãÀ´Ëµ£¬BasicLogAdapterµÄÉúÃüÖÜÆÚÊÇÕû¸ö½ø³Ì
 *        ÈçÒªÊÍ·Å£¬Ö»ÄÜÊ¹ÓÃcom_closelog()
 *
 * 		  ³ÌĞòÄ¬ÈÏÊ¹ÓÃullog´òÓ¡ÈÕÖ¾
 *        µ±µ÷ÓÃsetCategory, setDevice, setLayoutµÈº¯Êı£¬
 *        »á×Ô¶¯ÇĞ»»µ½comlogµÄ¹æÔò
 *
 *        setLevelÊÇ×Ü¿Ø¿ª¹Ø£¬¶Ôcomlog/ullog¾ùÉúĞ§¡££¨×Ô¶¨ÒåÈÕÖ¾²»ÊÜÔ¼Êø£©
 *
 *        Èç¹ûÒª´òÓ¡×Ô¶¨ÒåµÈ¼¶µÄÈÕÖ¾£¬±ØĞëÏÔÊ½µ÷ÓÃsetLogStat£¬Ê¾Àı£º
 *        BasicLogAdapter *p = new BasicLogAdapter();
 *        p->setDevice(xxx);//×¢²áDevice
 *        comspace::LogStat lstat = comspace::LogStat::createLogStat();//ĞèÒªÔÚsetDeviceÖ®ºóµ÷ÓÃ
 *        //ÒòÎªcreateLogStat±ØĞëÔÚcomlog±»³õÊ¼»¯Ö®ºó²Å¿ÉÒÔÊ¹ÓÃ£¬ËùÒÔ±ØĞëÏÈÖ´ĞĞsetDevice£¬ÀïÃæ»á×ö³õÊ¼»¯
 *        uint32_t level = 0;
 *        COMLOG_DEFAULT_MASK(level);
 *        lstat.setLevel(level, "SelfName1, SelfName2"); //Õâ¸öSelfName1Ò²ĞèÒªÔÚÅäÖÃÎÄ¼şÖĞµÄselfdefineÖĞ¶¨Òå
 *        p->setLogStat(lstat);
 *        p->write("SelfName1", "This is a self level log...");
 *        //»òÕß£º
 *        int self = comspace::getLogId("SelfName1");
 *        p->write(self, "This is also a self level log...");
 *        
 */
class BasicLogAdapter{
	protected:
		//Êµ¼ÊµÄÈÕÖ¾´òÓ¡¾ä±ú
		comspace :: Category * _cate;
		//ÏµÍ³ÈÕÖ¾µÈ¼¶
		int _level;
	public:
		BasicLogAdapter();
		/**
		 * @brief Ö±½ÓÉèÖÃCategory¾ä±ú£(comlog)
		 *
		 * @param [in] cate   : comspace::Category*
		 * @return  int 0³É¹¦£¬ÆäÓàÊ§°Ü
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 16:58:57
		**/
		int setCategory(comspace::Category * cate);
		/**
		 * @brief Ìí¼ÓcomlogµÄÉè±¸ 
		 *
		 * @param [in] dev   : com_device_t* Éè±¸
		 * @param [in] num   : int Éè±¸Êı
		 * @return  int -1³ö´í£¬·ñÔò·µ»Ø³É¹¦Ìí¼ÓµÄÉè±¸ÊıÁ¿
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:01:07
		**/
		int setDevice(com_device_t * dev, int num);
		/**
		 * @brief Ê¹ÓÃlogstatÉèÖÃÈÕÖ¾µÈ¼¶¿ØÖÆ£¬Ö»¶ÔcomlogÉúĞ§¡£
		 *
		 * @param [in] logstat   : comspace::LogStat*
		 * @return  int 0³É¹¦£¬ÆäÓàÊ§°Ü
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:04:07
		**/
		int setLogStat(comspace::LogStat * logstat);
		/**
		 * @brief ÉèÖÃÈÕÖ¾´òÓ¡µÄ×Ü¿ØµÈ¼¶£¬²»ÂÛÊÇcomlog´òÓ¡»¹ÊÇullog´òÓ¡£¬¾ùÉúĞ§¡£
		 *
		 * @param [in] level   : int
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:05:03
		**/
		int setLevel(int level);
        /**
         * @brief »ñÈ¡ÈÕÖ¾µÈ¼¶
         *
         * @return  int 
         * @retval   
         * @author baonh
         * @date 2009/07/25 21:59:36
        **/
        int getLevel();
		/**
		 * @brief ´òÓ¡ÈÕÖ¾µÄ±ê×¼½Ó¿Ú
		 *
		 * @param [in] level   : int ÈÕÖ¾µÈ¼¶
		 * @param [in] fmt   : const char* ÈÕÖ¾ÄÚÈİ
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:06:03
		**/
		int write(int level, const char * fmt, ...) __attribute__ ((format (printf,3,4)));
		/**
		 * @brief ´òÓ¡×Ô¶¨Òå¼¶±ğµÄÈÕÖ¾
		 *
		 * @param [in] level   : const char* ÈÕÖ¾µÈ¼¶Ãû
		 * @param [in] fmt   : const char* ÈÕÖ¾ÄÚÈİ
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:06:32
		**/
		int write(const char * level, const char * fmt, ...) __attribute__ ((format (printf,3,4)));
		//----- ÒÔÏÂ½Ó¿ÚÏàµ±ÓÚwriteµÄ°ü×°---
		/**
		 * @brief ´òÓ¡fatalÈÕÖ¾
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:08:29
		**/
		int fatal(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief ´òÓ¡warningÈÕÖ¾
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:08:49
		**/
		int warning(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief ´òÓ¡noticeÈÕÖ¾
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:03
		**/
		int notice(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief ´òÓ¡traceÈÕÖ¾
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:14
		**/
		int trace(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
		/**
		 * @brief ´òÓ¡debugÈÕÖ¾
		 *
		 * @param [in] fmt   : const char*
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:26
		**/
		int debug(const char * fmt, ...) __attribute__ ((format (printf,2,3)));
	protected:
		/**
		 * @brief ÕæÕıÏòÏÂµ÷ÓÃµÄÈÕÖ¾½Ó¿Ú£¬½ÓÊÕ²ÎÊıÎªÒÑ¾­´ò°üºÃµÄva_list
		 *
		 * @param [in] level   : int
		 * @param [in] fmt   : const char*
		 * @param [in] args   : va_list
		 * @return  int 
		 * @retval   
		 * @see 
		 * @author feimat
		 * @date 2009/03/06 17:09:37
		**/
		int write_args(int level, const char * fmt, va_list args);
		comspace :: Category * tryCategory();
		static int _instance;
		static pthread_mutex_t _lock;
};

















#endif  //__BASICLOGADAPTER_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
