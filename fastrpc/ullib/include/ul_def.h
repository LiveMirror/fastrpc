
/***************************************************************************
 * 
 * Copyright (c) 2007 feimat.com, Inc. All Rights Reserved
 * $Id: ul_def.h,v 1.4 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_def.h
 * @author baonh(baonh@feimat.com)
 * @version $Revision: 1.4 $ 
 * @brief ullib库公共定义 
 *  
 **/

#ifndef __UL_DEF_H__
#define __UL_DEF_H__

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/*==== for type def */
#define Uchar(inp) 	(unsigned char)(inp)
#define Ushort(inp)	(unsigned short)(inp)
#define Uint(inp)	(unsigned int)(inp)

typedef unsigned short	u_int16;
typedef unsigned int	u_int32;


#define UL_OFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)		  /**< 取结构体变量的偏移    */
#define UL_SAFEFREE(opt) do {free(opt); opt = NULL;} while(0);			  /**< free后变量置NULL      */
#define UL_SAFECLOSE(fp) do {close(fp); fp = -1;} while(0);				  /**< close后变量置-1       */
#define UL_SAFEDELETE(opt) do {delete opt; opt = NULL; }while(0);		  /**< delete  后置NULL		 */
#define UL_FLOATCMP(a,b) ((fabs((double)(a)-(double)(b))<1e-6)?0:(((a)<(b))?-1:1))		  /**< 浮点比较       */
#define UL_ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))				  /**< 取定义的数组长度          */

#define UL_RETURN_VAL_IF(exp, val) do {if (exp) {return val;}}while(0);		  /**< 满足exp条件直接return val  */
#define UL_RETURN_VAL_IF_FAIL(exp, val) do {if (!(exp)) {return val;}}while(0);  /**< 不满足exp条件直接return val*/



/*==== for logic */
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/* for shm */
#define SHM_MIN		1024

/*==== for math */
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Exchange(a, b, mid) (mid)=(a); (a)=(b); (b)=(mid)

/*==== for string */
#define CHAR_NUM		256
#define PATH_SIZE		256
#define LINE_SIZE		4096
#define WORD_SIZE		2048
#define MIN_WORD_SIZE	16

#define SEND			(char)0 	/**< '\0' */
#define TAB				(char)9		/**< tab  */
#define CR				(char)10	/**< '\n' */
#define LF				(char)13	/**< '\r' */
#define SPACE			(char)32	/**< ' '  */

/*==== for chinese */

#define CHN_CHAR_NUM	6768

#define SYM_HIMIN		0xA1	/**< GB symbol : minmum value of first byte */
#define SYM_HIMAX		0xF7	/**< GB symbol : maxmum value of first byte */
#define SYM_LOMIN		0xA1	/**< GB symbol : minmum value of second byte */
#define SYM_LOMAX		0xFE	/**< GB symbol : maxmum value of second byte */

#define CHAR_HIMIN		0xB0	/**< GB char   : min - 1st byte */
#define CHAR_HIMAX		0xF7	/**< GB char   : max - 1st byte */
#define CHAR_LOMIN		0xA1	/**< GB char   : min - 2nd byte */
#define CHAR_LOMAX		0xFE	/**< GB char   : max - 2nd byte */

#define CHAR_HINUM      0x48	/**< (0xF7-0xB0+1) */
#define CHAR_LONUM      0x5E	/**< (0xFE-0xA1+1) */

#define MARK_HIMIN		0xA1	/**< GB mark   : min - 1st byte */
#define MARK_HIMAX		0xAF	/**< GB mark   : max - 1st byte */
#define MARK_LOMIN		0xA1	/**< GB mark   : min - 2nd byte */
#define MARK_LOMAX		0xFE	/**< GB mark   : max - 2nd byte */

#define SMARK_HIMIN     0xA3    /**< simple GB mark   : min - 1st byte */
#define SMARK_HIMAX     0xA3    /**< simple GB mark   : max - 1st byte */
#define SMARK_LOMIN     0xA1    /**< simple GB mark   : min - 2nd byte */
#define SMARK_LOMAX     0xFE    /**< simple GB mark   : max - 2nd byte */

