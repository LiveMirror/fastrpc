/// Jack
#include "csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

typedef long int                i32;
typedef unsigned long           u32;

int GetParamFromString(std::string Str, std::vector<i32>& IntVec, char Delim=',') {
    char* p = strtok((char*)Str.c_str(), &Delim);
    while (p)
    {
        IntVec.push_back(atoi(p));
        p = strtok(NULL, &Delim);
    }
    return IntVec.size();
}

int GetParamFromString(std::string Str, std::vector<float>& FloatVec, char Delim=',') {
    char* p = strtok((char*)Str.c_str(), &Delim);
    while (p)
    {
        FloatVec.push_back(atof(p));
        p = strtok(NULL, &Delim);
    }
    return FloatVec.size();
}

int GetParamFromString(std::string Str, std::vector<u32>& uiIntVec, char Delim=',') {
    char* p = strtok((char*)Str.c_str(), &Delim);
    while (p)
    {
        uiIntVec.push_back(strtoul(p, NULL, 10));
        p = strtok(NULL, &Delim);
    }
    return uiIntVec.size();
}

int GetParamFromString(std::string Str, std::vector<std::string>& StringVec, char Delim=',') {
    std::string buffer;
    for (size_t i = 0;i<Str.size();i++)
    {
        char c = Str[i];
        if (c == Delim)
        {
            StringVec.push_back(buffer);
            //CCLOG("GetParamFromString: %s \n",buffer.c_str());
            buffer.clear();
        }
        else
        {
            buffer.push_back(c);
        }
    }
    StringVec.push_back(buffer);
    return StringVec.size();
  }

std::string GetParamFromString( std::string Str,char Delim /*= ','*/,int index /*= 0*/ )
{
      std::string buffer;
      int n = 0;
      for (size_t i = 0; i<Str.size(); i++)
      {
          char c = Str[i];
          if (c == Delim)
          {
              if (index == n++)
              {
                  return buffer;
              }
              buffer.clear();
          }
          else
          {
              buffer.push_back(c);
          }
      }
      if(index == n++)
      {
          return buffer;
      }
      else
      {
          buffer.clear();
          return buffer;
      }

}

