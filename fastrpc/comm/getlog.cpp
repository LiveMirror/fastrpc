#include "getlog.hpp"
#include "hashtable.h"
#include "myconfig.h"
/*int main()
{
	char *pszData = NULL;
	CGI cgi(pszData);
	char ReturnMsg[20] = {0};
	char szConfig[50] = {0};
	sprintf(szConfig,"/home/work/provision/smsapachemode/conf/getlog.conf");
	if (myconfig_init(szConfig))
    	{
        	exit (1);
    	}
	char *pPtr = NULL;
	pPtr = cgi.GetData();//获得XML数据

	if(pPtr != NULL)
	{
		long nNum;
		memcpy(&nNum,pPtr,4);
		if(ntohl(nNum) == strlen(pPtr+4))
		{
			sprintf(ReturnMsg,"SUCCESS");
			WriteDirLog((char *)myconfig_get("LOGFILE"),(char *)"SUCC:%s",pPtr+4);
			cgi.PrintHTTPHead(strlen(ReturnMsg));
			printf("%s",ReturnMsg);
		} else {
			WriteDirLog((char *)myconfig_get("LOGFILE"),(char *)"FAIL:%s",pPtr+4);
			sprintf(ReturnMsg,"FAILERR");
			cgi.PrintHTTPHead(strlen(ReturnMsg));
			printf("%s",ReturnMsg);
		}
	} else {
		WriteDirLog((char *)myconfig_get("LOGFILE"),(char *)"Get Data Error!!!");
		sprintf(ReturnMsg,"FAILERR");
		cgi.PrintHTTPHead(strlen(ReturnMsg));
		printf("%s",ReturnMsg);
	}
	myconfig_destroy();
	return 0;
}*/