#define DIGIT_HIMIN     0xa3    /**< GB 0---9 */
#define DIGIT_HIMAX     0xa3
#define DIGIT_LOMIN     0xb0
#define DIGIT_LOMAX     0xb9
 
#define HI_HIMIN        0xa3    /**< GB A---Z */   
#define HI_HIMAX        0xa3
#define HI_LOMIN        0xc1
#define HI_LOMAX        0xda
                        
#define LOW_HIMIN       0xa3    /**< GB a---z */ 
#define LOW_HIMAX       0xa3
#define LOW_LOMIN       0xe1
#define LOW_LOMAX       0xfa

#define GBK_HIMIN       0x81
#define GBK_HIMAX       0xfe
#define GBK_LOMIN       0x40
#define GBK_LOMAX       0xfe

/* define the range of gb */
#define IN_RANGE(ch, min, max) ( (((unsigned char)(ch))>=(min)) && (((unsigned char)(ch))<=(max)) )

#define IS_GB(cst)	( IN_RANGE((cst)[0], SYM_HIMIN, SYM_HIMAX) && IN_RANGE((cst)[1], SYM_LOMIN, SYM_LOMAX) )

#define IS_GBK(cst)      ( IN_RANGE((cst)[0], GBK_HIMIN, GBK_HIMAX) && IN_RANGE((cst)[1], GBK_LOMIN, GBK_LOMAX) )

#define IS_GB_CODE(cst)	( IN_RANGE((cst)[0], CHAR_HIMIN, CHAR_HIMAX) && IN_RANGE((cst)[1], CHAR_LOMIN, CHAR_LOMAX) )

#define IS_GB_MARK(cst)	( IN_RANGE((cst)[0], MARK_HIMIN, MARK_HIMAX) && IN_RANGE((cst)[1], MARK_LOMIN, MARK_LOMAX) )

#define IS_GB_SMAEK(cst) ( IN_RANGE((cst)[0], SMARK_HIMIN, SMARK_HIMAX) && IN_RANGE((cst)[1], SMARK_LOMIN, SMARK_LOMAX) )

#define IS_GB_SPACE(cst) ( (((unsigned char)((cst)[0]))==SYM_HIMIN) && (((unsigned char)((cst)[1]))==SYM_LOMIN) )

#define IS_GB_NOBREAK(cst) ( (unsigned char)(cst)[0]==0xa3 && ((unsigned char)(cst)[1]==0xa6 ||(unsigned char)(cst)[1]==0xad ||(unsigned char)(cst)[1]==0xdf) )

#define IS_GB_1234(cst) (IN_RANGE((cst)[0],DIGIT_HIMIN,DIGIT_HIMAX) && IN_RANGE((cst)[1],DIGIT_LOMIN,DIGIT_LOMAX) )

#define IS_GB_ABCD(cst) ( (IN_RANGE((cst)[0],HI_HIMIN,HI_HIMAX) && IN_RANGE((cst)[1],HI_LOMIN,HI_LOMAX) ) || (IN_RANGE((cst)[0],LOW_HIMIN,LOW_HIMAX) && IN_RANGE((cst)[1],LOW_LOMIN,LOW_LOMAX) ) ) 

//Other gb characters can not displayed properly:
//A9: F5--F6
//A8: EA--F6
//    96--A0
//A7: F2--F6
//    C2--D0
//A6: B9--C0
//    D9--DF
//    F6--FE
//A4: F3--F7
//A2: FE
//A2: FD--FE
#define IS_GB_UNDEFINED(cst) ( (IN_RANGE((cst)[0], 0xaa, 0xaf) && IN_RANGE((cst)[1], 0xa1, 0xfe) ) || (IN_RANGE((cst)[0], 0xf8, 0xfe) && IN_RANGE((cst)[1], 0xa1, 0xfe) ) || (IN_RANGE((cst)[0], 0xa1, 0xa7) && IN_RANGE((cst)[1], 0x40, 0xa0) ) || ( IN_RANGE((cst)[0], 0xa4, 0xa9) && IN_RANGE((cst)[1], 0xf7, 0xfe)) ) 


#endif // __UL_DEF_H
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
