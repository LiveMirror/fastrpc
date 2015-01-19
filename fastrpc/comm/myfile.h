// myfile.h

#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>

int fifowrite(const char* file_name, const char* pbuf, unsigned long size);
int fiforead(const char* file_name, char* pbuf, unsigned long size);
