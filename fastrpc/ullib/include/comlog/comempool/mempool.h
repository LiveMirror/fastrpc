/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: mempool.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file mempool.h
 * @author feimat(feimat@baidu.com)
 * @date 2008/03/07 21:53:09
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __MEMPOOL_H_
#define  __MEMPOOL_H_

#include "comlog/comempool/dlist.h"

namespace comspace
{

#define SIZEOFPTR(x) \
	((*((int *)((size_t)(x) - sizeof(int)))) - (int)sizeof(int))

struct memlist_t
{
	memlist_t *next;
	long  used;
	char cnt[0];
};

class FixMem
{
	//分配的内存链表
	memlist_t *_mem;
	//目前正在分配的内存链表
	memlist_t *_now;
	//回收链表头
	memlist_t *_free;

	//分配计数
	int _nowb;
	int _reab;

	//连续内存块的大小
	long _size;

	//定长代码块的大小
	int _bsize;
	//每次分配多少块
	int _bnum;
public:
	int create(int bnum, int bsize);
	void destroy();
	void *alloc();
	void dealloc(void *);
	void reset();
};


class MemPool
{
	static const int DEFLEVEL = 11;
	static const int DEFMAXMEM = 1<<20;
	static const int DEFMINMEM = 1<<10;

	//管理的内存区间，最小内存到最大内存，不包括头int
	int _maxmem;
	int _minmem;
	//目前所能分配的最大内存，超过部分直接malloc
	int _allocmem;
	//级数递增的比例
	float _rate;
	//目前最大的级数
	int _level;
	//内存管理池
	FixMem _fixmem[DEFLEVEL];
	//大内存的回收链表
	dlist_t *_biglist;
public:
	/**
	 * @brief 创建内存池
	 *
	 * @param [in/out] minmem   : int	最小的内存分配单元
	 * @param [in/out] maxmem   : int	管理最大的连续内存
	 * @param [in/out] rate   : float	根据最小单元每级递增的比例，不超过最大管理的连续内存
	 * 									最多10级
	 * @return  int 	成功返回0，失败－1
	 * @retval   
	 * @see 
	 * @author feimat
	 * @date 2008/03/10 11:31:54
	**/
	int create(int minmem=DEFMINMEM, int maxmem=DEFMAXMEM, float rate=2.0f);

	/**
	 * @brief 删除mempool
	 *
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author feimat
	 * @date 2008/03/10 14:21:45
	**/
	void destroy();

	/**
	 * @brief 分配内存
	 *
	 * @param [in/out] size   : int 需要分配的内存大小
	 * @return  void* 返回可用的内存指针，失败返回-1
	 * @retval   
	 * @see 
	 * @author feimat
	 * @date 2008/03/10 11:33:17
	**/
	void *alloc(int size);

	/**
	 * @brief 释放内存
	 *
	 * @return  void 
	 * @retval   
	 * @see 
	 * @author feimat
	 * @date 2008/03/10 14:20:27
	**/
	void dealloc(void *);

	void reset();

public:
	int getidx(int size);
};


};

#endif  //__MEMPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
