#include <sys/stat.h>
#include <assert.h>

#include "cpython.h"

pthread_mutex_t CPython::_mux = PTHREAD_MUTEX_INITIALIZER;
PyThreadState * CPython::mainThreadState = NULL;
std::string CPython::_script_path = "";

void printDict(PyObject* obj)
{
    if(!PyDict_Check(obj))
        return;
    PyObject *k, *keys;
    keys = PyDict_Keys(obj);
    for(int i = 0; i < PyList_GET_SIZE(keys); ++i)
    {
        k = PyList_GET_ITEM(keys, i);
        std::string strName = PyString_AsString(k);
        std::cout << strName << std::endl;
    }
}

PyObject* ReImport(const char* main_py_name,
              const char* main_class_name,
              PyObject*& pObj,
              PyObject*& pArgs,
              PyObject*& pClass,
              PyObject*& pDict,
              PyObject*& pModule,
              PyObject*& pName) {

    if (pObj) {
        Py_DECREF(pObj);
        pObj = NULL;
    }

    if (pArgs) {
        Py_DECREF(pArgs);
        pArgs = NULL;
    }
    if (pClass) {
        Py_DECREF(pClass);
        pClass = NULL;
    }
    if (pDict) {
        Py_DECREF(pDict);
        pDict = NULL;
    }
    if (pModule) {
        Py_DECREF(pModule);
        pModule = NULL;
    }
    if (pName) {
        Py_DECREF(pName);
        pName = NULL;
    }

    PyRun_SimpleString("import sys");
    std::string import_cmd;
    if (CPython::_script_path.empty()) {
        import_cmd = "sys.path.append('./')";
    } else {
        import_cmd.append("sys.path.append('");
        import_cmd.append(CPython::_script_path);
        import_cmd.append("')");
    }
    PyRun_SimpleString(import_cmd.c_str());
    // 载入名为test的脚本
    pName = PyString_FromString(main_py_name);

    // 这步就执行了python
    pModule = PyImport_Import(pName);
    if(!pModule) {
        std::cout << "can't find " << main_py_name << ".py";
        assert(false);
    }
    pDict = PyModule_GetDict(pModule);
    if(!pDict) {
        std::cout << "get py dict fail";
        assert(false);
    }

    pClass = PyDict_GetItemString(pDict, main_class_name);
    if (!pClass) {
        std::cout << "pClass is NULL";
        assert(false);
    }
    if(!pClass || !PyClass_Check(pClass)) {
        std::cout << "can't find function " << main_class_name;
        assert(false);
    }

    // 初始化对象
    pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", ""));
    pObj = PyInstance_New(pClass, pArgs, NULL);

    //// 检查是否初始化对象
    if(!PyInstance_Check(pObj)) {
        std::cout << "can't instance";
        assert(false);
    }
    return pObj;
}

ThreadObject::ThreadObject(PyThreadState* _myThreadState,
                           string& _main_filename,
                           string& _class_name) {
    pName = NULL;
    pModule = NULL;
    pDict = NULL;
    pClass = NULL;
    pArgs = NULL;
    obj = NULL;

    myThreadState = _myThreadState;
    main_filename = _main_filename;
    class_name = _class_name;
    struct stat file_stat;
    string main_filepath = main_filename + ".py";
    stat(main_filepath.c_str(), &file_stat);
    mtime = file_stat.st_mtime;
    ReImport(main_filename.c_str(),
             class_name.c_str(),
             obj,
             pArgs,
             pClass,
             pDict,
             pModule,
             pName);
}

ThreadObject::~ThreadObject() {
    if (obj) {
        Py_DECREF(obj);
        obj = NULL;
    }

    if (pArgs) {
        Py_DECREF(pArgs);
        pArgs = NULL;
    }
    if (pClass) {
        Py_DECREF(pClass);
        pClass = NULL;
    }
    if (pDict) {
        Py_DECREF(pDict);
        pDict = NULL;
    }
    if (pModule) {
        Py_DECREF(pModule);
        pModule = NULL;
    }
    if (pName) {
        Py_DECREF(pName);
        pName = NULL;
    }
}

PyObject* ThreadObject::GetObj() {
    //struct stat file_stat;
    //string main_filepath = main_filename + ".py";
    //stat(main_filepath.c_str(), &file_stat);
    //int64_t new_mtime = file_stat.st_mtime;
    //if (new_mtime != mtime) {
    //    printf("reload %d %d %s\n", new_mtime, mtime, main_filename.c_str());
    //    Py_DECREF(obj);
    //    ReImport(main_filename.c_str(),
    //             class_name.c_str(),
    //             obj,
    //             pArgs,
    //             pClass,
    //             pDict,
    //             pModule,
    //             pName);
    //    mtime = new_mtime;
    //}
    return obj;
}

void ThreadObject::Lock() {
    PyEval_AcquireLock();
    PyThreadState_Swap(myThreadState);
}

void ThreadObject::UnLock() {
    PyThreadState_Swap(NULL);
    PyEval_ReleaseLock();
}

bool CPython::Init(const std::string script_path) {
    _script_path = script_path;
    pthread_mutex_init(&_mux,NULL);
    Py_Initialize();
    if(!Py_IsInitialized()) {
        printf("init python fail\n");
        exit(1);
    }

    PyEval_InitThreads();
    mainThreadState = PyThreadState_Get();
    PyEval_ReleaseLock();
    return true;
}

