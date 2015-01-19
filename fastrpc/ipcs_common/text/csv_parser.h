/// Jack
#ifndef CSV_PARSER_H
#define CSV_PARSER_H
#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

namespace ipcs_common {


class CsvParser {

public:
    static char PATH_SEPARATOR;
	static std::string FIELD_SEP;      // separator characters
public:
    CsvParser(bool hasDescibe = false, bool parse_data = true);
    virtual ~CsvParser();

    bool    Load(const std::string& fileName);
    bool    Save(const std::string& strFilename);
    void    MakeFileHead(const std::string& fileHeader, const std::string& delimiter = "|");

    const char* GetData(unsigned int rows, unsigned int cols);
    const char* GetData(unsigned int rows, const char* colName);

    int GetInt(unsigned int row, const char* column_name);
    int GetInt(unsigned int row, unsigned int col);

    int GetRowNumber(const std::string& zero_key);

    void GetMultiInts(unsigned int row, const char* column_name,
                      std::vector<int>* array, const std::string delim="|");

    void GetMultiInts(unsigned int row, unsigned int col,
                      std::vector<int>* array, const std::string delim="|");

    int FindColsData(int cols, const char* value);

    unsigned int GetCols() const {
        return (unsigned int)_header.size();
    }

    unsigned int GetRows() const {
        return (unsigned int)_data.size();
    }

    int GetAllIID(std::vector<std::string>* array);

    bool    WriteDate(unsigned int rows, const char* colName, const int cellDate);
    bool    WriteDate(unsigned int rows, const char* colName, const std::string& cellDate);
    bool    WriteDate(unsigned int rows, const char* colName, const char* cellDate);
    bool    WriteDate(unsigned int rows, unsigned int cols, const int cellDate);
    bool    WriteDate(unsigned int rows, unsigned int cols, const std::string& cellDate);
    bool    WriteDate(unsigned int rows, unsigned int cols, const char* cellDate);

	static void	split(const std::string& line, std::vector<std::string>* field);
private:
    
    static int	advplain(const std::string& line, std::string& fld, int);
    static int	advquoted(const std::string& line, std::string& fld, int);
    std::vector<std::string> tokenize(const std::string hay, const std::string delim);

private:
    bool							_hasDescribe;   // if exist describe ,value is true
    bool							_do_parse_data; // whether parse data or not
    int								_currColumn;    // current column
    int								_currRow;       // current row
    std::vector< std::string >		_header;        // header info
    std::map< std::string, int>		_headerMap;

    std::vector< std::string >		_describe;      // describe info
    std::vector< std::vector<std::string> >	_data;          // data info
    std::map<std::string, int>		_rowMap;

};

}

#endif

