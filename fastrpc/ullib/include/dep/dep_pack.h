/***************************************************************************
 * 
 * Copyright (c) 2007 Baidu.com, Inc. All Rights Reserved
 * $Id: dep_pack.h,v 1.2 2008/08/13 02:28:55 baonh Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file ul_pack.h
 * @author baonh(baonh@baidu.com)
 * @version $Revision: 1.2 $ 
 * @brief pack包读写
 *  
 **/

#ifndef __DEP_PACK_H__
#define __DEP_PACK_H__



/**
 *  @brief 为兼容老版本中ul_package而保留
 *
 *  @deprecated 按照新规范结构体变量名应以 _t结尾, 新的版本应该使用ul_package
 *
 */
typedef struct ul_package ul_package;

typedef struct ul_package ul_package_t;
/**
 *  @brief 从pack中取指定名字的值, 因获取的数据pvalue没有长度限制,可能造成写越界
 *  
 *  @param[in]  ppack :  指向pack空间
 *  @param[in]  pname  : 添加的名字
 *  @param[out]  pvalue : 名字对应的值
 *  @return 函数操作结果
 *  @see ul_pack_getvalue_ex ul_pack_putvalue
 *  - 1  成功
 *  - 0  失败
 *  @deprecated pvalue没有长度限制,可能造成写越界
 */ 
int ul_pack_getvalue(ul_package_t *ppack, char *pname, char *pvalue);


/**
 *  @brief 从指定的句柄读取pack包, 因不带超时限制,不推荐使用
 *  
 *  @param[in]  sockfd  连接句柄
 *  @param[in]  ppack   指向pack空间
 *  @param[out] ppack   读取的包数据
 *  @return 函数操作结果
 *  - >=0  pack包体的大小
 *  - <0   失败
 *  @deprecated 不带超时限制,不推荐使用
 */
int ul_pack_read(int sockfd, ul_package_t *ppack);

 
/**
 *  @brief 从指定的句柄 读取数据, 因不带超时限制,不推荐使用
 *  
 *  @param[in]  fd   :   连接句柄
 *  @param[in]  ptr1 :   接收缓冲区
 *  @param[in]  nbytes :  要读取数据的字节数
 *  @param[out] ptr1   :  读取的数据
 *  @return 函数操作结果
 *  - >0  实际读取的字节数
 *  - <=0   失败
 *  @deprecated 因不带超时限制,不推荐使用
 */ 
int ul_pack_readall(int fd, void *ptr1, size_t nbytes);

/**
 *  @brief 读取包的结束标志, 因不带超时限制,不推荐使用
 *  
 *  @param[in]  fd   :   连接句柄
 *  @return 函数操作结果
 *  - 0  成功
 *  - -1 失败
 *  @deprecated 因不带超时限制,不推荐使用, 内部函数不应该对外开放
 */ 
int ul_pack_readend(int sockfd);


/**
 *  @brief 写向socket写pack, 因不带超时限制,不推荐使用
 *  
 *  @param[in]  sockfd   :   连接句柄
 *  @param[in]  ppack    :   待写的pack包
 *  @return 函数操作结果
 *  - 0   成功
 *  - -1  失败
 *  @deprecated 因不带超时限制,不推荐使用
 */ 
int ul_pack_write(int sockfd, ul_package_t *ppack);

 
/**
 *  @brief 向指定的句柄写数据(Inner function), 因不带超时限制,不推荐使用
 *  
 *  @param[in]  sockfd  :  连接句柄
 *  @param[in]  buf     :  待写数据
 *  @param[in]  n       :  数据的字节数
 *  @return 函数操作结果
 *  - >0    实际写出的字节数
 *  - <＝0  失败
 *  @deprecated 因不带超时限制,不推荐使用
 */
int ul_pack_writeall(int sockfd, char *buf, int n);



/**
 *  @brief 写pack包，先将pack包输出到临时缓冲区，
 *  再将缓冲区的内容通过连接句柄写出, 因不带超时限制,不推荐使用
 *  
 *  @param[in]  sockfd  :  连接句柄
 *  @param[in]  ppack   :  待写的pack包
 *  @return 函数操作结果
 *  - 0    成功
 *  - -1   失败
 *  @deprecated 因不带超时限制,不推荐使用
 */ 
