/////////////////////////////////////////////////////////////////////
//            Data struct abd function about data circle           //
/////////////////////////////////////////////////////////////////////
/**
 * @file
 * @brief 环形字符队列
 */
#ifndef __CIRCLEB_H__
#define __CIRCLEB_H__

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ul_log.h"
#include "ul_func.h"

/* 
** Return code for function below 
*/

#define DEF_ERR_CQFULL		-2
#define DEF_ERR_NOTINIT		-3
#define DEF_ERR_CQEMPTY		-4
#define DEF_ERR_INITAGAIN	-5
#define DEF_ERR_PARTOOLITTLE	-6
#define DEF_ERR_MALLOC		-7
#define DEF_ERR_INVALID_PARAM -8

struct circleB_t 
{
	void *pbyBuf; // store the data of the circle
	int sBufLen;  // length of the circle
	int sHead;    // head of the circle
	int sTail;    // tail of the circle
};


/**
 * 向队列中添加数据
 *  @param[in] pstCB  指向环形队列的指针
 *  @param[in] pbyPut 待添加的数据
 *  @param[in] nSize  待添加数据的字节数
 *  @param[out] 无
 *  @return 函数操作结果
 *  - 0 表示成功
 *  - DEF_ERR_CQFULL  表示失败
 */ 
extern int nCBput( struct circleB_t *pstCB, void *pbyPut, int nSize );

/**
 * 从队列中取数据
 *  @param[in] pstCB  指向环形队列的指针
 *  @param[in] pbyPut 待添加的数据
 *  @param[in] nSize  待添加数据的字节数
 *  @param[out] pbyGet 接收到的数据
 *  @return 函数操作结果
 *  - 0 表示成功
 *  - DEF_ERR_CQEMPTY  表示失败 
 */ 
extern int nCBget( struct circleB_t *pstCB, void *pbyGet, int nSize );

/**
 * 初始化环形字符队列
 *
 *  @param[in] npmBufSize 环形队列的空间大小
 *  @param[out] 无
 *  @return 指向环形队列的指针
 *  - NULL 表示失败
 *  - 非NULL 表示初始化成功 
 *  @note 初始化函数执行后，结束时必须使用 ::vCBfree 函数释放 
 */ 
extern struct circleB_t *pstCBinit( int npmBufSize );

/**
 * 释放环形字符队列
 *
 *  @param[in] pstCB 指向环形队列的指针
 *  @param[out] 无
 *  @return 无
 *  @note 初始化函数::pstCBinit执行后，结束时使用此函数
 */ 
extern void vCBfree( struct circleB_t *pstCB );

 
/**
 * 从队列中取数据，数据不从队列中移除
 *
 *  @param[in] pstCB  指向环形队列的指针
 *  @param[in] pbyGet 接收数据的缓冲区指针
 *  @param[in] nSize  接收数据缓冲区的大小
 *  @param[out] pbyGet 接收到的数据
 *  @return 函数操作结果
 *  - 0 表示成功
 *  - DEF_ERR_CQEMPTY  表示失败 
 */  
extern int nCBlookup( struct circleB_t *pstCB, void *pbyGet, int nSize );

 
/**
 * 取环形队列中数据的长度
 *
 *  @param[in] pstCB 指向环形队列的指针
 *  @param[out] 无
 *  @return 数据长度
 */
extern int nCBgetdatalen( struct circleB_t *pstCB );

 
/**
 * 取环形队列中剩余空间的大小
 *
 *  @param[in] pstCB 指向环形队列的指针
 *  @param[out] 无
 *  @return 空间大小
 */
extern int nCBgetbufreelen( struct circleB_t *pstCB );

#endif
