/***************************************************************************
 *
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @file http_util.cpp
 * @hemingzhe02@baidu.com
 * @date 2013/03/19
 * @brief
 *
 **/

#include "http_util.h"
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int CHttpResponseMaker::make(const char* szContent, int nContentLen, char* szBuffer, int nBufferSize, const char* content_type)
{
	int nRealContentLen = 0;
	if(nContentLen > 0)
		nRealContentLen = nContentLen;

	sprintf(szBuffer, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %s;charset=utf-8\r\nConnection: Keep-Alive\r\n\r\n",
		nRealContentLen, content_type);
	int nHeadLen = strlen(szBuffer);
	if(nContentLen > 0)
	{
		memcpy(szBuffer+nHeadLen, szContent, nContentLen);
		szBuffer[nHeadLen + nContentLen] = 0;
	}
	return strlen(szBuffer);
}

void CHttpResponseMaker::make_string(const string& strContent, string& strResp, string& content_type, string& add_head)
{
    strResp.clear();
    std::ostringstream s;
    s << strContent.size();
    strResp = "HTTP/1.1 200 OK\r\nContent-Length: "+s.str()+"\r\nContent-Type: "+content_type+";charset=utf-8\r\n"+add_head+"\r\n"+strContent;
}

void CHttpResponseMaker::make_404_error(string& strResp)
{
	string strContent;
	strContent += "<html>\r\n";
	strContent += "<head><title>404 Not Found</title></head>\r\n";
	strContent += "<body bgcolor=\"white\">\r\n";
	strContent += "<center><h1>404 Not Found</h1></center>\r\n";
	strContent += "<hr><center>http_util</center>\r\n";
	strContent += "</body>\r\n";
	strContent += "</html>\r\n";
	strContent += "<!-- The padding to disable MSIE's friendly error page -->";
	strContent += "<!-- The padding to disable MSIE's friendly error page -->";
	strContent += "<!-- The padding to disable MSIE's friendly error page -->";
	strContent += "<!-- The padding to disable MSIE's friendly error page -->";
	strContent += "<!-- The padding to disable MSIE's friendly error page -->";
	strContent += "<!-- The padding to disable MSIE's friendly error page -->";

	char szTemp[100];
	sprintf(szTemp, "Content-Length: %d\r\n", (int)strContent.size());

	strResp = "HTTP/1.1 404 Not Found\r\n";
	strResp += "Server: http_util\r\n";
	strResp += "Content-Type: text/html; charset=UTF-8\r\n";
	strResp += szTemp;
	strResp += "Connection: keep-alive\r\n";
	strResp += "\r\n";
	strResp += strContent;
}

void CHttpResponseMaker::make_302_error(const string& strLocation, const string& strMoveTo, string& strResp)
{
	string strContent;
	strContent += "<html><head><title>Object moved</title></head><body>\r\n";
	strContent += "<h2>Object moved to <a href=\"";
	strContent += strMoveTo;
	strContent += "\">here</a>.</h2>\r\n";
	strContent += "</body></html>\r\n";

	char szTemp[100];
	sprintf(szTemp, "Content-Length: %d\r\n", (int)strContent.size());

	strResp = "HTTP/1.1 302 Found\r\n";
	strResp += "Server: http_util\r\n";
	strResp += "Content-Type: text/html; charset=UTF-8\r\n";
	strResp += szTemp;
	strResp += "Connection: keep-alive\r\n";
	strResp += "Location: ";
	strResp += strLocation + "\r\n";
	strResp += "\r\n";
	strResp += strContent;
}

void CHttpParamStringMaker::add_param(const string& strKey, const string& strValue)
{
	HttpGetMakerParam param;
	param.strKey = strKey;
	param.strValue = strValue;
	m_params.push_back(param);
}

void CHttpParamStringMaker::add_param(const string& strKey, int nValue)
{
	char szValue[100];
	sprintf(szValue, "%u", nValue);
	add_param(strKey, szValue);
}

void CHttpParamStringMaker::set_paramlines(const string& strParamLines)
{
	m_strParamLines = strParamLines;
}

string CHttpParamStringMaker::get_params()
{
	if(!m_strParamLines.empty())
		return m_strParamLines;

	char szParams[4096] = {0};
	list<HttpGetMakerParam>::iterator it;
	for(it = m_params.begin(); it != m_params.end(); it++)
	{
		HttpGetMakerParam param = *it;

		char szTmp[1024];
		sprintf(szTmp, "%s=%s&", param.strKey.c_str(), param.strValue.c_str());
		strcat(szParams, szTmp);
	}
	if(szParams[strlen(szParams) -1] == '&')
		szParams[strlen(szParams) -1] = 0;
	return szParams;
}

