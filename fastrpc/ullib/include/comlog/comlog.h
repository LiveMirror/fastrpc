/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: comlog.h,v 1.5.6.2 2010/04/05 13:30:46 zhang_rui Exp $ 
 * 
 **************************************************************************/
 
/**
 * @file comlog.h
 * @author feimat(feimat@baidu.com)
 * @date 2008/01/30 17:21:07
 * @version $Revision: 1.5.6.2 $ 
 * @brief 
 *  
 **/


#ifndef  __COMLOG_H_
#define  __COMLOG_H_

#include "ul_def.h"
#include "ul_conf.h"

#define COM_USRLEVELSTART  			32		//ÓÃ»§µÈ¼¶idµÄÆäÊµÎ»ÖÃ
#define COM_LEVELNUM  				64		//×î´óÖ§³ÖµÄÈÕÖ¾µÈ¼¶
#define COM_SELFLEVELNUM 			32		//×î´óÖ§³ÖµÄ×Ô¶¨ÒåÈÕÖ¾µÈ¼¶ÊıÄ¿
#define COM_MAXLEVELNAMESIZE 		1024		//×Ô¶¨ÒåÈÕÖ¾µÈ¼¶ÃûµÄ×î´ó³¤¶È
#define COM_MAXDEVICENUM			1024		//×î´óÖ§³ÖµÄappenderÊı
#define COM_MAXCATEGORYNAMESIZE		64		//categoryÃû×ÖµÄ×î´ó³¤¶È
#define COM_MAXHOSTNAME				1024	//×î´óÖ÷»úÃû:port/path
#define COM_MAXFILENAME				256		//×î´óÎÄ¼şÃû
#define COM_MAXDUPSERVER			32		//·şÎñÆ÷ÈßÓàÌ¨Êı
#define COM_MAXSERVERSUPPORT 		32		//×î´óÖ§³ÖÁ¬½ÓµÄserverÊıÄ¿
#define COM_MODULENAMESIZE 			64		//Ä£¿éÃûµÄ×î³¤³¤¶È
#define COM_MAXLOGFILE				2018	//Ã¿¸öÈÕÖ¾ÎÄ¼şÖ§³ÖµÄ×î´ó³¤¶È£¬µ¥Î»m
#define COM_MAXCUTTIME				10000000 //×î´óÈÕÖ¾ÇĞ¸îÊ±¼ä£¬µ¥Î»·ÖÓ
#define COM_MAXAPPENDERNAME			16		//AppenderÃû×ÖµÄ×î´ó³¤¶È
#define COM_RESERVEDSIZE			32		//com_device_tÖĞ±£Áô×Ö¶ÎµÄ³¤¶È
#define COM_RESERVEDNUM             64      //com_device_tÖĞ±£ÁôÀ©Õ¹×Ö¶ÎµÄ¸öÊı

