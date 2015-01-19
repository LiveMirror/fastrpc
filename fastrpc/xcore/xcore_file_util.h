// 2008-09-23
// xcore_file_util.h
// 
// 文件系统操作类


#ifndef _XCORE_FILE_UTIL_H_
#define _XCORE_FILE_UTIL_H_

#include "xcore_define.h"

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// class XFileFinder
///////////////////////////////////////////////////////////////////////////////
class XFileFinder
{
public:
	enum
	{
		TYPE_NONE      = 0x00000000,   // 没有
		TYPE_DIR       = 0x00000001,   // 目录
		TYPE_REGULAR   = 0x00000002,   // 普通文件
		TYPE_LINK      = 0x00000004,   // 符号连接
		TYPE_CHAR      = 0x00000008,   // 字符特殊文件
		TYPE_BLOCK     = 0x00000010,   // 块特殊文件
		TYPE_FIFO      = 0x00000020,   // 管道
		TYPE_SOCKET    = 0x00000040,   // 套接字
		TYPE_OTHER     = 0x00000080,   // 其他类型
		TYPE_ALL       = 0x000000FF,   // 所有文件类型

		ATTR_HIDE      = 0x00000100,   // 隐藏
		ATTR_SYSTEM    = 0x00000200,   // 系统文件
		ATTR_ALL       = 0x00000300    // 所有属性
	};

public:
	XFileFinder();
	~XFileFinder();

	bool open(const string& path, uint32 mask = TYPE_ALL | ATTR_ALL);
	int  next(string& name);
	void close();

private:
	void*      m_finder;
	uint32     m_mask;
};


///////////////////////////////////////////////////////////////////////////////
// class XFileUtil
///////////////////////////////////////////////////////////////////////////////
class XFileUtil
{
public:
	static char PATH_SEPARATOR;
public:
	static bool make_dir(const string &strDirPath);
	static bool make_dir_p(const string &strDirPath);
	static bool remove_dir(const string &strDirPath);
	static bool remove_dir_r(const string &strDirPath);
	static bool remove_file(const string &strFilePath);
	static bool make_link(const string &strTargetPath, const string &strNewPath);
	static bool make_link_p(const string &strTargetPath, const string &strNewPath);

	// check type of file or dir
	static bool is_link(const string &strpath, /*out*/string &strTargetPath);
	static bool is_dir(const string &strPath);
	static bool is_regular(const string &strPath);
	static bool is_exist(const string &strPath);

	static bool parse_path(const string& path, string& dirpath, string& filename_perfix, string& filename_postfix);
	
	static bool   set_work_dir(const string &strDir);
	static string get_work_dir();
	static string get_module_path();
	static string get_system_dir();

	static uint32 get_page_size();
	static uint32 get_name_max(const char* dir_path = NULL);
	static uint32 get_path_max(const char* dir_path = NULL);
};

} // namespace xcore

#endif//_XCORE_FILE_UTIL_H_