int CHttpMaker::make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize)
{
	return GET_make(strHost, nPort, strUri, szBuffer, nBufferSize);
}

void CHttpMaker::make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest)
{
	GET_make_string(strHost, nPort, strUri, strRequest);
}

int CHttpMaker::GET_make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize)
{
	char szPort[100] = {0};
	if(nPort != 80)
	{
		sprintf(szPort, ":%d", nPort);
	}
	string strParams = get_params();
	sprintf(szBuffer, "GET %s?%s HTTP/1.1\r\nHost: %s%s\r\nConnection: Keep-Alive\r\nAccept: */*\r\nUser-Agent: http_util\r\n\r\n",
		strUri.c_str(), strParams.c_str(), strHost.c_str(), szPort);
	return strlen(szBuffer);
}

void CHttpMaker::GET_make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest)
{
	char szBuffer[4096];
	GET_make(strHost, nPort, strUri, szBuffer, sizeof(szBuffer));
	strRequest = szBuffer;
}

int CHttpMaker::POST_make(const string& strHost, unsigned short nPort, const string& strUri, char* szBuffer, int nBufferSize)
{
	char szPort[100] = {0};
	if(nPort != 80)
	{
		sprintf(szPort, ":%d", nPort);
	}
	string strParams = get_params();
	string strContentType = "application/x-www-form-urlencoded; charset=UTF-8";//二进制用application/octet-stream
	sprintf(szBuffer, "POST %s HTTP/1.1\r\nHost: %s%s\r\nContent-type: %s\r\nContent-Length: %d\r\nConnection: Keep-Alive\r\n\r\n%s",
		strUri.c_str(), strHost.c_str(), szPort, strContentType.c_str(), (int)strParams.size(), strParams.c_str());
	return strlen(szBuffer);
}

void CHttpMaker::POST_make_string(const string& strHost, unsigned short nPort, const string& strUri, string& strRequest)
{
	char szBuffer[4096];
	POST_make(strHost, nPort, strUri, szBuffer, sizeof(szBuffer));
	strRequest = szBuffer;
}

int CHttpLengthAnaly::get_length(const char* szData, int nDataLen)
{
	int nContentPos;
	int nContentLen;
	return get_length_ex(szData, nDataLen, nContentPos, nContentLen);
}

int CHttpLengthAnaly::get_length_ex(const char* szData, int nDataLen, int& nContentPos, int& nContentLen)
{
	bool bGetType = false;
	bool bPostType = false;
	bool bRespType = false;
	if(memcmp(szData, "GET ", 4) == 0)
	{
		bGetType = true;
	}
	else if(memcmp(szData, "POST ", 5) == 0)
	{
		bPostType = true;
	}
	else if(memcmp(szData, "HTTP/", 5) == 0)
	{
		bRespType = true;
	}
	else
	{
		return -1;
	}

	//根据http头结束符判断
	char* pHeadEnd = strstr((char*)szData, "\r\n\r\n");
	if(!pHeadEnd)
		return 0;

	nContentPos = 0;
	nContentLen = 0;
	int nHeadLen = pHeadEnd+4-szData;
	if(bPostType || bRespType)
	{
		char* pContentLen = strstr((char*)szData, "Content-Length: ");
		if(pContentLen)
		{
			pContentLen += strlen("Content-Length: ");
			char* pContentLenEnd = strstr(pContentLen, "\r\n");
			if(pContentLenEnd)
			{
				char szTmp[30];
				memset(szTmp, 0, sizeof(szTmp));
				memcpy(szTmp, pContentLen, pContentLenEnd-pContentLen);
				nContentLen = atoi(szTmp);

				//内容的相对位置
				nContentPos = pHeadEnd-szData + strlen("\r\n\r\n");
			}
		}
	}
	if(nDataLen < nHeadLen+nContentLen)
		return 0;

	return nHeadLen+nContentLen;
}

CHttpParamParser::CHttpParamParser(const char* szHttpParam, int nLen)
{
    if(szHttpParam && nLen > 0)
    {
        parse(szHttpParam, nLen);
    }
}