namespace ipcs_common {

CsvParser::CsvParser(const char* path) {
    if (path) {
        LoadCSV(path);
    }
}

CsvParser::~CsvParser() {
}


/*
=====================================

CsvParser::LoadCSV

=====================================
*/
bool CsvParser::LoadCSV(const char* csvfile)
{
    std::string documentPath = std::string(csvfile);

    unsigned long dwsize = 0;
    FILE* fp = fopen(csvfile, "rb");
    fseek(fp, 0, SEEK_END);
    dwsize = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    unsigned char* filebuffer = new unsigned char[dwsize];
    fread(filebuffer, 1, dwsize, fp);
    fclose(fp);

    if (filebuffer&&dwsize) {
        std::map<unsigned int , std::string> StringMap;
        bool    bquote=false;
        unsigned int        uiIndex = 1;
        std::string elembuff;
        for (char* p = (char*)filebuffer; p<(char*)filebuffer+dwsize; p++) {
            if (*p=='"'){bquote=!bquote;}
            if (!bquote && ((*p)== '\n'))
            {
                StringMap[uiIndex++] = elembuff;
                elembuff.clear();
            }
            else
            {
                elembuff.push_back(*p);
            }
        }
        delete[] filebuffer;
        for (std::map<unsigned int , std::string>::iterator iter = StringMap.begin(); iter != StringMap.end(); ++iter)
        {
            std::vector<std::string> StringVec;
            std::map<unsigned int , std::string> l_StringMap;
            GetParamFromString(iter->second, StringVec);
            for (size_t i = 0; i < StringVec.size(); ++i)
            {
                l_StringMap[i+1] = StringVec[i];
            }
            m_IntKeyMap[iter->first] = l_StringMap;
        }
        m_CSVName = documentPath;
        m_StringKeyMap = this->GetStringKeyMap();
        return true;
    }
    return false;
}

int CsvParser::GetInt( string usline, string usrow, int& iValue )
{
    string str ;
    bool br = GetString(usline,usrow,str);
    iValue = atol(str.c_str());
    return br ? iValue : 0;
}

long int CsvParser::GetInt( string usline, string usrow, long int& iValue )
{
    string str ;
    bool br = GetString(usline,usrow,str);
    iValue = atol(str.c_str());
    return br ? iValue : 0;
}

short int CsvParser::GetInt( string usline, string usrow, short int& iValue )
{
    string str ;
    bool br = GetString(usline,usrow,str);
    iValue = atol(str.c_str());
    return br ? iValue : 0;
}

unsigned int CsvParser::GetInt( string usline, string usrow, unsigned int& iValue )
{
    string str ;
    bool br = GetString(usline,usrow,str);
    iValue = atol(str.c_str());
    return br ? iValue : 0;
}

unsigned long int CsvParser::GetInt( string usline, string usrow, unsigned long int& iValue )
{
    string str ;
    bool br = GetString(usline,usrow,str);
    iValue = atol(str.c_str());
    return br ? iValue : 0;
}

unsigned short int CsvParser::GetInt( string usline, string usrow, unsigned short int& iValue )
{
    string str ;
    bool br = GetString(usline,usrow,str);
    iValue = atol(str.c_str());
    return br ? iValue : 0;
}

bool CsvParser::GetBool(string usline,string usrow,bool & bValue)
{
    string str ;
    bool br = GetString(usline,usrow,str);
    long int iValue = atol(str.c_str());
    bValue = (iValue != 0);
    return br;
}
/*
=====================================

CsvParser::GetFloat

=====================================
*/
float CsvParser::GetFloat(unsigned int  uiLine, unsigned int  uiRow, float& fValue)
{
    std::string* pKey = GetString(uiLine, uiRow);
    if (pKey)
    {
        fValue = atof(pKey->c_str());
        return fValue;
    }
    else
    {
        return 0.0;
    }
}

float CsvParser::GetFloat( string uiLine, string uiRow, float& fValue )
{
    string str ;
    bool br = GetString(uiLine,uiRow,str);
    fValue = atof(str.c_str());
    return br ? fValue : 0.0;
}

/*
=====================================

CsvParser::GetString

=====================================
*/
std::string* CsvParser::GetString(unsigned int  uiLine, unsigned int  uiRow)
{
    std::map<unsigned int , std::map<unsigned int , std::string> >::iterator iterLine = m_IntKeyMap.find(uiLine);
    if (iterLine != m_IntKeyMap.end())
    {

        std::map<unsigned int , std::string>& rStringMap = iterLine->second;
        std::map<unsigned int , std::string>::iterator iterRow = rStringMap.find(uiRow);
        if (iterRow != rStringMap.end())
        {
            iterRow->second.c_str();
            return &iterRow->second;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

bool CsvParser::GetString( string uiLine, string uiRow, string& sValue )
{
    bool br = false;

    std::map<string,map<string,string> > ::iterator it = m_StringKeyMap.find(uiLine);
    if (it != m_StringKeyMap.end())
    {
        map<string,string>::iterator jt  = (*it).second.find(uiRow);
        if (jt != (*it).second.end())
        {
            string var = (*jt).second;
            br = true;
            sValue = var;
        }
    }
    if (!br)
    {
    }

    return br;
}

/*
=====================================

CsvParser::SetNumber

=====================================
*/
bool CsvParser::SetNumber(unsigned int  uiLine, unsigned int  uiRow, int iValue)
{
    std::string* pKey = GetString(uiLine, uiRow);
    if (pKey)
    {
        char buffer[128];
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%d", iValue);
        pKey->clear();
        *pKey = buffer;
        return true;
    }
    else
    {
        return false;
    }
}

/*
=====================================

CsvParser::SetNumber

=====================================
*/
bool CsvParser::SetNumber(unsigned int  uiLine, unsigned int  uiRow, float fValue)
{
    std::string* pKey = GetString(uiLine, uiRow);
    if (pKey)
    {
        char buffer[100];
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%f", fValue);
        pKey->clear();
        *pKey = buffer;
        return true;
    }
    else
    {
        return false;
    }
}

/*
=====================================

CsvParser::SetString

=====================================
*/
bool CsvParser::SetString(unsigned int  uiLine, unsigned int  uiRow, const char* pStr)
{
    std::string* pKey = GetString(uiLine, uiRow);
    if (pKey)
    {
        pKey->clear();
        *pKey = pStr;
        return true;
    }
    else
    {
        return false;
    }
}

/*
=====================================

CsvParser::SaveCSV

=====================================
*/
bool CsvParser::SaveCSV(const char* path)
{
    if (path != NULL)
    {
        m_CSVName = path;
    }

    FILE* pfile = fopen(m_CSVName.c_str(), "w");
    if (pfile)
    {
        std::map<unsigned int , std::map<unsigned int , std::string> >::iterator iter = m_IntKeyMap.begin();
        for (; iter != m_IntKeyMap.end(); ++iter)
        {
            std::map<unsigned int , std::string>& rStringMap = iter->second;
            std::map<unsigned int , std::string>::iterator it = rStringMap.begin();
            for (; it != rStringMap.end(); ++it)
            {
                std::string key = it->second;
                key += ',';
                fwrite(key.c_str(), 1, key.size(), pfile);
            }
            char Delim = '\n';
            fwrite(&Delim, 1, 1, pfile);
        }
        fclose(pfile);
    }
    else
    {
        return false;
    }

    return true;
}

/*
=====================================

CsvParser::GetStringKeyMap
返回表,以第一列的值作为表的行索引,以第一行的值作为后序各行的列索引
=====================================
*/
std::map<string,map<string,string> > CsvParser::GetStringKeyMap()
{

    std::map<unsigned int, std::map<unsigned int, std::string> >::iterator StartLine = m_IntKeyMap.find(1);

    std::map<string, std::map<string,string> > kmapText;
    if (StartLine != m_IntKeyMap.end())
    {
        std::map<unsigned int, std::string>     start =  m_IntKeyMap[1];//(*StartLine).second;//

        for (std::map<unsigned int, std::map<unsigned  int, std::string> > ::iterator jt = m_IntKeyMap.begin();jt != m_IntKeyMap.end();jt ++)
        {
            std::map<string,string>             linetext;
            std::map<unsigned int, std::string> rows = (*jt).second;
            int k = 1 ;
            for (std::map<unsigned int, std::string>::iterator it = start.begin();it != start.end();it ++)
            {
                string str = (*it).second;
                string value    = rows[k++];
                linetext[str]   = value;
            }
            kmapText[rows[1]] = linetext;
        }
    }
    return kmapText;
}

std::map<unsigned int , std::map<unsigned int , std::string> >& CsvParser::GetCSVMap()
{
    return m_IntKeyMap;
}

std::string CsvParser::GetParam( string uiLine, string uiRow,  int index,  char Delim  )
{
    string buffer ;
    this->GetString(uiLine,uiRow,buffer);
    string strret = GetParamFromString(buffer,Delim,index);
    return strret;
}

void CsvParser::GetAllIID(std::vector<std::string>* array) {
    std::map<unsigned int , std::map<unsigned int , std::string> >& rowsMap = this->GetCSVMap();

    size_t rows = rowsMap.size();

    std::map<unsigned int , std::map<unsigned int , std::string> >::iterator it = rowsMap.begin();

    for (size_t i = 3; i <= rows; ++i) {
        std::map<unsigned int, std::string>& valueMap = rowsMap[i];
        array->push_back(valueMap[1]);
    }
}

}