//±ê×¼ÅäÖÃÃû³Æ
#define COMLOG_LEVEL				"COMLOG_LEVEL"	//ÈÕÖ¾µÈ¼¶
#define COMLOG_SELFDEFINE			"COMLOG_SELFDEFINE"	//½ÓÊÕµÄ×Ô¶¨ÒåÈÕÖ¾
#define COMLOG_DEVICE_NUM			"COMLOG_DEVICE_NUM"	//´æ´¢ÈÕÖ¾µÄÉè±¸Êı
#define COMLOG_DEVICE				"COMLOG_DEVICE"	//Ç°×º
#define COMLOG_PROCNAME				"COMLOG_PROCNAME"	//¸ÃÄ£¿éµÄÃû×Ö
#define COMLOG_LOGLENGTH			"COMLOG_LOGLENGTH" //ÈÕÖ¾×î´ó³¤¶È
#define COMLOG_TIME_FORMAT			"COMLOG_TIME_FORMAT" //ÈÕÖ¾ÖĞÊ±¼ä²¿·ÖµÄ×Ô¶¨Òå¸ñÊ½
#define COMLOG_DFS                  "COMLOG_DFS"        //Îª·Ö²¼Ê½Æ½Ì¨×öµÄÀ©Õ¹
#if 0
//Éè±¸ÀàĞÍ±êÖ¾·ûºÅ
#define COMLOG_DEVICE_FILE			"FILE"					//ÎÄ¼şÊä³ö
#define COMLOG_DEVICE_NET			"NET"					//ÍøÂçÊä³ö
#define COMLOG_DEVICE_TTY			"TTY"					//ÖÕ¶ËÊä³ö
#define COMLOG_DEVICE_ULLOG			"ULLOG"				//ÓëullogÎÄ¼ş²¼ÊğÒ»ÖÂ
#define COMLOG_DEVICE_ULNET			"ULNET"				//ÓëullogÎÄ¼ş²¼ÊğÒ»ÖÂµÄÍøÂçÈÕÖ¾
#define COMLOG_DEVICE_AFILE			"AFILE"				//Òì²½£¨·Ç×èÈû£©µÄÎÄ¼şÊä³ö
#endif
#define COMLOG_DEVICE_PATH			"PATH"					//Éè±¸Â·¾¶
#define COMLOG_DEVICE_NAME 			"NAME"					//ÈÕÖ¾ÎÄ¼şÃû
#define COMLOG_DEVICE_AUTH			"AUTH"					//ºÏ²¢ÊÚÈ¨
#define COMLOG_DEVICE_TYPE			"TYPE"					//Éè±¸ÀàĞÍ
#define COMLOG_DEVICE_OPEN			"OPEN"					//ÊÇ·ñÆôÓÃÕâ¸öÉè±¸
#define COMLOG_DEVICE_SIZE			"SIZE"					//ÈÕÖ¾´óĞ¡
#define COMLOG_DEVICE_SPLITE_TYPE	"SPLITE_TYPE"			//·Ö¸îÀàĞÍ
#define COMLOG_DEVICE_DATECUTTIME	"DATA_CUTTIME"			//ÇĞ¸îÖÜÆÚ ·ÖÖÓÎªµ¥Î»£¬´óÓÚ0 Ä¬ÈÏ60
#define COMLOG_DEVICE_COMPRESS		"COMPRESS"				//ÊÇ·ñÖ§³ÖÑ¹Ëõ
#define COMLOG_DEVICE_SYSLEVEL		"SYSLEVEL"				//Ö§³ÖµÄÏµÍ³ÈÕÖ¾µÈ¼¶
#define COMLOG_DEVICE_SELFLEVEL		"SELFLEVEL"				//Ö§³ÖµÄ×Ô¶¨ÒåÈÕÖ¾µÈ¼¶
#define COMLOG_DEVICE_LAYOUT		"LAYOUT"				//Éè±¸Ä£°æ
#define COMLOG_DEVICE_LAYOUT_NDC	"LAYOUT_NDC"			//ÉèÖÃndcÄ£°æ
#define COMLOG_DEVICE_RESERVED1		"RESERVED1"				//±£Áô×Ö¶Î
#define COMLOG_DEVICE_RESERVED2		"RESERVED2"
#define COMLOG_DEVICE_RESERVED3		"RESERVED3"
#define COMLOG_DEVICE_RESERVED4		"RESERVED4"
//////#ifndef u_int64
////#define u_int64 unsigned long long
//#endif
#if 0
enum {
	COM_DEVICESTART = 0,
	COM_FILEDEVICE = 1,	//ÎÄ¼şÉè±¸
	COM_NETDEVICE = 2,	//ÍøÂçÉè±¸
	COM_TTYDEVICE = 3,	//ÖÕ¶ËÏÔÊ¾
	COM_ULLOGDEVICE = 4,	//Óëul_logÎÄ¼ş²¼Êğ·ç¸ñÒ»ÖÂ
	COM_ULNETDEVICE = 5,	//ÓëullogÎÄ¼ş²¼ÊğÒ»ÖÂµÄÍøÂçÈÕÖ¾
	COM_AFILEDEVICE = 6,
	COM_DEVICENUM		//Ö§³ÖµÄÉè±¸Êı
};
#endif
//ÅäÖÃSPLITE_TYPEÊ±£¬ÅäÖÃÏîµÄÖµ¿ÉÒÔÊÇÏÂÃæµÄ×Ö·û´®£¬Ò²¿ÉÒÔÊ¹0£¬1£¬2µÄÊı×Ö
#define COMLOG_DEVICE_TRUNCT 		"TRUNCT"	
#define COMLOG_DEVICE_SIZECUT 		"SIZECUT"
#define COMLOG_DEVICE_DATECUT		"DATECUT"
enum {
	COM_TRUNCT = 0,		//Ö±½ÓÇå¿ÕÈÕÖ¾´Ó´ò(0)
	COM_SIZECUT,	//°´´óĞ¡·Ö¸îÈÕÖ¾(1)
	COM_DATECUT,	//ÈÕÆÚÇĞ¸î(2)
};

