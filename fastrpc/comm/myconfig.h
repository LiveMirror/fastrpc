#ifndef myconfig_h
#define myconfig_h

typedef struct _MyConfig
{
	const char *name;
	char *value;
}MyConfig;

int myconfig_init (const char *filename);
const char *myconfig_get (const char *name);
const char *myconfig_set (const char *name, const char *value);
void myconfig_destroy ();

#endif
