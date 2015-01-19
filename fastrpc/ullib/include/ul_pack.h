/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: ul_pack.h,v 1.3 2009/09/18 11:02:57 sun_xiao Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_pack.h
 * @author baonh(baonh@baidu.com)
 * @version $Revision: 1.3 $ 
 * @brief pack包读写
 *  
 **/

#ifndef __PACK_UL_H__
#define __PACK_UL_H__

#include <errno.h>
#include <unistd.h>

#include "ul_log.h"
#include "ul_net.h"
#include "ul_func.h"

#include "dep/dep_pack.h"

#define MAX_PACK_ITEM_NUM  200			/**< 可以插入的name:value对最大数量       */
#define MAX_ITEM_SIZE  2048				/**< name和value的最大长度       */
#define SIZE_NUM  20
#define MAX_HEAD_SIZE  26000				/**< 传输头最大长度 */

typedef struct ul_package {
    char name[MAX_PACK_ITEM_NUM][MAX_ITEM_SIZE];
    char value[MAX_PACK_ITEM_NUM][MAX_ITEM_SIZE];
    int pairnum;      /**< name:value 对的个数，不超过MAX_PACK_ITEM_NUM       */
    int bufsize;      /**< 传输的buff长度       */
    char *buf;        /**< 传输的buff，使用时需要外部分配后传入函数       */
} ul_package_t;

//typedef struct ul_package_t ul_package_t;

/**
 *  @brief 分配pack空间
 *
 *  
 *  @note 没有分配 char *buf用的空间, 使用时用 @ref ul_pack_putbuf 添加缓冲区
 *  @see ul_pack_free ul_pack_putbuf
 *  @return 指向pack空间的指针
 *  - NULL   表示失败
 *  - 非NULL 表示成功
 */
ul_package_t *ul_pack_init();

/**
 *  @brief 释放pack空间
 * 
 *
 *  @param[in]  ppack :  指向pack空间
 *  @note buff中的空间需要用户自己释放
 *  @return 指向pack空间的指针
 *  - 0   表示成功
 */
int ul_pack_free(ul_package_t *ppack);

/**
 *  @brief 向pack包中添加缓冲区
 *  
 *  @param[in]  ppack :  指向pack空间
 *  @param[in]  buffer :  待添加的缓冲区
 *  @param[in]  size :  缓冲区的大小
 *  @return 函数操作结果
 *  - 0   表示成功
 *  - <0  表示失败
 */
int ul_pack_putbuf(ul_package_t *ppack, char *buffer, int size);

/**
 *  @brief 从pack包中获取缓冲区地址
 *  
 *  @param[in]  ppack :  指向pack空间
 *  @return 指向缓冲区的指针
 *  - 非NULL   表示成功
 *  - NULL     表示失败
 */ 
char *ul_pack_getbuf(ul_package_t *ppack);

/**
 *  @brief 向pack包中添加名字和值
 *  
 *  @param[in]  ppack :  指向pack空间
 *  @param[in]  pname : 添加的名字
 *  @param[in]  pvalue : 名字对应的值
 *  @return 函数操作结果
 *  - 1  名字已存在，添加成功
 *  - 0  成功
 *  - -1 名字或值超过允许长度，失败
 *  - -2 pack没有空间，失败
 */  
int ul_pack_putvalue(ul_package_t *ppack, const char *pname, const char *pvalue);

/**
 *  @brief 从pack中取指定名字的值
 *  
 *  @param[in]  ppack :  指向pack空间
 *  @param[in]  pname  : 添加的名字
 *  @param[out]  pvalue : 名字对应的值
 *  @param[in]   size : pvalue的长度限制,包括'\\0'
 *  @return 函数操作结果
 *  @see ul_pack_putvalue
 *  - 1  成功
 *  - 0  失败
 */ 
int ul_pack_getvalue_ex(ul_package_t *ppack, const char *pname, char *pvalue, size_t size);

/**
 * @brief 删除指定名字的值
 *
 * @param [in] ppack   : ul_package_t*
 * @param [in] pname   : const char* delete key name
 * @return  int 
 * -1 : 输入错误 或者 key不存在
 * 0 : 成功
 * @retval   
 * @see 
 * @note 
 * @author sun_xiao
 * @date 2009/08/26 15:33:16
**/
int ul_pack_delvalue(ul_package_t *ppack, const char *pname);

/**
 *  @brief 从指定的句柄读取pack包，支持毫秒级超时
 *  
 *  @param[in]  sockfd   :   连接句柄
 *  @param[in]  ppack    :   指向pack空间
 *  @param[in]  msec     :   超时时间
 *  @param[out] ppack    :   读取的包数据
 *  @return 函数操作结果
 *  - >=0  pack包体的大小
 *  - <0   失败
 *
 */
int ul_pack_reado_ms(int sockfd, ul_package_t *ppack, int msec);

/**
 *  @brief 写向socket写pack,支持毫秒级超时
 *  
 *  @param[in]  sockfd   :   连接句柄
 *  @param[in]  ppack    :   待写的pack包
 *  @param[in]  msec     :   超时时间
 *  @return 函数操作结果
 *  - 0   成功
 *  - -1  失败
 *  - -2  句柄出错
 *  @note 注意写时传得buff长度是在@ref ul_pack_putbuf指定的size, 即整个buff的最大长度
 *  
 */
int ul_pack_writeo_ms(int sockfd, const ul_package_t *ppack, int msec);


/**
 *  写pack包，先将pack包输出到临时缓冲区，再将缓冲区的内容通过连接句柄写出,支持毫秒级超时
 *  
 *  @param[in]  sockfd :   连接句柄
 *  @param[in]  ppack  :   待写的pack包
 *  @param[in]  msec   :   超时时间
 *  @return 函数操作结果
 *  - 0    成功
 *  - -1   失败
 */ 
int ul_pack_write_mo_ms(int sockfd, ul_package_t *ppack, int msec);

/**
 *  序列化pack包，将pack包输出到内存buffer
 *  
 *  @param[in]  buf :   内存buffer
 *  @param[in]  size :  buffer的长度
 *  @param[in]  ppack  :   待写的pack包
 *  @return 函数操作结果
 *  - >=0    实际使用的buffer长度
 *  - -1   失败
 */ 
int ul_pack_serialization(void *buf, int size, ul_package_t *ppack);

/**
 *  反序列化内存buffer，将内存数据解析成pack包
 *  
 *  @param[in]  buf :   内存buffer
 *  @param[in]  size :  buffer的长度
 *  @param[in]  ppack  :   待写的pack包
 *  @return 函数操作结果
 *  - >=0    实际使用的buffer长度
 *  - -1   失败
 */ 
int ul_pack_deserialization(void *buf, int size, ul_package_t *ppack);

/**
 *  @brief 清空pack包的头，即pack包中名字和值
 *  
 *  @param[in]  ppack   :  指向pack空间
 *  @return 函数操作结果
 *  - 1    成功
 *  - -1   失败
 */
int ul_pack_cleanhead(ul_package_t* ppack);

 
/**
 *  @brief 清空pack包中的所有数据(buff 使用的指针没有被free, 请注意在外部释放空间)
 *  
 *  @param[in]  ppack   :  指向pack空间
 *  @return 函数操作结果
 *  - 1    成功
 *  - -1   失败
 */ 
int ul_pack_cleanall(ul_package_t* ppack);


#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