int ul_pack_write_m(int sockfd, ul_package_t *ppack);


 
/**
 *  @brief 从指定的句柄读取数据, 因不带超时限制,不推荐使用
 *  
 *  @param[in]  sockfd    :  连接句柄
 *  @param[in]  ppack     :  指向pack空间
 *  @param[in]  over_time :  超时时间
 *  @param[out] ppack     :  读取的包数据
 *  @return 函数操作结果
 *  - >=0  pack包体的大小
 *  - <0   失败
 *  @deprecated 因不带超时限制,不推荐使用
 */
int ul_pack_reado(int sockfd, ul_package_t *ppack, int over_time);

 
/**
 *  @brief 写向socket写pack,支持秒级超时
 *  
 *  @param[in]  sockfd   :   连接句柄
 *  @param[in]  ppack    :   待写的pack包
 *  @param[in]  sec      :   超时时间
 *  @return 函数操作结果
 *  - 0   成功
 *  - -1  失败
 *  @deprecated 因不带超时限制,不推荐使用
 */
int ul_pack_writeo(int sockfd, ul_package_t *ppack, int sec);


/**
 *  @brief 向指定的句柄写数据,支持秒级超时
 *  
 *  @param[in]  sockfd  :  连接句柄
 *  @param[in]  buf     :  待写数据
 *  @param[in]  n       :  数据的字节数
 *  @param[in]  sec     :  超时时间
 *  @return 函数操作结果
 *  - >0    实际写出的字节数
 *  - <＝0  失败
 *  @deprecated 推荐使用豪秒级超时函数
 */
int ul_pack_writeallo(int sockfd, char *buf, int n, int sec);


/**
 *  写pack包，先将pack包输出到临时缓冲区，再将缓冲区的内容通过连接句柄写出,支持秒级超时
 *  
 *  @param[in]  sockfd  :  连接句柄
 *  @param[in]  ppack   :  待写的pack包
 *  @param[in]  sec     :  超时时间
 *  @return 函数操作结果
 *  - 0    成功
 *  - -1   失败
 *  @deprecated 推荐使用豪秒级超时函数
 */ 
int ul_pack_write_mo(int sockfd, ul_package_t *ppack, int sec);


/**
 *  @brief 从指定的句柄读取数据，支持毫秒级超时
 *  
 *  @param[in]  fd       :  连接句柄
 *  @param[in]  ptr1     :  接收缓冲区
 *  @param[in]  nbytes   :  要读取数据的字节数
 *  @param[in]  msec     :  超时时间
 *  @param[out] ptr1     :  读取的数据
 *  @return 函数操作结果
 *  - >0  实际读取的字节数
 *  - <=0   失败
 *  @deprecated 推荐使用ul_net中的@ref ul_sreado_ms
 */
int ul_pack_readallo_ms(int fd, void *ptr1, size_t nbytes, int msec);

/**
 *  @brief 读取包的结束标志，支持毫秒级超时
 *  
 *  @param[in]  sockfd   :   连接句柄
 *  @param[in]  msec     :   超时时间
 *  @return 函数操作结果
 *  - 0   成功
 *  - -1  失败
 *  @deprecated 这应该作为内部函数，外部不使用
 */ 
int ul_pack_readendo_ms(int sockfd, int msec);


/**
 *  @brief 向指定的句柄写数据,支持毫秒级超时
 *  
 *  @param[in]  sockfd  :  连接句柄
 *  @param[in]  buf     :  待写数据
 *  @param[in]  n       :  数据的字节数
 *  @param[in]  msec    :  超时时间
 *  @return 函数操作结果
 *  - >0    实际写出的字节数
 *  - <＝0  失败
 *  @deprecated 推荐使用ul_net中的 @ref ul_swriteo_ms_ex 即可
 */
int ul_pack_writeallo_ms(int sockfd, char *buf, int n, int msec);



#endif
/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
