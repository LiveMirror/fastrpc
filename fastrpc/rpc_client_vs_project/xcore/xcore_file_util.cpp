// 2008-09-18
// xcore_file_util.cpp
// 
// 文件系统操作类


#include "xcore_file_util.h"
#include "xcore_str_util.h"
#include "xcore_critical.h"

#ifdef __WINDOWS__
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#endif//__WINDOWS__
#ifdef __GNUC__
#include <dirent.h>
#endif//__GNUC__

namespace xcore {

///////////////////////////////////////////////////////////////////////////////
// XFileFinder
///////////////////////////////////////////////////////////////////////////////

#ifdef __WINDOWS__

typedef struct
{
	HANDLE				m_handle;
	WIN32_FIND_DATA		m_findData;
	char                m_path[1024];
} __filefinder_handle;

XFileFinder::XFileFinder()
{
	m_finder = malloc(sizeof(__filefinder_handle));
	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	finder->m_handle = INVALID_HANDLE_VALUE;
	finder->m_path[0] ='\0';
	m_mask = 0;
}

XFileFinder::~XFileFinder()
{
	ASSERT(m_finder);
	this->close();
	free(m_finder);
	m_finder = NULL;
}

bool XFileFinder::open(const string& path, uint32 mask)
{
	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	string path_ = path;
	XStrUtil::chop(path_, " \t\r\n\\");
	if (path_.size() >= 1024) return false;
	memcpy(finder->m_path, path_.c_str(), path_.size() + 1);
	m_mask = mask & (TYPE_ALL | ATTR_ALL);
	this->close();
	return XFileUtil::is_dir(finder->m_path);
}

int XFileFinder::next(string& name)
{
	name.clear();
	if (0 == (m_mask & TYPE_ALL)) return TYPE_NONE;

	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	while (true)
	{
		if (finder->m_handle == INVALID_HANDLE_VALUE)
		{
			string path_ = finder->m_path;
			path_ += "\\*";
			finder->m_handle = FindFirstFile(path_.c_str(), &finder->m_findData);
			if (finder->m_handle == INVALID_HANDLE_VALUE) break;
		}
		else if (!FindNextFile(finder->m_handle, &finder->m_findData))
		{
			break;
		}
		name = finder->m_findData.cFileName;

		if (name == "." || name == "..")
		{
			continue;
		}
		if ((finder->m_findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !(m_mask & ATTR_HIDE))
		{
			continue;
		}
		if ((finder->m_findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && !(m_mask & ATTR_SYSTEM))
		{
			continue;
		}

		if(finder->m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return TYPE_DIR;
		}
		if (finder->m_findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			return TYPE_REGULAR;
		}
		if (finder->m_findData.dwFileAttributes & FILE_ATTRIBUTE_DEVICE)
		{
			return TYPE_BLOCK;
		}
		return TYPE_OTHER;
	}

	return TYPE_NONE;
}

void XFileFinder::close()
{
	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	if (finder->m_handle != INVALID_HANDLE_VALUE)
	{
		FindClose(finder->m_handle);
		finder->m_handle = INVALID_HANDLE_VALUE;
	}
	return;
}

#endif//__WINDOWS__


///////////////////////////////////////////////////////////////////////////////
// XFileFinder
///////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

typedef struct
{
	DIR*				m_dir;
	struct dirent*		m_dirent;
} __filefinder_handle;

XFileFinder::XFileFinder()
{
	m_finder = malloc(sizeof(__filefinder_handle));
	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	finder->m_dir = NULL;
	finder->m_dirent = NULL;
	m_mask = 0;
}

XFileFinder::~XFileFinder()
{
	ASSERT(m_finder);
	this->close();
	free(m_finder);
	m_finder = NULL;
}

bool XFileFinder::open(const string& strSearchPath, uint32 mask)
{
	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	m_mask = mask & (TYPE_ALL | ATTR_ALL);
	
	this->close();
	finder->m_dir = opendir(strSearchPath.c_str());
	return (finder->m_dir != NULL);
}

int XFileFinder::next(string& name)
{
	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	if ((finder->m_dir == NULL) || (0 == (m_mask && TYPE_ALL)))
	{
		return TYPE_NONE;
	}

	XLockGuard<XCritical> lock(XCritical::StaticCritical); // 非线程安全，需加锁

	while((finder->m_dirent = readdir(finder->m_dir)) != NULL)
	{
		name = finder->m_dirent->d_name;

		if (name == "." || name == "..")
		{
			continue;
		}

		// skip hide file
		if ((finder->m_dirent->d_name[0] == '.') && (0 == m_mask & ATTR_HIDE))
		{
			continue;
		}

		switch (finder->m_dirent->d_type)
		{
		case DT_DIR:
			return TYPE_DIR;
		case DT_REG:
			return TYPE_REGULAR;
		case DT_LNK:
			return TYPE_LINK;
		case DT_CHR:
			return TYPE_CHAR;
		case DT_BLK:
			return TYPE_BLOCK;
		case DT_FIFO:
			return TYPE_FIFO;
		case DT_SOCK:
			return TYPE_SOCKET;
		default:
			return TYPE_OTHER;
		}
	}
	return TYPE_NONE;
}

void XFileFinder::close()
{
	ASSERT(m_finder);
	__filefinder_handle* finder = (__filefinder_handle *)m_finder;
	if (finder->m_dir != NULL)
	{
		closedir(finder->m_dir);
		finder->m_dir = NULL;
	}
}

#endif//__GNUC__


///////////////////////////////////////////////////////////////////////////////
// class XFileUtil
///////////////////////////////////////////////////////////////////////////////
#ifdef __WINDOWS__

char XFileUtil::PATH_SEPARATOR = '\\';

bool XFileUtil::make_dir(const string &strDirPath)
{
	if (!CreateDirectory(strDirPath.c_str(), NULL))
	{
		if (ERROR_ALREADY_EXISTS == GetLastError() && is_dir(strDirPath))
		{
			return true;
		}
		return	false;
	}

	return true;
}

bool XFileUtil::make_dir_p(const string &strDirPath)
{
	vector<string> vItems;
	vector<string>::iterator it;
	string strPath = strDirPath;
	string strTmp;

	XStrUtil::chop(strPath, "\r\n\t ");
	XStrUtil::split(strPath, vItems, "\\/");
	if (vItems.size() == 0) return true;
	if (strPath.size() >= 2 && strPath[1] == ':')
	{
		// 带盘符的绝对路径
		strTmp += strPath[0];
		strTmp += ":\\";
		vItems.erase(vItems.begin());
	}
	else if (XStrUtil::compare("\\\\", strPath, 2) == 0)
	{
		// 局限网路径
		strTmp = "\\\\";
		strTmp += vItems[0];
		strTmp += '\\';
		vItems.erase(vItems.begin());
	}

	for (it = vItems.begin(); it != vItems.end(); it++)
	{
		strTmp += (*it);
		strTmp += '\\';
		if (!make_dir(strTmp))
		{
			return false;
		}
	}

	return true;
}

bool XFileUtil::remove_dir(const string &strDirPath)
{
	return (TRUE == RemoveDirectory(strDirPath.c_str()));
}

bool XFileUtil::remove_dir_r(const string &strDirPath)
{
	string strName;
	XFileFinder finder;
	int type = XFileFinder::TYPE_NONE;
	string dirpath = strDirPath;
	XStrUtil::chop_tail(dirpath, " \t\n\t\\");

	if (!finder.open(dirpath))
	{
		return false;
	}
	while ((type = finder.next(strName)) != XFileFinder::TYPE_NONE)
	{
		if (type == XFileFinder::TYPE_DIR)
		{
			remove_dir_r(dirpath + '\\' + strName);
		}
		else
		{
			remove_file(dirpath + '\\' + strName);
		}
	}
	finder.close();
	return remove_dir(dirpath);
}

bool XFileUtil::remove_file(const string &strFilePath)
{
	return (bool)(0 == remove(strFilePath.c_str()));
}

bool XFileUtil::make_link(const string &strTargetPath, const string &strNewPath)
{
	return false;
}

bool XFileUtil::make_link_p(const string &strTargetPath, const string &strNewPath)
{
	return false;
}

bool XFileUtil::is_link(const string &strpath, /*out*/string &strTargetPath)
{
	return false;
}

bool XFileUtil::is_dir(const string &strPath)
{
	string str = strPath;
	XStrUtil::chop_tail(str, " \r\n\t\\");

	struct _stat statinfo;
	if (0 != ::_stat(str.c_str(), &statinfo))
	{
		return false;
	}
	if (_S_IFDIR & statinfo.st_mode)
	{
		return true;
	}
	return false;
}

bool XFileUtil::is_regular(const string &strPath)
{
	struct _stat statinfo;
	if (0 != ::_stat(strPath.c_str(), &statinfo))
	{
		return false;
	}
	if (_S_IFREG & statinfo.st_mode)
	{
		return true;
	}
	return false;
}

bool XFileUtil::is_exist(const string &strPath)
{
	return (bool)(0 == ::_access_s(strPath.c_str(), 00));
}

string XFileUtil::get_work_dir()
{
	char buf[1024] = {};
	if (0 == ::GetCurrentDirectory(1023, buf))
	{
		return "";
	}
	return buf;
}

bool XFileUtil::set_work_dir(const string &strDir)
{
	return (TRUE == SetCurrentDirectory(strDir.c_str()));
}

string XFileUtil::get_module_path()
{
	char buf[1024];
	int ret = GetModuleFileName(GetModuleHandle(NULL), buf, 1023);
	buf[ret] = '\0';
	return buf;
}

bool XFileUtil::parse_path(const string& path, string& dirpath, string& filename_perfix, string& filename_postfix)
{
	string filename;
	string path_ = path;
	XStrUtil::chop(path_, " \r\n\t");
	XStrUtil::replace((char *)path_.c_str(), '/', '\\');
	dirpath = ".\\";
	filename_perfix.clear();
	filename_postfix.clear();

	size_t pos = path_.rfind('\\');
	if (pos == string::npos)
	{
		filename = path_;
		if ((filename.size() == 2 && filename.at(1) == ':') || // such as "c:"
			(filename == ".") ||  // such as "."
			(filename == ".."))   // such as ".."
		{
			dirpath = filename;
			filename.clear();
		}
	}
	else
	{
		filename = path_.substr(pos + 1);
		dirpath = path_.substr(0, pos + 1);
	}

	pos = filename.rfind('.');
	if (pos == string::npos)
	{
		filename_perfix = filename;
	}
	else
	{
		filename_perfix = filename.substr(0, pos);
		filename_postfix = filename.substr(pos + 1);
	}
	return true;
}

string XFileUtil::get_system_dir()
{
	char buf[1024];
	UINT ret = GetSystemDirectory(buf, 1023);
	if (ret <= 0 || ret >= 1023) return "";
	buf[ret] = '\0';
	return buf;
}

uint32 XFileUtil::get_page_size()
{
	return 4 * 1024;  // 所有NTFS文件系统4G以上分区，簇大小均为4KB
}

uint32 XFileUtil::get_name_max(const char* dir_path)
{
	return 255;
}

uint32 XFileUtil::get_path_max(const char* dir_path)
{
	return 1023;
}

#endif//__WINDOWS__


///////////////////////////////////////////////////////////////////////////////
// XFileUtil
///////////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__

char XFileUtil::PATH_SEPARATOR = '/';

bool XFileUtil::make_dir(const string &strDirPath)
{
	if (-1 == mkdir(strDirPath.c_str(), 0777))
	{
		if ((EEXIST == errno) && is_dir(strDirPath))
		{
			return true;
		}
		return false;
	}

	return true;
}

bool XFileUtil::make_dir_p(const string &strDirPath)
{
	vector<string> vItems;
	vector<string>::iterator it;
	string strPath = strDirPath;
	string strTmp;

	XStrUtil::chop(strPath, "\r\n\t ");
	XStrUtil::split(strPath, vItems, "\\/");
	if (vItems.size() == 0) return true;

	if (strPath[0] == '/') strTmp = "/";
	for (size_t i = 0; i < vItems.size(); i++)
	{
		strTmp += vItems[i];
		strTmp += '/';
		if (!make_dir(strTmp)) return false;
	}
	return true;
}

bool XFileUtil::remove_dir(const string &strDirPath)
{
	return (bool)(0 == rmdir(strDirPath.c_str()));
}

bool XFileUtil::remove_dir_r(const string &strDirPath)
{
	string strName;
	XFileFinder finder;
	int type = XFileFinder::TYPE_NONE;
	string strTmp = strDirPath;
	XStrUtil::chop_tail(strTmp, " \t\r\n/");
	if (strTmp.empty() && strDirPath.at(0) == '/') strTmp = "/";

	if (!finder.open(strTmp))
	{
		return false;
	}
	while ((type = finder.next(strName)) != XFileFinder::TYPE_NONE)
	{
		if (type == XFileFinder::TYPE_DIR)
		{
			remove_dir_r(strTmp + '/' + strName);
		}
		else
		{
			remove_file(strTmp + '/' + strName);
		}
	}
	finder.close();
	return remove_dir(strTmp);
}

bool XFileUtil::remove_file(const string &strFilePath)
{
	return (bool)(0 == unlink(strFilePath.c_str()));
}

bool XFileUtil::make_link(const string &strTargetPath, const string &strNewPath)
{
	return (bool)(0 == symlink(strTargetPath.c_str(), strNewPath.c_str()));
}

bool XFileUtil::make_link_p(const string &strTargetPath, const string &strNewPath)
{
	string strDirPath, strFileName_prefix, strFileName_postfix, strTmp;
	if (!parse_path(strNewPath, strDirPath, strFileName_prefix, strFileName_postfix)) return false;
	if (!make_dir_p(strDirPath)) return false;
	if (0 != symlink(strTargetPath.c_str(), strNewPath.c_str()))
	{
		if (EEXIST == errno)
		{
			if (!is_link(strNewPath, strTmp) || (strTmp != strTargetPath))
			{
				return false;
			}
		}
	}
	return true;
}

bool XFileUtil::is_link(const string &strpath, string &strTargetPath)
{
	char buf[1024] = {};
	if (-1 == readlink(strpath.c_str(), buf, 1023))
	{
		return false;
	}
	strTargetPath = buf;
	return true;
}

bool XFileUtil::is_dir(const string &strPath)
{
	struct stat statinfo;
	if (0 != lstat(strPath.c_str(), &statinfo))
	{
		return false;
	}
	if (!S_ISDIR(statinfo.st_mode))
	{
		return false;
	}
	return true;
}

bool XFileUtil::is_regular(const string &strPath)
{
	struct stat statinfo;
	if (0 != lstat(strPath.c_str(), &statinfo))
	{
		return false;
	}
	if (!S_ISREG(statinfo.st_mode))
	{
		return false;
	}
	return true;
}

bool XFileUtil::is_exist(const string &strPath)
{
	return (bool)(0 == access(strPath.c_str(), F_OK));
}

string XFileUtil::get_work_dir()
{
	char buf[1024] = {};
	if (NULL == getcwd(buf, 1023))
	{
		return "";
	}
	return buf;
}

bool XFileUtil::set_work_dir(const string &strDir)
{
	return (bool)(0 == chdir(strDir.c_str()));
}

string XFileUtil::get_module_path()
{
	char buf[1024];
	int ret = readlink("/proc/self/exe", buf, 1023);
	if (-1 == ret) return "";
	buf[ret] = '\0';
	return buf;
}

bool XFileUtil::parse_path(const string& path, string& dirpath, string& filename_perfix, string& filename_postfix)
{
	string path_ = path;
	string filename;
	XStrUtil::chop(path_, " \r\n\t");
	dirpath = "./";
	filename_perfix.clear();
	filename_postfix.clear();

	size_t pos = path_.rfind('/');
	if (pos == string::npos)
	{
		if ((path_ == ".") || (path_ == ".."))   // such as ".' or ".."
		{
			dirpath = path_;
		}
		else
		{
			filename = path_;
		}
	}
	else
	{
		filename = path_.substr(pos + 1);
		dirpath = path_.substr(0, pos + 1);
		if (filename == "." || filename == "..")
		{
			dirpath = dirpath + '/' + filename;
			filename.clear();
		}
	}
	if (filename.empty()) return true;

	string name = filename;
	if (filename.at(0) == '.') name = filename.substr(1);
	pos = name.rfind('.');
	if (pos == string::npos)
	{
		filename_perfix = name;
	}
	else
	{
		filename_perfix = name.substr(0, pos);
		filename_postfix = name.substr(pos + 1);
	}
	if (filename.at(0) == '.') filename_perfix = string(".") + filename_perfix;
	return true;
}

string XFileUtil::get_system_dir()
{
	return "/";
}

uint32 XFileUtil::get_page_size()
{
	long val;
	errno = 0;
	if ((val = sysconf(_SC_PAGE_SIZE)) < 0)
	{
		if (errno != 0) return 4096; // error, return default is 4096
		return 4096;  // no defined, return default is 4096
	}
	
	return (uint32)val;
}

uint32 XFileUtil::get_name_max(const char* dir_path)
{
	long val;
	errno = 0;
	if ((val = pathconf(dir_path == NULL ? "." : dir_path, _PC_NAME_MAX)) < 0)
	{
		if (errno != 0) return 255;  // error, return default is 255
		return 255; // no limit, return default is 255
	}

	return (uint32)val;
}

uint32 XFileUtil::get_path_max(const char* dir_path)
{
	long val;
	errno = 0;
	if ((val = pathconf(dir_path == NULL ? "." : dir_path, _PC_PATH_MAX)) < 0)
	{
		if (errno != 0) return 1023;  // error, return default is 1023
		return 1023; // no limit, return default is 1023
	}
	
	return (uint32)val;
}

#endif//__GNUC__

} // namespace xcore


////////////////////////////////////////////////////////////////////////////////
// test section
////////////////////////////////////////////////////////////////////////////////

#ifdef _XCORE_NEED_TEST

#include "xcore_test.h"

namespace xcore
{

bool xcore_test_file_util()
{
	return true;
}

}//namespace xcore

#endif//_XCORE_NEED_TEST
