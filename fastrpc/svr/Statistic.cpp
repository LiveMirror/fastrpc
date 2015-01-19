#include <list>
#include "Statistic.h"

CStatistic* CStatistic::m_pStatistic = NULL;
struct IP_NUM
{
    char ip[16];
    int num;
};
int sort_ip_num_fun( const void *a, const void *b)
{
    return ((IP_NUM *)a)->num < ((IP_NUM *)b)->num;
}
struct UIN_NUM
{
    unsigned uin;
    int num;
};
int sort_uin_num_fun( const void *a, const void *b)
{
    return ((UIN_NUM *)a)->num < ((UIN_NUM *)b)->num;
}
struct ERR_NUM
{
    int err;
    int num;
};
int sort_err_num( const void *a, const void *b)
{
    return ((ERR_NUM *)a)->num < ((ERR_NUM *)b)->num;
}
void sort_ip_num(map<string,int> &map_ip, IP_NUM *&p_ip_num)
{
    if (!map_ip.empty())
    {
        p_ip_num = new IP_NUM[map_ip.size()];
        map<string,int>::iterator it = map_ip.begin();
        for (int i = 0; it != map_ip.end(); it++,i++)
        {
            strncpy(p_ip_num[i].ip, it->first.c_str(), sizeof(p_ip_num[i].ip));
            p_ip_num[i].num = it->second;
        }
        qsort((void *)p_ip_num, map_ip.size(), sizeof(IP_NUM), sort_ip_num_fun);
    }
}
void sort_uin_num(map<unsigned,int> &map_uin, UIN_NUM *&p_uin_num)
{
    if (!map_uin.empty())
    {
        p_uin_num = new UIN_NUM[map_uin.size()];
        map<unsigned,int>::iterator it = map_uin.begin();
        for (int i = 0; it != map_uin.end(); it++,i++)
        {
            p_uin_num[i].uin = it->first;
            p_uin_num[i].num = it->second;
        }   
        qsort((void *)p_uin_num, map_uin.size(), sizeof(UIN_NUM), sort_uin_num_fun);
    }   
}
void sort_err_num(map<int,int> &map_err, ERR_NUM *&p_err_num)
{
    if ( !map_err.empty() )
    {
        p_err_num = new ERR_NUM[map_err.size()];
        map<int,int>::iterator it = map_err.begin();
        for (int i = 0; it != map_err.end(); it++,i++)
        {
            p_err_num[i].err = it->first;
            p_err_num[i].num = it->second;
        }   
        qsort((void *)p_err_num, map_err.size(), sizeof(ERR_NUM), sort_err_num);
    }   
}

int CStatistic::Initialize(char *pszLogBaseFile,int iMaxSize/*=20000000*/,int iMaxNum/*=10*/,
						int iTimeOutUs1/*=100000*/,
						int iTimeOutUs2/*=500000*/,
						int iTimeOutUs3/*=1000000*/)
{
	strcpy(m_szLogBase, pszLogBaseFile);
	m_iLogMaxSize = iMaxSize;
	m_iLogMaxNum = iMaxNum;

	m_iTimeOutUs[0] = iTimeOutUs1;
	m_iTimeOutUs[1] = iTimeOutUs2;
	m_iTimeOutUs[2] = iTimeOutUs3;

    _print_err_num = 5;

	m_bUseMutex = true;
	pthread_mutex_init(&m_stMutex, NULL);

	ClearStat();
	return 0;
}

int CStatistic::xAddStep(const char* pszTypeName,int iResultID,struct timeval *pstBegin,struct timeval *pstEnd,unsigned int unSize,string ip)
{
	if(m_bUseMutex)	pthread_mutex_lock(&m_stMutex);
	int iRet = AddStat(pszTypeName,iResultID,pstBegin,pstEnd,unSize,ip,20000);
	if(m_bUseMutex)	pthread_mutex_unlock(&m_stMutex);
	return iRet;
}

TTypeInfo* CStatistic::xGetStat(char* pszTypeName)
{
	TTypeInfo *info = NULL;
	if(m_bUseMutex)	pthread_mutex_lock(&m_stMutex);
	info = GetStat(pszTypeName);
	if(m_bUseMutex)	pthread_mutex_unlock(&m_stMutex);
	return info;
}

CStatistic* CStatistic::GetInstance()
{
	if (NULL == m_pStatistic)
	{
		m_pStatistic = new CStatistic();
	}
	return m_pStatistic;
}

void CStatistic::DestoryInstance()
{
	if (NULL != m_pStatistic)
	{
		delete m_pStatistic;
		m_pStatistic = NULL;
	}
}



int CStatistic::AddStat(const char* pszTypeName,int iResultID/* =STAT_RESULT_OK */, struct timeval *pstBegin/* =NULL */, struct timeval *pstEnd/* =NULL */,unsigned int unSize/* =0 */,
                        string ip/* ="0.0.0.0" */,unsigned uin/* =9999 */)
{
	for(int i=0; i<m_iTypeNum; i++)
	{
		if(strcmp(pszTypeName,m_astTypeInfo[i].m_szTypeName)==0)
		{

            TTypeInfo &info = m_astTypeInfo[i];

			m_astTypeInfo[i].m_unAllCount++;
			if(iResultID != STAT_RESULT_OK)
			{
				m_astTypeInfo[i].m_unFailedCount++;

                map<string,int>::iterator iter = info.map_ip.find(ip);
                if ( iter == info.map_ip.end() ) m_astTypeInfo[i].map_ip[ip] = 1;
                else m_astTypeInfo[i].map_ip[ip]++;

                //printf("uin=%u map size=%u\n",uin,info.map_uin.size());
                //info.map_uin[uin] = 1;
                //printf("uin=%u map size=%u\n",uin,info.map_uin.size());

                map<unsigned,int>::iterator uin_iter = m_astTypeInfo[i].map_uin.find(uin);
                if ( uin_iter == m_astTypeInfo[i].map_uin.end() ) m_astTypeInfo[i].map_uin[uin] = 1;
                else m_astTypeInfo[i].map_uin[uin]++;

                map<int,int>::iterator err_iter = m_astTypeInfo[i].map_err.find(iResultID);
                if ( err_iter == m_astTypeInfo[i].map_err.end() ) m_astTypeInfo[i].map_err[iResultID] = 1;
                else m_astTypeInfo[i].map_err[iResultID]++;

			}			
			m_astTypeInfo[i].m_unSize += unSize;
			_AddTime(i,pstBegin, pstEnd);	
			return 0;				
		}
	}

	//第一次统计iType	
	if(m_iTypeNum >= TYPE_NUM)
	{
		printf("ERROR:No type Alloc!\n[%s:%d]",__FILE__,__LINE__);
		return -1;
	}
	
    m_astTypeInfo[m_iTypeNum].Clear();
	strncpy(m_astTypeInfo[m_iTypeNum].m_szTypeName,pszTypeName,MAXTYPENAME-1);

	m_astTypeInfo[m_iTypeNum].m_unAllCount = 1;

	if(iResultID != STAT_RESULT_OK)
	{
		m_astTypeInfo[m_iTypeNum].m_unFailedCount = 1;

        m_astTypeInfo[m_iTypeNum].map_ip[ip] = 1;
        m_astTypeInfo[m_iTypeNum].map_uin[uin] = 1;
        m_astTypeInfo[m_iTypeNum].map_err[iResultID] = 1;
	}
	
	m_astTypeInfo[m_iTypeNum].m_unSize = unSize;	
	_AddTime(m_iTypeNum,pstBegin, pstEnd);
	
	++m_iTypeNum;

	//按统计名称排序
	//qsort((void *)&m_astTypeInfo[0],m_iTypeNum,sizeof(TTypeInfo),fTypeInfoCmp);
	return 0;
}
	
int CStatistic::WriteToFile(bool display_map_err_ip_uin)
{
	if(m_bUseMutex)		pthread_mutex_lock(&m_stMutex);
	if(m_iTypeNum == 0)	
	{
		if(m_bUseMutex)		pthread_mutex_unlock(&m_stMutex);
		return 0;
	}

	int iStatSpan = time(0)-m_iLastClearTime;
	// 5分钟打印一次
	if ( iStatSpan<STAT_WRITE_SPAN )
	{
		if(m_bUseMutex)		pthread_mutex_unlock(&m_stMutex);
		return 0;
	}


	unsigned long long  unTmpAveUSec;	
	char szTmpStr[128];
	char szTmpStr2[128];
	char szTmpStr3[128];
	GetDateString(szTmpStr);

	_WriteLog("\nStatistic in %ds, %s\n\n",iStatSpan,szTmpStr);

    _WriteLog("---------------------------------------------------------------------------------------------------------------\n");

	sprintf(szTmpStr,">%.3fms",m_iTimeOutUs[0]/(float)1000);
	sprintf(szTmpStr2,">%.3fms",m_iTimeOutUs[1]/(float)1000);
	sprintf(szTmpStr3,">%.3fms",m_iTimeOutUs[2]/(float)1000);
	
	_WriteLog("%-20s|%8s|%8s|%8s|%8s|%8s|%8s|%11s|%11s|%11s|\n",
			"","TOTAL","FAILED","AVG(ms)","MAX(ms)","MIN(ms)","ALLSIZE",szTmpStr,szTmpStr2,szTmpStr3);
	
	TTypeInfo stAllTypeInfo;
	stAllTypeInfo.Clear();

	for(int i=0; i<m_iTypeNum; i++)
	{
        // 平均时间 = 总的时间 /总的次数
		unTmpAveUSec = 1000000* (unsigned long long) m_astTypeInfo[i].m_stTime.tv_sec;
		unTmpAveUSec += (unsigned long long) m_astTypeInfo[i].m_stTime.tv_usec;
		unTmpAveUSec /= (unsigned long long)m_astTypeInfo[i].m_unAllCount;
			
		_WriteLog("%-20s|%8u|%8u|%8.2f|%8.2f|%8.2f|%8u|%11u|%11u|%11u|\n",
				m_astTypeInfo[i].m_szTypeName,
				m_astTypeInfo[i].m_unAllCount,
				m_astTypeInfo[i].m_unFailedCount,
				unTmpAveUSec/(float)1000,
				m_astTypeInfo[i].m_unMaxTime/(float)1000,
				m_astTypeInfo[i].m_unMinTime/(float)1000,
				m_astTypeInfo[i].m_unSize,
				m_astTypeInfo[i].m_unTimeOut[0],
				m_astTypeInfo[i].m_unTimeOut[1],
				m_astTypeInfo[i].m_unTimeOut[2]);

		stAllTypeInfo.m_unAllCount += m_astTypeInfo[i].m_unAllCount;
		stAllTypeInfo.m_unFailedCount += m_astTypeInfo[i].m_unFailedCount;

		if(stAllTypeInfo.m_unMaxTime < m_astTypeInfo[i].m_unMaxTime)
			stAllTypeInfo.m_unMaxTime = m_astTypeInfo[i].m_unMaxTime;

		if((stAllTypeInfo.m_unMinTime==0)||(stAllTypeInfo.m_unMinTime > m_astTypeInfo[i].m_unMinTime))
			stAllTypeInfo.m_unMinTime = m_astTypeInfo[i].m_unMinTime;
		
		stAllTypeInfo.m_stTime.tv_sec += m_astTypeInfo[i].m_stTime.tv_sec;
		stAllTypeInfo.m_stTime.tv_usec += m_astTypeInfo[i].m_stTime.tv_usec;

		stAllTypeInfo.m_unTimeOut[0] += m_astTypeInfo[i].m_unTimeOut[0];
		stAllTypeInfo.m_unTimeOut[1] += m_astTypeInfo[i].m_unTimeOut[1];
		stAllTypeInfo.m_unTimeOut[2] += m_astTypeInfo[i].m_unTimeOut[2];
		stAllTypeInfo.m_unSize += m_astTypeInfo[i].m_unSize;
	}

    // 统计所有统计项的时间

	if(stAllTypeInfo.m_stTime.tv_usec > 1000000)
	{
		stAllTypeInfo.m_stTime.tv_usec %= 1000000;
		stAllTypeInfo.m_stTime.tv_sec +=(stAllTypeInfo.m_stTime.tv_usec/1000000);
	}
	
	unTmpAveUSec = 1000000*(unsigned long long )stAllTypeInfo.m_stTime.tv_sec;
	unTmpAveUSec += (unsigned long long) stAllTypeInfo.m_stTime.tv_usec;
	unTmpAveUSec /= (unsigned long long ) stAllTypeInfo.m_unAllCount;
		
	_WriteLog("---------------------------------------------------------------------------------------------------------------\n");	
	_WriteLog("%-20s|%8u|%8u|%8.2f|%8.2f|%8.2f|%8u|%11u|%11u|%11u|\n",
			"ALL",
			stAllTypeInfo.m_unAllCount,
			stAllTypeInfo.m_unFailedCount,
			unTmpAveUSec/(float)1000,
			stAllTypeInfo.m_unMaxTime/(float)1000,
			stAllTypeInfo.m_unMinTime/(float)1000,
			stAllTypeInfo.m_unSize,
			stAllTypeInfo.m_unTimeOut[0],
			stAllTypeInfo.m_unTimeOut[1],
			stAllTypeInfo.m_unTimeOut[2]);

    // 显示热点查询词


    // 显示 Preupload 形式 的错误IP,Uin,ErrCode.
    if ( display_map_err_ip_uin )
    {
        // 错误IP
        _WriteLog("\n\n\n");
        _WriteLog("The distribution of Error Operation's errcode\n");
        _WriteLog("---------------------------------------------------------------------------------------------------------------\n");
        _WriteLog("     Operation| %11s %11s %11s %11s %11s %11s\n",
            "ERR_1","ERR_2","ERR_3","ERR_4","ERR_5","ERR_6");
        _WriteLog("---------------------------------------------------------------------------------------------------------------\n");

        for(int i=0; i<m_iTypeNum; i++) 
        {
            TTypeInfo &statInfo = m_astTypeInfo[i];

            if (!statInfo.map_err.empty())
            {
                ERR_NUM *p_err_num = NULL;
                _WriteLog("%14s| ", statInfo.m_szTypeName);

                sort_err_num(statInfo.map_err, p_err_num);
                for (unsigned i = 0; i < statInfo.map_err.size() && (int)i < _print_err_num; i++)
                {
                    char tmp[80] = {0};
                    sprintf(tmp, "%d/%d", p_err_num[i].err, p_err_num[i].num);
                    _WriteLog(" %11s", tmp);
                }

                _WriteLog("\n");
                if ( p_err_num != NULL ) 
                {
                    delete []p_err_num;
                    p_err_num = NULL;
                }
            }
        }


        //- print ip/uin distributed
        _WriteLog("\n\n\n");
        _WriteLog("The distribution of Error Operation's IP & UIN\n");
        _WriteLog("---------------------------------------------------------------------------------------------------------------\n");
        _WriteLog("     Operation| %21s %21s %21s\n",  "IP_1/UIN_1", "IP_2/UIN_2", "IP_3/UIN_3");
        _WriteLog("---------------------------------------------------------------------------------------------------------------\n");

        for(int i=0; i<m_iTypeNum; i++) 
        {
            TTypeInfo &statInfo = m_astTypeInfo[i];

            IP_NUM *p_ip_num = NULL;
            UIN_NUM *p_uin_num = NULL;

            if (!statInfo.map_ip.empty())
            {
                _WriteLog("%14s| ", statInfo.m_szTypeName);

                sort_ip_num(statInfo.map_ip, p_ip_num);
                sort_uin_num(statInfo.map_uin, p_uin_num);
                for (unsigned i = 0; i < statInfo.map_ip.size() && i < 3; i++)
                {
                    char tmp[80] = {0};
                    sprintf(tmp, "%15s -> %-5d", p_ip_num[i].ip, p_ip_num[i].num);
                    _WriteLog(" %21s", tmp);
                }
                _WriteLog("\n");
                _WriteLog("%14s| ", "");
                for (unsigned i = 0; i < statInfo.map_uin.size() && i < 3; i++)
                {
                    char tmp[80] = {0};
                    sprintf(tmp, "%15u -> %-5d", p_uin_num[i].uin, p_uin_num[i].num);
                    _WriteLog(" %21s", tmp);
                }

                _WriteLog("\n");

                if ( p_ip_num ) { delete []p_ip_num;  p_ip_num = NULL;  }
                if ( p_uin_num ) { delete []p_uin_num; p_uin_num = NULL; }
            }
        }
    }
    
    ClearStat();
	_ShiftFiles();

	if(m_bUseMutex)		pthread_mutex_unlock(&m_stMutex);
    return 0;
}

TTypeInfo* CStatistic::GetStat(char* pszTypeName)
{
    if(!pszTypeName)
        return NULL;

    for(int i=0; i<m_iTypeNum; i++)
    {
        if(strcmp(pszTypeName,m_astTypeInfo[i].m_szTypeName)==0)
        {
            return &m_astTypeInfo[i];
        }
    }
    return NULL;
}

void CStatistic::ClearStat()
{
	m_iLastClearTime = time(0);
	m_iTypeNum = 0;

	for(unsigned i=0; i<TYPE_NUM; i++)
    {
        m_astTypeInfo[i].Clear();
    }
}

void CStatistic::_AddTime(int iTypeIndex,struct timeval *pstBegin, struct timeval *pstEnd)
{
	if (!pstBegin || !pstEnd)
	{
		return;
	}

	unsigned int unTimeSpanUs = (pstEnd->tv_sec - pstBegin->tv_sec)*1000000 + 
								(pstEnd->tv_usec - pstBegin->tv_usec);

    // 超时个数
	if(unTimeSpanUs > m_iTimeOutUs[0])
		m_astTypeInfo[iTypeIndex].m_unTimeOut[0]++;
	if(unTimeSpanUs > m_iTimeOutUs[1])
		m_astTypeInfo[iTypeIndex].m_unTimeOut[1]++;
	if(unTimeSpanUs > m_iTimeOutUs[2])
		m_astTypeInfo[iTypeIndex].m_unTimeOut[2]++;
	
    // 最大时间
	if(unTimeSpanUs>m_astTypeInfo[iTypeIndex].m_unMaxTime)
	{
		m_astTypeInfo[iTypeIndex].m_unMaxTime = unTimeSpanUs;
	}
    // 最小时间
	if((m_astTypeInfo[iTypeIndex].m_unMinTime == 0)||
		(unTimeSpanUs<m_astTypeInfo[iTypeIndex].m_unMinTime))
	{
		m_astTypeInfo[iTypeIndex].m_unMinTime = unTimeSpanUs;
	}

    // 总的使用时间
	m_astTypeInfo[iTypeIndex].m_stTime.tv_sec += (pstEnd->tv_sec - pstBegin->tv_sec);
	m_astTypeInfo[iTypeIndex].m_stTime.tv_usec += (pstEnd->tv_usec - pstBegin->tv_usec);

	if(m_astTypeInfo[iTypeIndex].m_stTime.tv_usec > 1000000)
	{
		m_astTypeInfo[iTypeIndex].m_stTime.tv_sec++;
		m_astTypeInfo[iTypeIndex].m_stTime.tv_usec -= 1000000;
	}

	if(m_astTypeInfo[iTypeIndex].m_stTime.tv_usec<0)
	{
		m_astTypeInfo[iTypeIndex].m_stTime.tv_usec += 1000000;
		m_astTypeInfo[iTypeIndex].m_stTime.tv_sec--;
	}
}

/*
    x.log
    x0.log
    x1.log
    x2.log
    x3.log

    如果判断 x.log 文件大小大于 m_iLogMaxSize
    就把 x.log 改名为 x0.log
    x0.log 改名为 x1.log 等等。

    删除最好的，并保持老的文件数，不超过 m_iLogMaxNum。

*/
int CStatistic::_ShiftFiles()
{
	struct stat stStat;
	char sLogFileName[300];
	char sNewLogFileName[300];
	int i;

	sprintf(sLogFileName,"%s.log", m_szLogBase);
	if(stat(sLogFileName, &stStat) < 0)
		return -1;

	if (stStat.st_size < m_iLogMaxSize)
		return 0;

	sprintf(sLogFileName,"%s%d.log", m_szLogBase, m_iLogMaxNum-1);
	if (access(sLogFileName, F_OK) == 0)
	{
		if (remove(sLogFileName) < 0 )
			return -1;
	}

	for(i = m_iLogMaxNum-2; i >= 0; i--)
	{
		if (i == 0)
			sprintf(sLogFileName,"%s.log", m_szLogBase);
		else
			sprintf(sLogFileName,"%s%d.log", m_szLogBase, i);
			
		if (access(sLogFileName, F_OK) == 0)
		{
			sprintf(sNewLogFileName,"%s%d.log", m_szLogBase, i+1);
			if (rename(sLogFileName,sNewLogFileName) < 0 )
			{
				return -1;
			}
		}
	}
	return 0;
}

void CStatistic::_WriteLog(const char *sFormat, ...)
{
	va_list ap;
	va_start(ap, sFormat);
	
	FILE  *pstFile;
	char szLogFileName[300];
   	sprintf(szLogFileName,"%s.log", m_szLogBase);
	if ((pstFile = fopen(szLogFileName, "a+")) == NULL)
	{
		printf("Fail to open log file %s\n",szLogFileName);
		return;
	}
	vfprintf(pstFile, sFormat, ap);
	fclose(pstFile);
	va_end(ap);
	
	//_ShiftFiles();
}

void CStatistic::GetDateString(char *szTimeStr)
{
	timeval tval;
	gettimeofday(&tval,NULL);
	struct tm curr;
	curr = *localtime(&tval.tv_sec);

	if (curr.tm_year > 50)
	{
		sprintf(szTimeStr, "%04d-%02d-%02d %02d:%02d:%02d", 
			curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
			curr.tm_hour, curr.tm_min, curr.tm_sec);
	}
	else
	{
		sprintf(szTimeStr, "%04d-%02d-%02d %02d:%02d:%02d",
	        curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
	        curr.tm_hour, curr.tm_min, curr.tm_sec);
	}
}



/*
x.log
x0.log
x1.log
x2.log
x3.log

如果判断 x.log 文件大小大于 m_iLogMaxSize
就把 x.log 改名为 x0.log
x0.log 改名为 x1.log 等等。

删除最好的，并保持老的文件数，不超过 m_iLogMaxNum。

*/


/*
int main()
{
	timeval tBegin,tEnd;
	gettimeofday(&tBegin,NULL);
	usleep(10000);
	gettimeofday(&tEnd,NULL);

	CStatistic::Instance()->Initialize("mmm", 20000000,10, 100000, 200000, 500000);
	CStatistic::Instance()->AddStat("SetUindex",0, &tBegin,&tEnd, 1024);

	
    gettimeofday(&tBegin,NULL);
	usleep(20000);
	gettimeofday(&tEnd,NULL);
	
	CStatistic::Instance()->AddStat("SetUindex",0, &tBegin,&tEnd, 1024);

	gettimeofday(&tBegin,NULL);
	usleep(300000);
	gettimeofday(&tEnd,NULL);
	
	CStatistic::Instance()->AddStat("SetUindex",0, &tBegin,&tEnd, 1024);

	CStatistic::Instance()->WriteToFile();
	CStatistic::Instance()->ClearStat();
	sleep(3);
	gettimeofday(&tBegin,NULL);
	usleep(600000);
	gettimeofday(&tEnd,NULL);
	
	CStatistic::Instance()->AddStat("SetUindex",STAT_RESULT_FAILED, &tBegin,&tEnd, 1024);

	gettimeofday(&tBegin,NULL);
	usleep(1000000);
	gettimeofday(&tEnd,NULL);
	
	CStatistic::Instance()->AddStat("SetFindex",STAT_RESULT_FAILED, &tBegin,&tEnd, 1024);
	CStatistic::Instance()->AddStat("SetFyndex",STAT_RESULT_FAILED, &tBegin,&tEnd, 1024);
    
    
    CStatistic::Instance()->AddStat("GetUindex",0, &tBegin,&tEnd, 1024);
    CStatistic::Instance()->AddStat("GetUindex",0, &tBegin,&tEnd, 1024);
    CStatistic::Instance()->AddStat("GetUindex",STAT_RESULT_FAILED, &tBegin,&tEnd, 1024,"192.168.0.1",51405);
    
    CStatistic::Instance()->AddStat("DelUindex",STAT_RESULT_FAILED, &tBegin,&tEnd, 1024);

    CStatistic::Instance()->AddStat("Indexer",-7001, &tBegin,&tEnd, 1024,"192.168.0.1",51405);
    CStatistic::Instance()->AddStat("DelUindex",-7001, &tBegin,&tEnd, 1024,"192.168.0.1",51405);
    CStatistic::Instance()->AddStat("DelUindex",-7001, &tBegin,&tEnd, 1024,"192.168.0.1",51405);
    CStatistic::Instance()->AddStat("DelUindex",-7001, &tBegin,&tEnd, 1024,"192.168.0.1",51405);
    CStatistic::Instance()->AddStat("DelUindex",-9001, &tBegin,&tEnd, 1024,"192.168.0.2",51405);
    CStatistic::Instance()->AddStat("DelUindex",-1000, &tBegin,&tEnd, 1024,"192.168.0.2",51405);


	CStatistic::Instance()->AddStat("MSGIN");
	CStatistic::Instance()->AddStat("MSGIN");
	CStatistic::Instance()->AddStat("MSGIN");
	CStatistic::Instance()->WriteToFile();
	CStatistic::Instance()->ClearStat();
	return 0;
}
*/
