/// Jack

#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "csv_parser.h"


bool getLine(std::ifstream &is, std::string &str) {
    return std::getline(is, str, '\n');
}

int replace(std::string& src_str, const std::string& old_str, const std::string& new_str) {
    int count   = 0;
    int old_str_len = int(old_str.length());
    int new_str_len = int(new_str.length());
    std::string::size_type pos = 0;
    while((pos=int(src_str.find(old_str,pos)))!=std::string::npos)
    {
        src_str.replace(pos,old_str_len,new_str);
        pos += new_str_len;
        ++count;
    }
    return count;
}

bool writeLine(std::ofstream& fout, std::vector<std::string>& info) {
    std::vector<std::string>::iterator it;
    for (it= info.begin(); it != info.end(); ++it)
    {
        std::string str = *it;
        replace(str, "\"", "\"\"");
        fout << '"' << str << '"';
        if (it + 1 != info.end())
        {
            fout << ',';
        }
    }
    fout << std::endl;
    return true;
}

namespace ipcs_common {

char CsvParser::PATH_SEPARATOR = '/';
std::string CsvParser::FIELD_SEP = ",";
/*
=================================
    CsvParser::swCSVFile
=================================
*/
CsvParser::CsvParser(bool hasDescibe, bool parse_data)
    : _hasDescribe(hasDescibe)
    , _do_parse_data(parse_data)
    , _currColumn(0)
    , _currRow(0) {
}

/*
=================================
    CsvParser::~swCSVFile
=================================
*/
CsvParser::~CsvParser() {
    _header.clear();
    _describe.clear();

    unsigned int i = 0;
    for (i=0; i<_data.size(); i++)
    {
        _data[i].clear();
    }
    _data.clear();
}

int CsvParser::GetAllIID(std::vector<std::string>* array) {
    int rows = GetRows();
    if (_hasDescribe && rows <= 2) return 0;

    unsigned int start_index = 0;
    if (_hasDescribe) {
        start_index = 2;
    }
    for (unsigned int r = start_index; r < (unsigned int)rows; ++r) {
        array->push_back(std::string(GetData(r, 0U)));
    }
    return 0;
}

/*
=================================
    CsvParser::load
=================================
*/
bool CsvParser::Load(const std::string& fileName)
{
    if (fileName.size() == 0)
    {
        return false;
    }
    std::string pathKey = fileName;
    unsigned long dwsize = 0;
    FILE* fp = fopen(pathKey.c_str(), "rb");
    if (fp == NULL) {
        abort();
    }

    fseek(fp, 0, SEEK_END);
    dwsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    unsigned char* filebuffer = new unsigned char[dwsize];
    bzero(filebuffer, dwsize);
    fread(filebuffer, 1, dwsize, fp);
    fclose(fp);

    if (filebuffer&&dwsize)
    {
        std::map<unsigned int , std::string> StringMap;
        bool    bquote=false;
        unsigned int        uiIndex = 0;
        std::string elembuff;
        _currRow = 0;
        for (char* p = (char*)filebuffer; p<(char*)filebuffer+dwsize; p++)
        {
            if (*p=='"')
            {
                bquote=!bquote;
            }
            if (!bquote && ((*p)== '\n'))
            {
                if (uiIndex == 0)
                {
                    split(elembuff, &_header);
                    for(unsigned int index = 0; index < _header.size(); index++)
                    {
                        _headerMap[_header[index]]=index;
                    }
                }
                else if(uiIndex == 1&&_hasDescribe)
                {
                    split(elembuff, &_describe);
                }
                else
                {
                    std::vector<std::string> field;
                    split(elembuff, &field);

                    _rowMap[field[0]] = _currRow;
                    _data.push_back(field);
                    _currRow++;
                }
                uiIndex++;
                elembuff.clear();
            }
            else
            {
                elembuff.push_back(*p);
            }
        }
    }

    delete[] filebuffer;
    return true;
}

bool CsvParser::Save(const std::string& fileName)
{
    if (fileName.size() == 0)
    {
        return false;
    }
    std::string pathKey = fileName;

    std::ofstream fout(pathKey.c_str(), std::ios_base::out);

    if (_hasDescribe)
    {
        writeLine(fout, _describe);
    }

    writeLine(fout, _header);

    unsigned int row;
    for (row = 0; row < _data.size(); row++)
    {
        writeLine(fout, _data[row]);
    }

    fout.close();

    return true;
}

void CsvParser::MakeFileHead(const std::string& fileHeader, const std::string& delimiter)
{
    size_t pos_begin = 0;
    size_t pos_end = 0;
    int index = 0;
    while (pos_end != std::string::npos)
    {
        pos_begin = fileHeader.find_first_not_of(delimiter, pos_end);
        if (pos_begin == std::string::npos) break;
        pos_end = fileHeader.find_first_of(delimiter, pos_begin);

        std::string strTmp(fileHeader, pos_begin, pos_end - pos_begin);

        if (!strTmp.empty())
        {
            _header.push_back(strTmp);
            _headerMap[strTmp] = index;
            index++;
        }
    }
}

/*
=================================
    CsvParser::split
=================================
*/
void CsvParser::split(const std::string& line, std::vector<std::string>* field)
{
    std::string fld;
    unsigned int i=0, j=0;
    if (line.length() == 0) {
        return ;
    }

    do {
        if (i < line.length() && line[i] == '"') {
            // skip quote
            j = advquoted(line, fld, ++i);
        } else {
            // skip plain
            j = advplain(line, fld, i);
        }
        field->push_back(fld);
        i = j + 1;
    } while (j < line.length());
}

/*
=================================
    swCSVParse::advquoted
=================================
*/
int CsvParser::advquoted(const std::string& s, std::string& fld, int i)
{
    unsigned int j = 0;

    fld = "";
    for (j = i; j < s.length(); j++)
    {
        if (s[j] == '"' && s[++j] != '"')
        {
            unsigned int k = s.find_first_of(FIELD_SEP, j);
            if (k > s.length()) {
                k = s.length();
            }
            for (k -= j; k-- > 0; ) {
                fld += s[j++];
            }
            break;
        }
        fld += s[j];
    }
    return j;
}

/*
=================================
    swCSVParse::advplain
=================================
*/
int CsvParser::advplain(const std::string& s, std::string& fld, int i)
{
    unsigned int j;

    // look for separator
    j = s.find_first_of(FIELD_SEP, i);

    // none found
    if (j > s.length())
        j = s.length();
    fld = std::string(s, i, j-i);

    return j;
}


int CsvParser::GetInt(unsigned int row, const char* column_name) {
    std::string value = std::string(this->GetData(row, column_name));
    if (value == "") {
        return 0;
    } else {
        return atoi(value.c_str());
    }
}

int CsvParser::GetInt(unsigned int row, unsigned int col) {
    std::string value = std::string(this->GetData(row, col));
    if (value == "") {
        return 0;
    } else {
        return atoi(value.c_str());
    }
}

std::vector<std::string> CsvParser::tokenize(const std::string hay,
                                             const std::string delim) {
    std::vector<std::string> tokens;
    if (hay == delim) {
        return tokens;
    }
    std::string::size_type start_pos = hay.find_first_not_of(delim);

    std::string::size_type last_delim_pos = hay.find_first_of(delim);
    if (last_delim_pos == std::string::npos) {
        tokens.push_back(hay);
        return tokens;
    }

    for (; start_pos != std::string::npos &&
           last_delim_pos != std::string::npos; ) {
        tokens.push_back(hay.substr(start_pos, last_delim_pos - start_pos));
        start_pos = hay.find_first_not_of(delim, last_delim_pos);
        last_delim_pos = hay.find_first_of(delim, start_pos);
        if (start_pos != std::string::npos && last_delim_pos == std::string::npos) {
            tokens.push_back(hay.substr(start_pos, std::string::npos));
        }
    }

    return tokens;
}

void CsvParser::GetMultiInts(unsigned int row, const char* column_name,
                             std::vector<int>* array, const std::string delim) {
    std::string value = std::string(this->GetData(row, column_name));
    std::vector<std::string> values = tokenize(value, delim);

    std::vector<std::string>::iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        array->push_back(atoi((*it).c_str()));
    }
}