bool CHttpParamParser::parse(const char* szHttpParam, int nLen)
{
	//复制到缓冲
	char* szBuffer = new (std::nothrow)  char[4096+nLen];
    if (szBuffer == NULL)
    {
        return false;
    }
	memcpy(szBuffer, szHttpParam, nLen);
	szBuffer[nLen] = 0;
	if(szBuffer[strlen(szBuffer)-1] != '&')
		strcat(szBuffer, "&");

	//处理参数
	char* szParam = szBuffer;
	while(1)
	{
		if(szParam[0] == '\0')
			break;

		if(!((szParam[0] >='a' && szParam[0] <='z') || (szParam[0] >='A' && szParam[0] <='Z')))
		{
			szParam ++;
			continue;
		}

		char* szValue = strchr(szParam, '=');
		if(!szValue)
			break;

		char* szSplit = strchr(szParam, '&');
		if(!szSplit)
			break;

		string strKey;
		string strValue;

		//char szTmp[1024]; bug!!!
		char* szTmp = new (std::nothrow) char [4+nLen];
        if (szTmp == NULL)
        {
            delete []szBuffer;
            return false;
        }
		//提取key
		memset(szTmp, 0,4+nLen);
        if ( szValue-szParam > 0)
        {
		    memcpy(szTmp, szParam, szValue-szParam);
		    strKey = szTmp;
        }

		//提取value
		memset(szTmp, 0, 4+nLen);
        if ( szSplit-szValue-1 > 0)
        {
		    memcpy(szTmp, szValue+1, szSplit-szValue-1);
		    strValue = szTmp;
        }
		delete []szTmp;
        if ( !strKey.empty())
        {
		    m_mapValues.insert(std::make_pair(strKey, strValue));
        }
		szParam = szSplit + 1;

	}

	delete []szBuffer;
	return true;
}

string CHttpParamParser::get_param(const char* szKey)
{
	map<string, string>::iterator it = m_mapValues.find(szKey);
	if(it != m_mapValues.end())
	{
		return it->second;
	}
	else
	{
		return "";
	}
}

int CHttpParamParser::get_param_int(const char* szKey)
{
	return atoi(get_param(szKey).c_str());
}

CHttpParser::CHttpParser(const char* szHttpReq, int nDataLen, int nExtraParamType)
: m_pszContent(NULL)
, m_nContentLen(0)
, m_nHttpMethod(HTTP_UTIL_METHOD_NONE)
, m_pszUri(NULL)
, m_pszActParam(NULL)
    , m_nExtraParamType(HTTP_UTIL_PARAM_ALL)
{
    if(szHttpReq != NULL)
    {
        if(nDataLen > 0)
        {
            parse_head(szHttpReq, nDataLen, nExtraParamType);
        }
        else
        {
            assert(0);
        }
    }
    m_szFirstLine[0] = 0;
}

string CHttpParser::UrlDecode(const std::string& szToDecode)
{
	std::string result;
	int hex = 0;
	for (size_t i = 0; i < szToDecode.length(); ++i)
	{
		switch (szToDecode[i])
		{
		case '+':
			result += ' ';
			break;
		case '%':
			if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
			{
				std::string hexStr = szToDecode.substr(i + 1, 2);
				hex = strtol(hexStr.c_str(), 0, 16);
				if (!((hex >= 48 && hex <= 57) || //0-9
					(hex >=97 && hex <= 122) || //a-z
					(hex >=65 && hex <= 90) || //A-Z
					hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
					|| hex == 0x2a || hex == 0x2b|| hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
					|| hex == 0x3A || hex == 0x3B|| hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
					))
				{
					result += char(hex);
					i += 2;
				}
				else result += '%';
			}else {
				result += '%';
			}
			break;
		default:
			result += szToDecode[i];
			break;
		}
	}
	return result;
}

int CHttpParser::parse_head(const char* szHttpReq, int nDataLen, int nExtraParamType)
{
	int nContentPos = 0;

	m_nExtraParamType = nExtraParamType;

	//判断是否接收完成
	int nTotalLen = CHttpLengthAnaly::get_length_ex(szHttpReq, nDataLen, nContentPos, m_nContentLen);
	if(nTotalLen <= 0)
		return nTotalLen;

	//分离头域字段
	if(!parseField(szHttpReq, nTotalLen))
		return -1;

	//分析第一行的信息
	if(!parseFirstLine())
		return -1;

	//分析参数详细信息
	if(m_pszActParam)
	{
		if(nExtraParamType == HTTP_UTIL_PARAM_ALL || nExtraParamType == HTTP_UTIL_PARAM_HEADPARAM)
		{
			m_paramParser.parse(m_pszActParam, strlen(m_pszActParam));
		}
	}

	return nTotalLen;
}

void CHttpParser::parse_form_body()
{
	if(m_pszContent)
	{
		m_paramParser.parse(m_pszContent, m_nContentLen);
	}
}

