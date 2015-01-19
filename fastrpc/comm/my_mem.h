#ifndef _MY_MEM_HEADER_H_
#define _MY_MEM_HEADER_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#include "mem_hash.h"
       
#define MEM_LEN (1000000000/8)+1
#define MEM_HEAD_LEN 32

typedef struct for_mem
{
	int mem_len;
	int key;
	int shm_id;
	char *p;
	char *p_mem;
	int m_len;
} for_mem;

typedef struct m_byte 
{
	char b1:1;
	char b2:1;
	char b3:1;
	char b4:1;
	char b5:1;
	char b6:1;
	char b7:1;
	char b8:1;
} m_byte;

extern int init_mem(for_mem *mymem, int key, int len);
extern int get_tel_status(for_mem *mymem, char *tel);
int read_16(char *str);
int init_mem_hash_head(for_mem *mymem, ht **d);
int get_mem_hash_value(char *name, char *value, for_mem *mymem, ht **d);
int get_num_mem_value(char *name, char *value, for_mem *mymem, ht **d);
#endif
