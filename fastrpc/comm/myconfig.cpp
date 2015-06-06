#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "myconfig.h"
#include "hashtable.h"

//static MyConfig _default_config;
static HASH_TABLE _myconfig_list;

int
myconfig_init (const char *filename)
{
	FILE *fp;
	char buf[8192];
	char *p, *q;
	int c;

	if (!filename)
		return -1;

	hashTableInit(&_myconfig_list);

	if ((fp = fopen (filename, "r")) == NULL)
	{
		return -1;
	}

	while (fgets (buf, sizeof (buf), fp))
	{
		p = (char *) strchr (buf, '\n');
		if (!p)
		{
			while ((c = getc (fp)) >= 0 && c != '\n')
				;
		}
		else
		{
			*p = 0;
		}
		/*
		if ((p = (char *) strchr (buf, '#')) != 0)
			*p = 0;
			*/
		for (p = buf; *p; p++)
			if (!isspace ((int) (unsigned char) *p))
				break;

		if (*p == 0 || *p == '#') continue;

		for (; *p; p++)
			if (isspace ((int) (unsigned char) *p) || *p == '=')
				break;
		if (*p == 0)
			continue;
		*p++ = 0;

		while (*p && isspace ((int) (unsigned char) *p))
			++p;
		if (*p == '"')
		{
			++p;
			q = (char *) strchr (p, '"');
			if (q)
				*q = 0;
		}
		myconfig_set (buf, p);
	}
	fclose (fp);
	return 0;
}
const char *
myconfig_get (const char *name)
{
	MyConfig *mcnode;

	if (!name)
		return NULL;

	mcnode=(MyConfig *)hashTableLookup(&_myconfig_list,name,0);
	if (mcnode)
	{
		if (mcnode->value)
			return mcnode->value;
	}
	return NULL;
}
const char *
myconfig_set (const char *name, const char *value)
{
	MyConfig *mcnode;

	if (!name || !value)
		return NULL;


	mcnode=(MyConfig *)hashTableLookup(&_myconfig_list,name,0);
	if (mcnode)
	{
		if (mcnode->value) free(mcnode->value);
		mcnode->value=strdup(value);
	}
	else
	{
		mcnode=(MyConfig *)hashTableLookup(&_myconfig_list,strdup(name),sizeof(MyConfig));
		if (!mcnode)
		{
			return NULL;
		}
		else
		{
			mcnode->value=strdup(value);
		}
	}
	return mcnode->value;
}

void
myconfig_destroy ()
{
	MyConfig *mcnode;
	HASH_TABLE_ITER iter;

	hashTableIterInit(&iter,&_myconfig_list);
	while ((mcnode=(MyConfig *)hashTableIterNext(&iter))!=NULL)
	{
		free((void *)mcnode->name);
		if (mcnode->value) free(mcnode->value);
	}
	hashTableDestroy(&_myconfig_list);
	return;
}
