/**
************************************
* 	Copyright (c) 2012，Baidu
*	All rights reserved.
************************************
*\n
* 	@file common_def.h
* 	@breif 安全云端服务器全局定义
*\n
* 	@version 0.0.1
* 	@author clarencelei
* 	@date 2012.3.12
*/
#ifndef _SRC_INCLUDE_COMMON_DEF_H_
#define _SRC_INCLUDE_COMMON_DEF_H_
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define DELETE_OBJ(x) \
{ \
if (NULL != x) \
{ \
	delete x;x=NULL;\
} \
}

const int DBNAME_LEN = 20;
const int HOST_LEN = 100;
const int USER_LEN = 20;
const int PASSWD_LEN = 30;
const int MAX_SLAVE_NUM = 10;  // 最大slave个数
const int MAX_REDIS_NUM = 5;   // 最大支持查询的redis缓存的个数

const int SQL_LEN = 1024;  // SQL语句最大长度
const int REDIS_CMD_LEN = SQL_LEN; // REDIS 命令最大长度
const int SINGLE_RESULT_LEN = SQL_LEN; // 单条返回记过的长度
const int MAX_RESULT_LEN = SQL_LEN;
const int MAX_USER_AGENT_LEN = SQL_LEN;

const int FILE_ID_LEN = 32;
const int MAX_IP_LEN = 20;
const int MAX_PROGRAM_INFO_LEN = 20;
const int MAX_MD5_NUM = 100;
const int MAX_MD5_LEN = FILE_ID_LEN * MAX_MD5_NUM;
const int MAX_RES_LEN = MAX_MD5_NUM * SINGLE_RESULT_LEN;

const int MAX_GUID_LEN = 100;
const char UNKOWN_GUID[] = ""; // 此处保留一个空格
const char UNKOWN_USER_AGENT[] = "";


// 保证KEY异或出来的字符在 [0x20 0x7E]中，但不包括以下字符： + 空格 / ? % # & =
const char XorKey[FILE_ID_LEN] = {
	0x01,0x02,0x00,0x01,0x02,0x00,0x01,0x02,
	0x01,0x03,0x01,0x02,0x02,0x01,0x02,0x03,
	0x02,0x02,0x02,0x00,0x03,0x02,0x00,0x03,
	0x03,0x01,0x03,0x01,0x02,0x03,0x01,0x02};

typedef enum EM_CODE_TYPE
{
	NOT_CODE = 0, // URL检测
	BE_CODE = 1  // MD5检测
};

typedef enum EM_CONN_TYPE
{
	WRITE_TYPE = 0, // 写类型
	READ_TYPE = 1   // 读类型
};

const int REDIS_FILE_HASH_KEY_LEN = 5 + FILE_ID_LEN;
const int REDIS_MGET_MAX = 20;
const char REDIS_MGET_CMD[] = "MGET %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s";

// 存放批量查询的key
typedef struct tagMultiKeys
{
	int nKeyNum;// key 的个数
	char strKeys[MAX_MD5_NUM][REDIS_FILE_HASH_KEY_LEN + 1]; // 存放key的数组

	// 结构体构造函数
	tagMultiKeys()
	{
		nKeyNum = 0;
		for (int i = 0; i < MAX_MD5_NUM; ++i)
		{
			strKeys[i][0] = '\0';
		}
	};

	// 初始化结构体
	bool Init()
	{
		nKeyNum = 0;
		for (int i = 0; i < MAX_MD5_NUM; ++i)
		{
			strKeys[i][0] = '\0';
		}
		return true;
	};

	void UnInit()
	{
		//
	};

	void Reset()
	{
		nKeyNum = 0;
		for (int i = 0; i < MAX_MD5_NUM; ++i)
		{
			strKeys[i][0] = '\0';
		}
	};
}MULTI_KEYS;

// 存放批量查询的返回值
typedef struct tagMultiValues
{
	int nValueNum;// value的个数
	char strValues[MAX_MD5_NUM][SINGLE_RESULT_LEN + 1]; // 存放value的数组

	// 结构体构造函数
	tagMultiValues()
	{
		nValueNum = 0;
		for (int i = 0; i < MAX_MD5_NUM; ++i)
		{
			strValues[i][0] = '\0';
		}
	};

	// 初始化结构体
	bool Init()
	{
		nValueNum = 0;
		for (int i = 0; i < MAX_MD5_NUM; ++i)
		{
			strValues[i][0] = '\0';
		}
		return true;
	};

	void UnInit()
	{
		//
	};

	void Reset()
	{
		nValueNum = 0;
		for (int i = 0; i < MAX_MD5_NUM; ++i)
		{
			strValues[i][0] = '\0';
		}
	};
}MULTI_VALUES;

// 请求结构体
typedef struct tagProtocolNode
{
	int nBeCode;						// 是否加密

	int nFileNum;					// 批量查询的md5个数

	char strFileMd5[MAX_MD5_LEN + 1];	// 批量md5查询的原始串

	char strFileId[MAX_MD5_NUM][FILE_ID_LEN + 1];  // 格式化为大写后的md5

	char strResult[MAX_RES_LEN + 1]; // 返回结果

	char strUserAgent[MAX_USER_AGENT_LEN + 1];

	char strGuid[MAX_GUID_LEN + 1];     // 客户端的GUID

	char strIp[MAX_IP_LEN + 1];         // 客户端的IP

	char strProgramName[MAX_PROGRAM_INFO_LEN + 1];              // 客户端程序名字

	char strProgramVer[MAX_PROGRAM_INFO_LEN + 1];				// 客户端程序版本


	// 请求结构体构造函数
	tagProtocolNode()
	{
		memset(this,0,sizeof(tagProtocolNode));
	};

	// 初始化结构体
	bool Init()
	{
		memset(this,0,sizeof(tagProtocolNode));
		return true;
	};

	void UnInit()
	{

	};

	void Reset()
	{
		memset(this,0,sizeof(tagProtocolNode));
	};
}PROTOCOL_NODE;

// 错误码
const int OK = 1;
const int ERROR = -1;
const int NO_RESULT = 2;



#endif