int CHttpParser::getContentLen()
{
    return m_nContentLen;
}

bool CHttpParser::parseField(const char* szHttpReq, int nTotalLen)
{
	//获取第一行
	char* ptr = strstr((char*)szHttpReq, "\r\n");
	if(!ptr)
		return false;


	CInsertTempZero z1(ptr);
	strncpy(m_szFirstLine, szHttpReq, sizeof(m_szFirstLine)-1);
	ptr += strlen("\r\n");

	while(1)
	{
		//防止越界
		if(ptr > szHttpReq + nTotalLen - 4)	//4 == strlen("\r\n\r\n")
			break;

		//是否到了文本区域末尾
		if(memcmp(ptr, "\r\n", 2) == 0)
			break;

		//行末
		char* pLineEnd = strstr(ptr, "\r\n");
		if(!pLineEnd)
			break;

		CInsertTempZero zLineEnd(pLineEnd);

		//对一行进行分析
		char* p = strstr(ptr, ": ");
		if(!p)
		{
			break;
		}

		//提取头域名和值
		CInsertTempZero zp(p);
		string strFieldName = ptr;

		p += strlen(": ");
		string strValue = p;

		m_mapFields.insert(std::make_pair(strFieldName, strValue));

		ptr = pLineEnd + strlen("\r\n");
	}

	ptr += strlen("\r\n");

	if(ptr < szHttpReq + nTotalLen)
	{
		m_pszContent = ptr;
	}
	return true;
}

bool CHttpParser::parseFirstLine()
{
	if(strlen(m_szFirstLine) < 10)		//第一行不能小于10个字符
		return false;

	char* pBegin = NULL;
	//分析method
	while(1)
	{
		if(parseMethod(m_szFirstLine, "GET ", HTTP_UTIL_METHOD_GET))
		{
			pBegin = m_szFirstLine + 4;
			break;
		}
		if(parseMethod(m_szFirstLine, "POST ", HTTP_UTIL_METHOD_POST))
		{
			pBegin = m_szFirstLine + 5;
			break;
		}
		if(parseMethod(m_szFirstLine, "HTTP/", HTTP_UTIL_METHOD_RESP))
		{
			pBegin = m_szFirstLine + 5;
			break;
		}
	}
	//没找到可支持的method则返回
	if(m_nHttpMethod == HTTP_UTIL_METHOD_NONE)
		return false;

	//提取uri和动作参数
	char* szParam = strchr(pBegin, '?');
	if(szParam)
	{
		*szParam = 0;
		m_pszActParam = szParam+1;
	}
	m_pszUri = pBegin;
	return true;
}

bool CHttpParser::parseMethod(const char* szFirstLine, const char* szMethod, int nMethodType)
{
	int len = strlen(szMethod);
	if(memcmp(m_szFirstLine, szMethod, len) == 0)
	{
		char* pEnd = strstr(m_szFirstLine, " HTTP");
		if(pEnd)
		{
			*pEnd = 0;
			m_nHttpMethod = nMethodType;
			return true;
		}
	}
	return false;
}

string CHttpParser::get_head_field(const string& strFieldName)
{
	map<string, string>::iterator it = m_mapFields.find(strFieldName);
	if(it != m_mapFields.end())
	{
		return it->second;
	}
	return "";
}
string CHttpParser::get_cookie()
{
	return get_head_field("Cookie");
}

string CHttpParser::get_param(const char* szKey)
{
	return m_paramParser.get_param(szKey);
}

int CHttpParser::get_param_int(const char* szKey)
{
	return m_paramParser.get_param_int(szKey);
}

string CHttpParser::get_param_string()
{
	if(m_pszActParam && (m_nExtraParamType == HTTP_UTIL_PARAM_ALL || m_nExtraParamType == HTTP_UTIL_PARAM_HEADPARAM))
		return m_pszActParam;

	if(m_pszContent && (m_nExtraParamType == HTTP_UTIL_PARAM_ALL || m_nExtraParamType == HTTP_UTIL_PARAM_CONTENT))
		return m_pszContent;

	return "";
}

string CHttpParser::get_uri()
{
	if(!m_pszUri)
		return "";

	return m_pszUri;
}

string CHttpParser::get_object()
{
	if(!m_pszUri)
		return "";

	char* ptr = strrchr((char*)m_pszUri, '/');
	if(!ptr)
		return "";

	return ptr+1;
}

int CHttpParser::get_http_method()
{
	return m_nHttpMethod;
}
