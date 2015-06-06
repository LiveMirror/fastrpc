/***************************************************************************
 *
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @file http_util.h
 * @hemingzhe02@baidu.com
 * @date 2013/03/19
 * @brief
 *
 **/

#ifndef  __HTTP_UTIL_H_
#define  __HTTP_UTIL_H_

#include <assert.h>
#include <string>
#include <map>
#include <list>
#include <iterator>
using namespace std;

enum HttpMethodType{
	HTTP_UTIL_METHOD_NONE,
	HTTP_UTIL_METHOD_GET,
	HTTP_UTIL_METHOD_POST,
	HTTP_UTIL_METHOD_RESP
};


enum HttpParamType{
	HTTP_UTIL_PARAM_ALL,			//所有类型的参数
	HTTP_UTIL_PARAM_HEADPARAM,		//只获取HEADPARAM
	HTTP_UTIL_PARAM_CONTENT			//只获取CONTENT
};

/////////////////////////////////////////////////////////////////////////////
//构造http回应包
class CHttpResponseMaker
{
public:
	CHttpResponseMaker(){}
	virtual ~CHttpResponseMaker(){}
public:
	static int		make(const char* szContent, int nContentLen, char* szBuffer, int nBufferSize, const char* content_type);
	static void		make_string(const string& strContent, string& strResp, string& content_type, string& add_head);
	static void		make_404_error(string& strResp);
	static void		make_302_error(const string& strLocation, const string& strMoveTo, string& strResp);
protected:
private:
};

/////////////////////////////////////////////////////////////////////////////
//构造参数字符串
class CHttpParamStringMaker
{
public:
	CHttpParamStringMaker(){}
	virtual ~CHttpParamStringMaker(){}

public:
	void	add_param(const string& strKey, const string& strValue);
	void	add_param(const string& strKey, int nValue);
	void	set_paramlines(const string& strLines);
	string  get_params();
protected:
private:
	typedef struct
	{
		string strKey;
		string strValue;
	}HttpGetMakerParam;
	list<HttpGetMakerParam>	m_params;
	string m_strParamLines;
};

/////////////////////////////////////////////////////////////////////////////
//构造请求包
class CHttpMaker	: public CHttpParamStringMaker
{
public:
	CHttpMaker(){}
	virtual ~CHttpMaker(){}

public:
	int		make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize);
	void	make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest);

public:
	int		GET_make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize);
	void	GET_make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest);

	int		POST_make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize);
	void	POST_make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest);
};

/////////////////////////////////////////////////////////////////////////////
//构造GET请求包
class CHttpGetMaker	: public CHttpMaker
{
public:
	CHttpGetMaker(){}
	virtual ~CHttpGetMaker(){}

public:
	int make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize)
	{
		return GET_make(strHost, nPort, strUri, szBuffer, nBufferSize);
	}
	void make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest)
	{
		GET_make_string(strHost, nPort, strUri, strRequest);
	}
};

/////////////////////////////////////////////////////////////////////////////
//构造POST请求包
class CHttpPostMaker	: public CHttpMaker
{
public:
	CHttpPostMaker(){}
	virtual ~CHttpPostMaker(){}

public:
	int make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize)
	{
		return POST_make(strHost, nPort, strUri, szBuffer, nBufferSize);
	}
	void make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest)
	{
		POST_make_string(strHost, nPort, strUri, strRequest);
	}
};

/////////////////////////////////////////////////////////////////////////////
//分析Http的数据包长度，支持GET\POST\RESP
class CHttpLengthAnaly
{
public:
	CHttpLengthAnaly(){}
	virtual ~CHttpLengthAnaly(){}
public:
	//获取长度，错误数据返回-1，数据不完整返回0，接收完全返回>0
	static int		get_length(const char* szData, int nDataLen);

	//获取长度，错误数据返回-1，数据不完整返回0，接收完全返回>0
	//nContentPos返回内容区域的位置
	static int		get_length_ex(const char* szData, int nDataLen, int& nContentPos, int& nContentLen);
};

/////////////////////////////////////////////////////////////////////////////
//解析参数
//说明：用于解析如aaa=123&bbb=321&ccc=888之类的字符串
class CHttpParamParser
{
public:
	CHttpParamParser(const char* szHttpParam = NULL, int nLen = 0);
	virtual ~CHttpParamParser(){}

public:
	bool parse(const char* szHttpParam, int nLen);
	string get_param(const char* szKey);
	int get_param_int(const char* szKey);

private:
	map<string, string> m_mapValues;
};

/////////////////////////////////////////////////////////////////////////////
//分离字符串工具
class CInsertTempZero
{
public:
	CInsertTempZero(char* pSrc)
	{
		m_szOld = *pSrc;
		m_pSrc = pSrc;
		*m_pSrc = 0;
	}
	virtual ~CInsertTempZero()
	{
		*m_pSrc = m_szOld;
	}
protected:
private:
	char* m_pSrc;
	char  m_szOld;
};

/////////////////////////////////////////////////////////////////////////////
//HTTP协议解析器
class CHttpParser
{
public:
	CHttpParser(const char* szHttpReq = NULL, int nDataLen = 0, int nExtraParamType = HTTP_UTIL_PARAM_ALL);

	virtual ~CHttpParser(){}

public:

    static string UrlDecode(const std::string& szToDecode);

	int parse_head(const char* szHttpReq, int nDataLen, int nExtraParamType = HTTP_UTIL_PARAM_ALL);
    void parse_form_body();

    int getContentLen();

	//获取各个头域参数值
	string get_head_field(const string& strFieldName);

	//获取cookie
	string get_cookie();

	//获取参数项的数值
	string get_param(const char* szKey);
	int get_param_int(const char* szKey);

	//获取参数字符串，如 "aaa=123&bbb=321&ccc=888"
	string		get_param_string();

	//获取uri内容，如 "/update/mytest"
	string		get_uri();

	//获取object，比如http://127/aa/bb/cc?fff=999 中的cc
	string		get_object();

	//获取类型
	int			get_http_method();

protected:
	//解析头域
	bool parseField(const char* szHttpReq, int nTotalLen);
	//解析第一行，提取method和headparam
	bool parseFirstLine();
	//解析method
	bool parseMethod(const char* szFirstLine, const char* szMethod, int nMethodType);

private:
	map<string, string>	m_mapFields;
	char				m_szFirstLine[4096];
	const char*			m_pszContent;
	int					m_nContentLen;
	int					m_nHttpMethod;

	const char*			m_pszUri;
	const char*			m_pszActParam;

	CHttpParamParser	m_paramParser;		//参数解析器

	int					m_nExtraParamType;
};

#endif

