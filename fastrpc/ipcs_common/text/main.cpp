
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "csv_parse.h"

using namespace std;

int main(int argc, char** argv)
{
    cout << "start generate csv parse file...";
    if (argc < 1){
        cout << "please input generate file! " << endl;
        return 0;
    }
    string file_path = argv[0];


    cout << "file path :"<< file_path << endl;

    common_util::CsvParser* csv_parser = new common_util::CsvParser(true, false);
    csv_parser->Load(file_path);
    CppPage* cpp_page = new CppPage();
    cpp_page->open(file_path);
    if(!cpp_page->make(*csv_parser)){
        cout << "generate csv parse cpp failed ! " << endl;
    } else {
        cout << "generate csv parse cpp success ! " << endl;
    }
    system( "pause" );
    return 0;
}