void CsvParser::GetMultiInts(unsigned int row, unsigned int col,
                             std::vector<int>* array, const std::string delim) {

    std::string value = std::string(this->GetData(row, col));
    std::vector<std::string> values = tokenize(value, delim);

    std::vector<std::string>::iterator it;
    for (it = values.begin(); it != values.end(); ++it) {
        array->push_back(atoi((*it).c_str()));
    }
}

/*
=================================
    CsvParser::getData
=================================
*/
const char* CsvParser::GetData(unsigned int rows, unsigned int cols)
{
    if (rows >= _data.size() ||
        cols >= _data[rows].size()) {
        return "";
    }

    return _data[rows][cols].c_str();
}

/*
=================================
    CsvParser::getData
=================================
*/
const char* CsvParser::GetData(unsigned int rows, const char* colName)
{
    std::map<std::string, int>::iterator iter = _headerMap.find(colName);

    if (iter == _headerMap.end())
    {
        return "";
    }
    int cols = iter->second;
    if (rows >= _data.size())
    {
            return "";
    }

    return _data[rows][cols].c_str();
}


int CsvParser::GetRowNumber(const std::string& zero_key)
{
    std::map<std::string, int>::iterator iter = _rowMap.find(zero_key);
    if (iter != _rowMap.end())
    {
        return iter->second;
    }
    return -1;
}

