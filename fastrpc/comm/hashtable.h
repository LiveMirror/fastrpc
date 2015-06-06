#ifndef hashtable_h
#define hashtable_h

#include <stddef.h>

typedef const char *KEY;

#define KEY_DEF

typedef struct
{
	KEY name;
}
NAMED;

typedef struct
{
	NAMED **v;
	size_t size;
	size_t used;
	size_t usedLim;
}
HASH_TABLE;

NAMED *hashTableLookup (HASH_TABLE * table, KEY name, size_t createSize);
void hashTableInit (HASH_TABLE *);
void hashTableDestroy (HASH_TABLE *);

typedef struct
{
	NAMED **p;
	NAMED **end;
}
HASH_TABLE_ITER;

void hashTableIterInit (HASH_TABLE_ITER *, const HASH_TABLE *);
NAMED *hashTableIterNext (HASH_TABLE_ITER *);

#endif
