#ifndef _CPYTHON_H_
#define _CPYTHON_H_


#include <iostream>
#include <Python.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <deque>
#include "json.h"

using std::string;
using std::map;
using std::vector;

class ThreadObject {
public:
    ThreadObject(PyThreadState* _mainThreadState,
                 string& _main_filename,
                 string& _class_name);
    ~ThreadObject();
    PyObject *pName;
    PyObject *pModule;
    PyObject *pDict;
    PyObject *pClass;
    PyObject *pArgs;
    PyObject* obj;
    PyThreadState* myThreadState;
    string main_filename;
    string class_name;
    int64_t mtime;
    void Lock();
    void UnLock();

    PyObject* GetObj();
};

class CPython{
public:
    static pthread_mutex_t _mux;
    static PyThreadState *mainThreadState;
    static std::string _script_path;
    sem_t _jobsem;
    pthread_mutex_t _jobmux;
    std::deque<ThreadObject*> obj_list;
public:
    static bool Init(const std::string script_path);
    static ThreadObject* GetNewInstance(const string main_py_name,
                                    const string main_class_name);
    CPython();
    ~CPython();
    bool RegiPyClass(string file_name,
                     string class_name,
                     int num);
    string RunScript(string fun_name,
                     string param);
    bool RunScript(string fun_name,
                   Json::Value& param,
                   Json::Value& res);
    bool AllRunScript(string fun_name,
                      Json::Value& param,
                      std::vector<Json::Value>& res_list);
    ThreadObject* GetPyObj();
    ThreadObject* GetHeadObj();
    bool Release(ThreadObject* obj);
};

class PyThreadStateLock
{
public:
    PyThreadStateLock(void) {
        state = PyGILState_Ensure();
    }

    ~PyThreadStateLock(void) {
        PyGILState_Release(state);
    }
private:
    PyGILState_STATE state;
};

#endif