enum {
	LOG_NOT_DEFINED = 0, //Î´ÏÔÊ½µ÷ÓÃÈÎºÎopenlog/loadlog£¬Ä¬ÈÏullog
	USING_COMLOG,//Ê¹ÓÃcomlog
	USING_ULLOG,//Ê¹ÓÃullog
};

#define COMLOG_FATAL 1
#define COMLOG_WARNING 2
#define COMLOG_NOTICE 4
#define COMLOG_TRACE 8
#define COMLOG_DEBUG 16

#define UL_LOGLINE(x) UL_LOGLINE_(x)
#define UL_LOGLINE_(x) #x

#define CFATAL_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_FATAL, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)

#define CWARNING_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_WARNING, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)

#define CNOTICE_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_NOTICE, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)

#ifdef CLOSE_TRACE_LOG
#define CTRACE_LOG(fmt, arg...) ((void *)(0))
#else
#define CTRACE_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_TRACE, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)
#endif

#ifdef CLOSE_DEBUG_LOG
#define CDEBUG_LOG(fmt, arg...) ((void *)(0))
#else
#define CDEBUG_LOG(fmt, arg...) \
do { \
	com_writelog(COMLOG_DEBUG, "["__FILE__":"UL_LOGLINE(__LINE__)"]"fmt, ##arg); \
} while (0)
#endif

namespace comspace {
	class Layout;
	class Category;
	class Event;
	class Appender;
};

//Ìí¼ÓÖ§³ÖµÄÈÕÖ¾µÈ¼¶
#define COMLOG_ADDMASK(dev, loglvl) \
{ \
	(dev).log_mask |= ((unsigned long long)(1)<<loglvl);\
}
//É¾³ıÖ§³ÖµÄÈÕÖ¾µÈ¼¶
#define COMLOG_DELMASK(dev, loglvl) \
{ \
	(dev).log_mask &= ~((unsigned long long)(1)<<loglvl); \
}
//Çå³ıÈÕÖ¾µÄÈÕÖ¾µÈ¼¶
#define COMLOG_CLEARMASK(dev) \
{ \
	(dev).log_mask = 0; \
}
//³õÊ¼»¯½¨ÒéµÄÏµÍ³ÈÕÖ¾µÈ¼¶
#define COMLOG_SETSYSLOG(dev) \
{ \
	COMLOG_ADDMASK(dev, COMLOG_FATAL); \
	COMLOG_ADDMASK(dev, COMLOG_WARNING); \
	COMLOG_ADDMASK(dev, COMLOG_NOTICE); \
	COMLOG_ADDMASK(dev, COMLOG_TRACE); \
	COMLOG_ADDMASK(dev, COMLOG_DEBUG); \
}

//Éè±¸Ãû
struct com_device_t
{
	char host[COM_MAXHOSTNAME];	//Ö÷»úÃû, ip:port/path Èç¹ûÖ÷»úÃûÎª¿Õ±íÊ¾Îª±¾µØÎÄ¼ş
	char file[COM_MAXFILENAME];	//ÈÕÖ¾´òÓ¡ËùÔÚÎÄ¼ş
	char auth[COM_MODULENAMESIZE];	//Èç¹û·Ç¿Õ£¬ÄÇÃ´ÏàÍ¬ÎÄ¼şÃûµÄÈÕÖ¾»áÔÚÍ¬Ò»Ì¨Ö÷»úÉÏºÏ²¢
	char type[COM_MAXAPPENDERNAME]; //Éè±¸ÀàĞÍ£¨appenderÀàĞÍ£©

	char reserved1[COM_RESERVEDSIZE];//±£Áô×Ö¶Î
	char reserved2[COM_RESERVEDSIZE];
	char reserved3[COM_RESERVEDSIZE];
	char reserved4[COM_RESERVEDSIZE];
    int log_size;		//»Ø¹öÈÕÖ¾´óĞ¡ 
	char open;			//ÊÇ·ñÆôÓÃ
	char splite_type;	//TRUNCT±íÊ¾Ö±½Ó»Ø¹ö£¬SIZECUT ±íÊ¾°´´óĞ¡·Ö¸î£¬DATECUT±íÊ¾°´Ê±¼ä·Ö¸î
	char compress;		//ÍøÂçĞ­Òé£¬ÊÇ·ñÑ¹Ëõ 0±íÊ¾²»Ñ¹Ëõ£¬1±íÊ¾Ñ¹Ëõ
	int cuttime;		//ÇĞ¸îÊ±¼äÖÜÆÚ£¬Ä¬ÈÏÎª0£¬ÕûµãÇĞ¸î
	unsigned long long log_mask;		//½ÓÊÕÄÄĞ©µÈ¼¶ÈÕÖ¾£¬Ä¬ÈÏÎª0±íÊ¾½ÓÊÕÏµÍ³µÈ¼¶ÈÕÖ¾,ADD_MASK, DEL_MASK ºêÀ´¿ØÖÆ			
    //ÓÃÓÚÀ©Õ¹µÄÅäÖÃÊı×é£¬¿¼ÂÇµ½4¸öreserved×Ö¶ÎÒÑ¾­±»Ê¹ÓÃ£¬reserved×Ö¶Î²»É¾³ı
    char name[COM_MAXAPPENDERNAME]; //ÅäÖÃÖĞµÄÃû×Ö
    unsigned int reserved_num;
    ul_confitem_t reservedext[COM_RESERVEDNUM];
    ul_confdata_t reservedconf;
	
	comspace::Layout *layout;		//×Ô¶¨ÒåÈÕÖ¾µÄ¸ñÊ½£¬null±íÊ¾²ÉÓÃÄ¬ÈÏ¸ñÊ½
public:
	com_device_t();
};

int set_comdevice_default(com_device_t *dev);
int set_comdevice_selflog(com_device_t *dev, const char *logname);

struct com_logstat_t
{
	//¸ú16±íÊ¾debugµÈ¼¶8±íÊ¾traceµÈ¼¶4±íÊ¾noticeµÈ¼¶£¬2±íÊ¾warningµÈ¼¶£¬1±íÊ¾fatalµÈ¼¶
	int sysevents;			//ÏµÍ³ÈÕÖ¾µÈ¼¶
	//Èç¹û²»ÉèÖÃ£¬userevents[0] = 0; ±íÊ¾´òÓ¡ËùÓĞ×Ô¶¨ÒåÈÕÖ¾
	//ÉèÖÃµÄ»° ×Ô¶¨ÒåµÈ¼¶Ãû,×Ô¶¨ÒåµÈ¼¶Ãû£»ÓÃ¶ººÅ¸ô¿ª£¬±íÊ¾Òª´òÓ¡
	char userevents[1024];	//×Ô¶¨ÒåÈÕÖ¾µÈ¼¶
	//1±íÊ¾ÔÚÖÕ¶Ë´òÓ¡ÈÕÖ¾£¬0±íÊ¾²»ÏÔÊ¾
	int ttydisp;			//ÊÇ·ñÔÚÖÕ¶ËÏÔÊ¾Êı¾İ

	com_logstat_t();
};

/**
 * @brief ¸ù¾İÖ¸¶¨Éè±¸´ò¿ªÈÕÖ¾
 *
 * @param [in] procname ½ø³ÌÃû
 * @param [in] dev   : device_t* ÈÕÖ¾Êä³öµÄÉè±¸¼¯ºÏ
 * @param [in] dev_num   : int	Éè±¸ÊıÄ¿
 * @param [in] stat   : logstat_t*	ÈÕÖ¾µÈ¼¶¹ÜÀí
 * @return  int ³É¹¦·µ»Ø0£¬Ê§°ÜÆäËû
 * @retval   
 * 	ÏµÍ³ÈÕÖ¾»á¸ù¾İµ±Ç°µÈ¼¶ÏòÖ¸¶¨µÄÉè±¸´òÓ¡£¬
 * 	±ÈÈç´´½¨ÁËÒ»¸öÎÄ¼şÈÕÖ¾Éè±¸£¬ÄÇÃ´Ëü»á´´½¨name.logºÍname.log.wfÁ½¸öÎÄ¼ş
 * 	notice, trace, debugÍùname.logÎÄ¼ş´ò£¬fatal, warningÍùname.log.wf´ò
 * @see 
 * @author feimat
 * @date 2008/01/30 18:43:11
**/
int com_openlog(const char *procname, com_device_t *dev, int dev_num, com_logstat_t *stat);//´ò¿ªÈÕÖ¾£¬²¢Ö¸¶¨ÈÕÖ¾µÈ¼¶£¬ºÍÊä³öµÄÈÕÖ¾Éè±¸

/**
 * @brief ¸ù¾İÅäÖÃ×°ÔØÈÕÖ¾£¬ÕâÑùµÄ»°Ö÷Ïß³Ì²»ĞëÒªµ÷ÓÃcom_openlog
 *
 * @param [in/out] path   : const char*
 * @param [in/out] file   : const char*
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/26 12:35:29
**/
int com_loadlog(const char *path, const char *file);

/**
 * @brief support multiprocess in bigpipe
 *
 * @param [in/out] path   : const char*
 * @param [in/out] file   : const char*
 * @param [in/out] id     : process id by app, not pid
 * @return  int 
 * @retval   
 * @see 
 * @author wangshaoyan
 * @date 2012/05/24 16:39:29
**/
int com_loadlog_ex(const char *path, const char *file, int id);

/**
 * @brief ³õÊ¼»¯Ïß³ÌÊı¾İ
 *
 * @return  int  0³É¹¦£¬ÆäËû£­1
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/07 18:21:56
**/
int com_openlog_r();


/**
 * @brief ¹Ø±ÕÏß³ÌÊı¾İ
 *
 * @return  int 0³É¹¦£¬ÆäËû-1
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/07 18:22:15
**/
int com_closelog_r();

/**
 * @brief ¸ù¾İlogÃû·µ»Ølogid
 *
 * @param [in/out] selflog   : char*	logÃû
 * @return  int ·µ»ØµÄlogid > 0£¬·ñÕßÈÕÖ¾²»´æÔÚ 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/15 14:28:57
**/
int com_getlogid(char *selflog);

/**
 * @brief Ôö¼ÓĞÂµÄÉè±¸£¬Èç¹ûÊÇÖ÷Ïß³ÌÌí¼Ó£¬ÊÇ±»È«¾Ö¹²Ïí£¬ÔÚ×ÓÏß³ÌÌí¼Ó£¬ÊÇ±»×ÓÏß³Ì·ÖÏí
 * 	×¢Òâ£¬Ö÷Ïß³ÌÌí¼Óºó²Åcreate×ÓÏß³ÌÓĞĞ§£¬Èç¹ûÊÇÔÚ×ÓÏß³ÌcreateÍêÖ®ºóÌí¼ÓµÄ£¬½öÊÇÖ÷Ïß³Ì¶ÀÓĞµÄ
 * 	Èç¹ûÉè±¸ÒÑ¾­´æÔÚ£¬»á¸ù¾İselflogÄÚÈİÉèÖÃ×Ô¶¨ÒåÊôĞÔ
 * 	Èç¹ûÉè±¸²»´æÔÚ»áĞÂÔö
 * 	Èç¹ûĞèÒªÉèÖÃ¶à¸ö×Ô¶¨ÒåÈÕ×Ó£¬Çë²»¶Ïµ÷ÓÃÕâ¸öº¯Êı
 *
 * @param [in] dev   : com_device_t*
 * @param [in] nums	: Ôö¼ÓµÄÉè±¸Êı 
 * @param [in] selflog : ×Ô¶¨ÒåÈÕÖ¾Ãû×Ö£¬null±íÊ¾Ã»ÓĞ×Ô¶¨ÒåÈÕ×Ó
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/15 14:24:13
**/
int com_setdevices(com_device_t *dev, int nums, char *selflog = NULL);

/**
 * @brief ¹Ø±ÕÈÕÖ¾£¬Ö÷Ïß³Ìµ÷ÓÃ
 *
 * @param [in] waittime µÈ´ıÈÕÖ¾´òÍêµÄ×î´óµÈ´ıÊ±¼ä£¬Èç¹û³¬¹ıÕâ¸öÊ±¼äÈÕÖ¾»¹Ã»·¢ËÍµ½Ö¸¶¨Éè±¸
 * 	Ç¿ÖÆ¹Ø±ÕÈÕÖ¾£¬¶ªÊ§Ã»ÓĞ´òÓ¡³öÈ¥µÄÈÕÖ¾¡£ÉèÖÃ 0 ±íÊ¾²»µÈ´ı
 * 	 ¡¾×¢Òâ¡¿ÕâÀïµÄwaittimeÖ»¶ÔNetAppenderÓĞĞ§£¬Èç¹ûÄãÊ¹ÓÃµÄAsyncFileAppender»òÕßNetcpAppender
 * 	 ÕâÑùµÄÒì²½´òÓ¡µÄ·½Ê½£¨ËûÃÇ¶¼»áÆô¶¯Ò»¸öµ¥¶ÀµÄÏß³Ì¸ºÔğIO²Ù×÷£©£¬
 * 	 ÔòĞèÒªÔÚµ÷ÓÃcom_closelogÖ®Ç°×ÔĞĞµ÷ÓÃsleepº¯ÊıÒÔµÈ´ıÒì²½Ïß³ÌµÄ´òÓ¡²Ù×÷¡£
 * 	 ·ñÔò»áÇ¿ĞĞÍË³ö£¬ÁôÔÚÄÚ´æÖĞµÄÈÕÖ¾»á±»¶ªÆú¡£
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/03/07 18:23:31
**/
int com_closelog(int waittime = 1000);//¹Ø±ÕÈÕÖ¾

/**
 * @brief resetlog after fork(2) in multiprocess
 *
 * @return  int 
 * @retval   
 * @see the appender need implement its own reset, now it public/bigpipe-appender
 * has supported it.
 * @author wangshaoyan
 * @date 2012/06/10 22:58:31
**/
int com_resetlog();



/* ¹Ø±Õ¸ÃÏß³ÌÍùÖ¸¶¨appender_typeµÄÊä³ö,²»Ó°ÏìÆäËüÏß³Ì
* appender_type :  appenderÔÚÅäÖÃÖĞµÄÀàĞÍ 
* ·µ»Ø0 ³É¹¦ £¬ ·Ç0 Ê§°Ü
*/
int com_setappendermask(char *appender_type);

/**
 * @brief ¸ù¾İÈÕÖ¾id´òÓ¡ÈÕÖ¾
 *
 * @param [in] events   : int ÈÕÖ¾µÈ¼¶id
 * @param [in] fmt   : const char* Òª´òÓ¡µÃÈÕÖ¾ĞÅÏ¢
 * @return  int ³É¹¦·µ»Ø0£¬ÆäËûÊ§°Ü
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 11:58:06
**/
int com_writelog(int events, const char *fmt, ...);//¸ù¾İÈÕÖ¾id´òÓ¡ÈÕÖ¾

/**
 * @brief com_writelog_ex
 *
 * @param [in] events : int
 * @param [in] version : int
 * @param [in] type : const char* 
 * @param [in] fmt : const char* 
 * @return  int 
 * @retval   
 * @see 
 * @author wangshaoyan
 * @date 2012/06/18 21:22:06
**/
int com_writelog_ex(int events, int version, const char *type, const char *fmt, ...);

/**
 * @brief ¸ù¾İÈÕÖ¾Ãû´òÓ¡ÈÕÖ¾ĞÅÏ¢
 *
 * @param [in/out] name   : const char* ÈÕÖ¾Ãû£¬·Ç¿Õ£¬±ØĞë×¢²á¹ı
 * @param [in/out] fmt   : const char* ÈÕÖ¾ĞÅÏ¢
 * @return  int ³É¹¦·µ»Ø0£¬Ê§°ÜÆäËû
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 11:58:46
**/
int com_writelog(const char *name, const char *fmt, ...);//¸ù¾İÈÕÖ¾Ãû´òÓ¡ÈÕÖ¾

/**
 * @brief com_writelog_ex
 *
 * @param [in] events : const char *
 * @param [in] version : int
 * @param [in] type : const char * 
 * @param [in] fmt : const char * 
 * @return  int 
 * @retval   
 * @see 
 * @author wangshaoyan
 * @date 2012/06/18 21:24:06
**/
int com_writelog_ex(const char *name, int version, const char *type, const char *fmt, ...);

/*
 * @brief ´òÓ¡key£¬value¶Ó
 * @param [in] key, ¼üÃû 
 * @param [in] fmt, ¸ñÊ½»¯ÄÚÈİ
 * @param [in] arg..., ÄÚÈİÊı¾İ
 * @return ÄÜ¹»ÔÙ´Î´òÓ¡µÃ¶ÔÏóÒıÓÃ
 * @see com_pushlog("key", "value")("key2", "val%d", 2)(COMLOG_NOTICE, "end");
 * 	ÒÔ(ÈÕÖ¾µÈ¼¶, Êı¾İ) ½áÊø´òÓ¡
 **/
#define com_pushlog(key, fmt, arg...) com_getcategory()->push(key, fmt, ##arg)

/**
 * @brief ½«ÅäÖÃÎÄ¼şµÃÂ·¾¶ºÍÎÄ¼şÃû¸æËß¿â£¬¿â»á·ÖÎöÕâ¸öÅäÖÃ¸ñÊ½£¬
 * 	ÔÙÌõ¼ş´¥·¢ÏÂ£¬¸Ä±äÈÕÖ¾µÈ¼¶
 *
 * @param [in/out] path   : const char*
 * @param [in/out] file   : const char*
 * @return  int ³É¹¦·µ»Ø0 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 12:02:48
**/
int com_reglogstat(const char *path, const char *file);

/**
 * @brief µ÷ÓÃÕâ¸öº¯Êı£¬½«¸Ä±äÈÕÖ¾´òÓ¡µÈ¼¶
 *
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 12:03:33
**/
int com_changelogstat();

/**
 * @brief µ÷ÓÃÕâ¸öº¯Êı£¬½«¸ù¾İÉèÖÃµÃlogstatĞÅÏ¢£¬¸Ä±äÈÕÖ¾µÈ¼¶
 *
 * @param [in/out] logstat   : com_logstat_t*
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/04/21 12:03:51
**/
int com_changelogstat(com_logstat_t *logstat);

/**
 * @brief ÅĞ¶Ïµ±Ç°´ò¿ªµÄÊÇÄÄÒ»¸ölog
 *
 * @return  int 
 * 			USING_COMLOG : Ê¹ÓÃcomlog
 * 			USING_ULLOG  : Ê¹ÓÃullog
 * 			LOG_NOT_DEFINED : Î´ÏÔÊ½µ÷ÓÃÈÎºÎopenlog/loadlog
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/12/16 01:36:18
**/
int com_logstatus();

/**
 * @brief ¶¯Ì¬Ìí¼ÓĞÂµÄappender¡£ÓÃÓÚ¶ş´Î¿ª·¢¡£
 * ÓÃ»§¿ÉÒÔ¼Ì³ĞAppenderÀà£¬ÊµÏÖÒ»¸ö×Ô¼ºµÄAppender£¬²¢×¢²áÏàÓ¦µÄget/tryº¯Êı
 * name±íÊ¾×Ô¶¨ÒåappenderµÄÃû³Æ£¬¿ÉÒÔÔÚÅäÖÃÎÄ¼şÖĞ±»Ê¶±ğ
 *
 * @param [in] name   : const char* ×Ô¶¨ÒåappenderµÄÃû³Æ
 * @param [in] getAppender   : func_type getAppenderº¯Êı
 * @param [in] tryAppender   : func_type tryAppenderº¯Êı
 * @return  int 
 * @retval   
 * @see 
 * @author feimat
 * @date 2008/09/10 17:24:14
**/
typedef comspace::Appender*(*func_type)(com_device_t &);
int com_registappender(const char * name, func_type getAppender, func_type tryAppender);

/**
 * @brief log enabled
 *
 * @param [in] event
 * @return  int 
 * 1 - enable
 * 0 - disable
 * -1 - error
 * @author wangshaoyan
 * @date 2012/12/13
**/
int com_log_enabled(int level);
int com_log_enabled(const char *selflevel);

//Ïß³ÌÊı¾İ
//ÕâĞ©Êı¾İÁô¸ø¿ª·¢ÈËÔ±£¬²»ÍÆ¼öÊ¹ÓÃ
struct com_thread_data_t
{
	comspace::Category *category;
	comspace::Event *event;
};
comspace::Category * com_getcategory();
comspace::Event * com_getevent();
com_thread_data_t * com_getthreaddata();

#endif  //__LOG_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */

