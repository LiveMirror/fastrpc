#ifndef __STATISTIC_H__
#define __STATISTIC_H__

/*
	feimat@feimat.com
	myindex服务器framework的第三方库的统计代码 

	会把client请求各个过程的耗时，成功次数，失败次数，平均延时，最大延时，最小延时统计出来。
	使用方法见 test.cpp

	Statistic in 60s, 2013-01-09 15:20:12

	---------------------------------------------------------------------------------------------------------------
						|   TOTAL|  FAILED| AVG(ms)| MAX(ms)| MIN(ms)| ALLSIZE| >100.000ms| >500.000ms|>1000.000ms|
	GetCache            |       1|       0|   12.07|   12.07|   12.07|       1|          0|          0|          0|
	ReadMysql           |       1|       0|    1.99|    1.99|    1.99|       1|          0|          0|          0|
	WriteMysql          |       1|       0|   12.15|   12.15|   12.15|       1|          0|          0|          0|
	---------------------------------------------------------------------------------------------------------------
	ALL                 |       3|       0|    8.74|   12.15|    1.99|       3|          0|          0|          0|


*/

#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <map>
#include <string>
#include <vector>
#include <pthread.h>


using namespace std;

#define STAT_OPEN                   CStatistic::GetInstance()->Initialize
#define STAT_ADD                    CStatistic::GetInstance()->xAddStep
#define STAT_WRITE_FILE             CStatistic::GetInstance()->WriteToFile


#define TYPE_NUM 50
#define MAXTYPENAME	64
#define MAX_TIMEOUT_LEVEL	3

#define STAT_RESULT_OK	0
#define STAT_RESULT_FAILED	(-1)


#define STAT_WRITE_SPAN		(60)

struct TTypeInfo
{
	char m_szTypeName[MAXTYPENAME];
	unsigned int m_unAllCount;
	unsigned int m_unFailedCount;
	unsigned int m_unMaxTime;
	unsigned int m_unMinTime;
	struct timeval m_stTime;	
	unsigned int m_unTimeOut[MAX_TIMEOUT_LEVEL];
	unsigned int m_unSize;	

    map<int, int> map_err;		//-  错误码分布
    map<string,int> map_ip;		//-  ip分布 
    map<unsigned,int> map_uin;	//-  uin分布

    TTypeInfo() { 
        Clear();
    }

    void Clear(){
        memset(m_szTypeName,0,sizeof(m_szTypeName));
        m_unAllCount = 0;
        m_unFailedCount = 0;
        m_unMaxTime = 0;
        m_unMinTime = 0;
        
        memset(&m_stTime,0,sizeof(m_stTime));
        memset(m_unTimeOut,0,sizeof(m_unTimeOut));

        m_unSize = 0;
        map_err.clear();
        map_ip.clear();
        map_uin.clear();
    }

};

class CStatistic
{
public:
	
	CStatistic(){};
	~CStatistic(){};

	static CStatistic* GetInstance();
	void DestoryInstance();

	//iTimeOutUs :时间计数标尺,单位us
	int Initialize(char *pszLogBaseFile=(char *)"./Statistic",int iMaxSize=20000000,int iMaxNum=10,
		int iTimeOutUs1=100000,
		int iTimeOutUs2=500000,
		int iTimeOutUs3=1000000);

    // 带x开头的函数都是加线程锁的。
	int xAddStep(const char* pszTypeName,int iResultID=STAT_RESULT_OK, 
	struct timeval *pstBegin=NULL, struct timeval *pstEnd=NULL,unsigned int unSize=0,string ip="0.0.0.0");

	TTypeInfo* xGetStat(char* pszTypeName);
	// 没加线程锁函数
	int AddStat(const char* pszTypeName,int iResultID=STAT_RESULT_OK, 
			struct timeval *pstBegin=NULL, struct timeval *pstEnd=NULL,unsigned int unSize=0,string ip="0.0.0.0",unsigned uin=9999);
	
	int AddStat(int iTypeName,int iResultID=0, 
			struct timeval *pstBegin=NULL, struct timeval *pstEnd=NULL,unsigned int unSize=0,string ip="0.0.0.0",unsigned uin=9999)
	{
		char szTmp[MAXTYPENAME];
		sprintf(szTmp,"%d",iTypeName);
		return AddStat(szTmp,iResultID,pstBegin,pstEnd,unSize,ip,uin);
	}
	TTypeInfo* GetStat(char* pszTypeName);
	
	int WriteToFile(bool display_map_err_ip_uin = false);	
	void ClearStat();	

private:
	static CStatistic* m_pStatistic;
	void GetDateString(char *szTimeStr);
	int _ShiftFiles();
	void _WriteLog(const char *sFormat, ...);
	void _AddTime(int iTypeIndex,struct timeval *pstBegin, struct timeval *pstEnd);
	
	int m_iTypeNum;
	TTypeInfo m_astTypeInfo[TYPE_NUM];
    //vector<TTypeInfo> m_astTypeInfo;
	
	char m_szLogBase[256];
	int m_iLogMaxSize;
	int m_iLogMaxNum;

	unsigned int m_iTimeOutUs[MAX_TIMEOUT_LEVEL];
	
	int m_iLastClearTime;
    int _print_err_num;

	//互斥使用
	bool m_bUseMutex;
	pthread_mutex_t m_stMutex;
};


#endif