/*
=================================
    swCSVParse::findColsData
=================================
*/
int CsvParser::FindColsData(int cols, const char* value)
{
    unsigned int i;
    for (i=0; i<_data.size(); i++) {
        if (strcmp(GetData(i, cols), value)==0 )
        {
            return i;
        }
    }
    return -1;
}

/*
=================================
    CsvParser::writeDate
=================================
*/
bool CsvParser::WriteDate(unsigned int rows, const char* colName, const int cellDate)
{
    std::stringstream ss;
    ss << cellDate;
    return WriteDate(rows, colName, ss.str().c_str());
}

/*
=================================
    CsvParser::writeDate
=================================
*/
bool CsvParser::WriteDate(unsigned int rows, const char* colName, const std::string& cellDate)
{
    return WriteDate(rows, colName, cellDate.c_str());
}

/*
=================================
    CsvParser::writeDate
=================================
*/
bool CsvParser::WriteDate(unsigned int rows, const char* colName, const char* cellDate)
{
    std::map<std::string, int>::iterator iter = _headerMap.find(colName);
    if (iter == _headerMap.end())
    {
        return false;
    }

    if (rows < _data.size())
    {
        _data[rows][iter->second] = std::string(cellDate);
        return true;
    }


    while (rows >= _data.size())
    {
        std::vector<std::string> newRow;
        newRow.resize(_header.size());
        _data.push_back(newRow);
        if (rows == _data.size()-1)
        {
            _data[rows][iter->second] = std::string(cellDate);
        }
    }

    return true;
}

/*
=================================
    CsvParser::writeDate
=================================
*/
bool CsvParser::WriteDate(unsigned int rows, unsigned int cols, const int cellDate)
{
    std::stringstream ss;
    ss << cellDate;
    return WriteDate(rows, cols, ss.str().c_str());
}

/*
=================================
    CsvParser::writeDate
=================================
*/
bool CsvParser::WriteDate(unsigned int rows, unsigned int cols, const std::string& cellDate)
{
    return WriteDate(rows, cols, cellDate.c_str());
}

/*
=================================
    CsvParser::writeDate
=================================
*/
bool CsvParser::WriteDate(unsigned int rows, unsigned int cols, const char* cellDate)
{
    if (cols >= _data[rows].size())
    {
            return false;
    }

    if (rows < _data.size())
    {
        _data[rows][cols] = std::string(cellDate);
        return true;
    }

    while (rows >= _data.size())
    {
        std::vector<std::string> newRow;
        newRow.resize(_header.size());
        _data.push_back(newRow);
        if (rows == _data.size()-1)
        {
            _data[rows][cols] = std::string(cellDate);
        }
    }
    return true;
}

}
