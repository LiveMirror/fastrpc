#ifndef _MEM_HASH_HEADER_H_
#define _MEM_HASH_HEADER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define READ_WAIT -2

#ifndef KEY_DEF
typedef const char *KEY;
#endif

typedef struct ht
{
	unsigned int size;
	unsigned int num;
	char name_len;
	char value_len;
	char status;
	char psize;
}ht;

int mem_hash_init(ht *d, int name_len, int value_len, int max_size);
int mem_hash_init_new(ht *d, int name_len, int value_len, int max_size, int my_way);
int finish_hash(ht *d);
int pause_hash(ht *d);
int get_key(char *p, ht *d, char *name );
int make_key(char *p, ht *d, char *name, char *value);
char * get_mem_name(char *p, ht *d, size_t h);
char * get_mem_value(char *p, ht *d, size_t h);
void input_mem(char *p, ht *d, char *value, unsigned int i);

#endif
