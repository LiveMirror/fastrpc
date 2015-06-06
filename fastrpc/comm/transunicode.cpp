#include "transunicode.hpp"
#include <iconv.h>
extern "C"
{
	#include "base64.h"
}
using namespace std;
/*int main(int argc,char *argv[])
{
	FILE *fp = fopen("/home/work/log/encode.txt", "r");
	int nSign = 0;
	char filename[128] = {0};
	char szDate[32] = {0};
	DateAdd(-1, szDate, (char *)"%Y%m%d", sizeof(szDate));
	sprintf(filename, "/home/work/log/code_%s.txt", szDate);
	while(1)
	{
		if(feof(fp)) break;
		char value[1024*15] = {0};
		fgets(value, sizeof(value), fp);
		char dename[1024*15] = {0};
		int  b64resnum = 1;
		base64_decode((unsigned char *)value, strlen(value), (unsigned char *)dename, &b64resnum);
		int nHandle = open(filename,O_CREAT|O_APPEND|O_RDWR, 0666);
		if(nHandle && nSign == 0)
        	{
                	write(nHandle,dename,b64resnum);
			nSign = 1;
        	} else if(nHandle && nSign == 1)
		{
			write(nHandle,dename+2,b64resnum-2);
		}
                close(nHandle);
	}
	fclose(fp);
	return 0;
}*/