ThreadObject* CPython::GetNewInstance(const string main_py_name,
                                  const string main_class_name) {
    pthread_mutex_lock(&_mux);

    string main_file_name = main_py_name;
    string class_name = main_class_name;

    PyEval_AcquireLock();
    PyInterpreterState* maininterpreterState = mainThreadState->interp;
    PyThreadState* myThreadState = PyThreadState_New(maininterpreterState);
    PyEval_ReleaseLock();
    ThreadObject* thread_obj = NULL;
    thread_obj = new ThreadObject(myThreadState,
                                  main_file_name,
                                  class_name);

    pthread_mutex_unlock(&_mux);
    return thread_obj;

}

CPython::CPython() {
    pthread_mutex_init(&_jobmux,NULL);
    if((sem_init(&_jobsem,0,0))<0){
        std::cout << "init sem fail";
        assert(false);
    }
}

CPython::~CPython() {
}

bool CPython::RegiPyClass(string file_name,
                          string class_name,
                          int num) {
    pthread_mutex_lock(&_jobmux);
    for (int i = 0; i < num; ++i) {
        ThreadObject* thread_obj = CPython::GetNewInstance(file_name, class_name);
        if (!thread_obj) {
            std::cout << "new thread obj fail";
            assert(false);
        }
        obj_list.push_back(thread_obj);
        sem_post(&_jobsem);
    }
    pthread_mutex_unlock(&_jobmux);
    return true;
}

ThreadObject* CPython::GetPyObj() {
    ThreadObject* obj = NULL;
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    if (!obj_list.empty()){
        obj = obj_list.back();
        obj_list.pop_back();
    }
    if (!obj) {
        std::cout << obj_list.size();
    }
    pthread_mutex_unlock(&_jobmux);
    return obj;
}

ThreadObject* CPython::GetHeadObj() {
    ThreadObject* obj = NULL;
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    if (!obj_list.empty()){
        obj = obj_list.front();
        obj_list.pop_front();
    }
    if (!obj) {
        std::cout << obj_list.size();
    }
    pthread_mutex_unlock(&_jobmux);
    return obj;
}

bool CPython::Release(ThreadObject* obj) {
    if (!obj) {
        return false;
    }
    pthread_mutex_lock(&_jobmux);
    obj_list.push_back(obj);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);
    return true;
}

string CPython::RunScript(string fun_name,
                          string param) {
    string sContent;
    ThreadObject* obj = GetPyObj();
    if (!obj) {
        assert(false);
    }
    obj->Lock();
    // 这个lock容易让人产生误解
    // 实际上此lock为取得gil
    // RunScript方法对所有CPython对象的所有ThreadObject
    // 都是并行执行的并且是线程安全的
    PyObject* pObj = obj->GetObj();
    PyObject *ret_value = PyObject_CallMethod(pObj,
                                              (char*)fun_name.c_str(),
                                              (char*)"(s)",
                                              (char*)param.c_str());
    int ret_size = PyString_Size(ret_value);
    char* res = PyString_AsString(ret_value);
    sContent.assign(res,ret_size);
    Py_DECREF(ret_value);
    obj->UnLock();
    Release(obj);
    return sContent;
}

bool CPython::AllRunScript(string fun_name,
                           Json::Value& param,
                           std::vector<Json::Value>& res_list) {
    Json::FastWriter fast_writer;
    std::string param_str = fast_writer.write(param);
    size_t size = obj_list.size();
    while (size-- > 0) {
        string sContent;
        ThreadObject* obj = GetHeadObj();
        if (!obj) {
            assert(false);
        }
        obj->Lock();
        PyObject* pObj = obj->GetObj();
        PyObject *ret_value = PyObject_CallMethod(pObj,
                                                  (char*)fun_name.c_str(),
                                                  (char*)"(s)",
                                                  (char*)param_str.c_str());
        int ret_size = PyString_Size(ret_value);
        char* res_str = PyString_AsString(ret_value);
        sContent.assign(res_str,ret_size);
        Py_DECREF(ret_value);
        obj->UnLock();
        Release(obj);
        Json::Value res;
        Json::Reader reader;
        if (!reader.parse(sContent, res)) {
            return false;
        }
        res_list.push_back(res);
    }
    return true;
}

bool CPython::RunScript(string fun_name,
                        Json::Value& param,
                        Json::Value& res) {
    Json::FastWriter fast_writer;
    std::string param_str = fast_writer.write(param);
    string sContent;
    ThreadObject* obj = GetPyObj();
    if (!obj) {
        assert(false);
    }
    obj->Lock();
    // 这个lock容易让人产生误解
    // 实际上此lock为取得gil
    // RunScript方法对所有CPython对象的所有ThreadObject
    // 都是并行执行的并且是线程安全的
    PyObject* pObj = obj->GetObj();
    PyObject *ret_value = PyObject_CallMethod(pObj,
                                              (char*)fun_name.c_str(),
                                              (char*)"(s)",
                                              (char*)param_str.c_str());
    int ret_size = PyString_Size(ret_value);
    char* res_str = PyString_AsString(ret_value);
    sContent.assign(res_str,ret_size);
    Py_DECREF(ret_value);
    obj->UnLock();
    Release(obj);
    Json::Reader reader;
    if (reader.parse(sContent, res)) {
        return true;
    } else {
        res["err"] = sContent;
        return false;
    }
}



