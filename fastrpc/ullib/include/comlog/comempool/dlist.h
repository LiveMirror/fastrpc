/***************************************************************************
 * 
 * Copyright (c) 2008 feimat.com, Inc. All Rights Reserved
 * $Id: dlist.h,v 1.2 2009/03/25 12:34:11 feimat Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file dlist.h
 * @author feimat(feimat@feimat.com)
 * @date 2008/03/10 14:27:09
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/


#ifndef  __DLIST_H_
#define  __DLIST_H_

namespace comspace
{

struct dlist_t
{
	dlist_t *next;
	int size[0];
};

dlist_t *dlist_insert(dlist_t *root, dlist_t *node);
dlist_t *dlist_erase(dlist_t *root, dlist_t *node);

int dlist_walk(dlist_t *root, void (*fun)(void *));
};

#endif  //__DLIST_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
