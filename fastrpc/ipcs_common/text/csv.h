// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_TEXT_H
#define IPCS_COMMON_TEXT_H
#pragma once

#include <map>
#include <string>
#include <vector>

namespace ipcs_common {

using namespace std;

class CsvParser {

public:
    virtual ~CsvParser();
    CsvParser(const char* path = NULL);


    bool LoadCSV(const char* path);
    bool SaveCSV(const char* path = NULL);

    float GetFloat(unsigned int  uiLine, unsigned int  uiRow, float& fValue);
    bool GetString(string uiLine, string  uiRow, string& sValue);
    string GetParam(string uiLine, string  uiRow,  int index = 0,char Delim ='|');
    int GetInt(string usline, string  usrow, int& iValue);
    long int GetInt(string usline, string  usrow, long int& iValue);
    short int GetInt(string usline, string  usrow, short int& iValue);
    unsigned int GetInt(string usline, string  usrow, unsigned int& iValue);
    unsigned long int GetInt(string usline, string  usrow, unsigned long int& iValue);
    unsigned short int GetInt(string usline, string  usrow, unsigned short int& iValue);

    float GetFloat(string  uiLine, string  uiRow, float& fValue);
    bool GetBool(string usline,string usrow,bool & bValue);
    std::string* GetString(unsigned int  uiLine, unsigned int  uiRow);
    bool SetNumber(unsigned int  uiLine, unsigned int  uiRow, int iValue);
    bool SetNumber(unsigned int  uiLine, unsigned int  uiRow, float fValue);
    bool SetString(unsigned int  uiLine, unsigned int  uiRow, const char* pStr);

    // 返回以行号列号为索引的表
    std::map<unsigned int , std::map<unsigned int , std::string> >& GetCSVMap();
    // 返回表,以第一列的值作为表的行索引,以第一行的值作为后序各行的列索引
    std::map<string,map<string,string> > GetStringKeyMap();

public:
    void GetAllIID(std::vector<std::string>* array);

protected:
    std::string m_CSVName;
    std::map<unsigned int , std::map<unsigned int , std::string> > m_IntKeyMap;
    std::map<string , std::map<string, std::string> >              m_StringKeyMap;
    //std::map<long int , std::map<string, std::string> >              m_IntStringMap;
};

}

#endif
