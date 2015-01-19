#ifndef _CMONGO_H_
#define _CMONGO_H_

#include <deque>
#include "dbclient.h"
#include <stdlib.h>

using namespace mongo;

//函数返回0:成功 >0 出错
enum MongoStatus {
    RET_OK = 0,
    RET_ERR = 1,
    RET_PARERR = 2
};

class CmongoPool{
public:
    //传入连接数到构造函数，默认连接数为size
    CmongoPool(int pool_size, string mhost="127.0.0.1", int mport=27017, int time_out=3);
    //析构函数
    ~CmongoPool();
public:
    //获取其中一个连接给外部用，没有连接则等待
    DBClientConnection* get_con();
    //同上，但不等待没有连接则马上返回NULL
    DBClientConnection* get_con_nowait();
    //将连接释放回连接池
    bool release(DBClientConnection* con);
    //设置tcp读写超时时间
    int set_wr_timeout(double t);
    //设置db collection
    int setdb(string mdb,string mcollection);
    //设置索引
    int setindex(string& doc, string key);
    //查询
    int get(string& doc, map<string,string>& out,vector<string> in,string key,string key_val);
    //投递一批要查询的字段，fields为要查询哪些字段
    int gets(string& doc, map< string,map<string,string> >& rout,vector<string> fields,vector<string> in,string key);
    //dump key-value dumpkey对应一个value
    int dumpkey(string& doc, map< string,string >& rout,string key,string val);
    //dump key->map<key,value> dumpkey对应一组value
    int dumpvals(string& doc, map< string,map<string,string> >& rout,vector<string> in,string key);
    //写入
    int set(string& doc, map<string,string> in,string key,string key_val);
    //批量写入
    //更新接口，批量更新key="id"
    //  "123456":<key,value>,<key,value>
    //  "123457":<key,value>,<key,value>
    int sets(string& doc, map< string,map<string,string> > in,string key);
    //删除
    int remove(string& doc, string key,string key_val);
private:
    string m_doc;
    //tcp读写超时时间
    double wr_timeout;
    pthread_mutex_t _jobmux;
    sem_t _jobsem;
    std::deque<DBClientConnection*> con_queue;
    std::deque<DBClientConnection*> con_queue_using;
    pthread_mutex_t _dbmux;

};


CmongoPool::CmongoPool(int pool_size, string mhost, int mport, int time_out){
    //doc
    m_doc="db.col";
    wr_timeout=(double)time_out;
    //最大连接0-200
    if(pool_size<0){
        pool_size=1;
    }
    if(pool_size>200){
        pool_size=200;
    }
    if(!con_queue.empty()>0){
        return;
    }
    bool auto_conn=true;
    pthread_mutex_init(&_jobmux,NULL);
    if((sem_init(&_jobsem,0,0))<0){
        return;
    }
    pthread_mutex_lock(&_jobmux);
    for(int i=0;i<pool_size;++i){
        DBClientConnection* pconn = new DBClientConnection(auto_conn,0,wr_timeout);
        if(!pconn) {
            cerr<<"new DBClientConnection fail";
            exit(0);
        }
        HostAndPort hp(mhost,mport);
        string errmsg="";
        if (!pconn->connect(hp,errmsg)) {
            cerr<<"connect mhost:"<<mhost<<" mport:"<<mport<<" msg:"<<errmsg<<endl;
            exit(0);
        }
        con_queue.push_back(pconn);
        sem_post(&_jobsem);
    }
    pthread_mutex_unlock(&_jobmux);
}

CmongoPool::~CmongoPool(){
    m_doc="";
    pthread_mutex_lock(&_jobmux);
    while(!con_queue.empty()){
        DBClientConnection* p = con_queue.back();
        delete p;
        con_queue.pop_back();
    }
    while(!con_queue_using.empty()){
        DBClientConnection* p = con_queue_using.back();
        delete p;
        con_queue_using.pop_back();
    }
    pthread_mutex_unlock(&_jobmux);
}

int CmongoPool::set_wr_timeout(double t){
    wr_timeout=t;
    return RET_OK;
}

int CmongoPool::setdb(string mdb,string mcollection){
    if(mdb.empty() || mcollection.empty()){
        return RET_PARERR;
    }
    m_doc=mdb+"."+mcollection;
    return RET_OK;
}
int CmongoPool::setindex(string& doc, string key){
    if(key.empty()){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p = NULL;
    while(!con_queue.empty()){
        p = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p) {
        return RET_ERR;
    }
    string bindex="{"+key+":1}";
    p->ensureIndex(doc,fromjson(bindex));

    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return RET_OK;
}

//out为检索出来的key-value数据对应，in 为要检索的字段,key,key_value为要检索的条件,暂不支持多条件检索
//单列查询
int CmongoPool::get(string& doc, map<string,string>& out,vector<string> in,string key,string key_val){
    //key key_val 要检索字段
    if(key.empty() || key_val.empty() || in.size()<=0){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }
    BSONObjBuilder b;
    for(vector<string>::iterator iter=in.begin();iter!=in.end();++iter){
        b.append(*iter,1);
    }

    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p_con) {
        return RET_ERR;
    }
    BSONObj ob=b.obj();

    BSONObj p=p_con->findOne(doc,QUERY(key<<key_val),&ob);

    for(vector<string>::iterator iter=in.begin();iter!=in.end();++iter){
        string mkey=*iter;
        out[*iter]=p.getStringField(mkey.c_str());
    }

    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return RET_OK;
}

//查询key为key的一批数据的 某些字段
//fields为要查询的字段集
//key="id" 值为in 一批key
//返回key->map<key,value>
int CmongoPool::gets(string& doc, map< string,map<string,string> >& rout,vector<string> fields,vector<string> in,string key){
    if(key.empty()){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p_con) {
        return RET_ERR;
    }

    BSONObjBuilder b;
    b.append(key,1);
    for(vector<string>::iterator iter=fields.begin();iter!=fields.end();++iter){
        b.append(*iter,1);
    }

    BSONObj p=b.obj();
    for(vector<string>::iterator iter2=in.begin();iter2!=in.end();++iter2){
        BSONObj ob=p_con->findOne(doc,QUERY(key<<*iter2),&p);
        map<string,string> temp;
        for(vector<string>::iterator iter=fields.begin();iter!=fields.end();++iter){
            string mkey=*iter;
            temp[*iter]=ob.getStringField(mkey.c_str());
        }
        rout[ob.getStringField(key.c_str())]=temp;
    }

    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return RET_OK;
}

//dumpkey key-value 返回 key对应的val值
//key val
int CmongoPool::dumpkey(string& doc, map< string,string >& rout,string key,string val){
    if(key.empty()){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p_con) {
        return RET_ERR;
    }

    BSONObjBuilder b;
    b.append(key,1);
    if(!val.empty()){
        b.append(val,1);
    }

    BSONObj p=b.obj();

    pthread_mutex_lock(&_dbmux);
    auto_ptr<DBClientCursor> cursor = p_con->query(doc,Query(),0,0,&p);
    while(cursor->more()){
        BSONObj ob=cursor->next();
        rout[ob.getStringField(key.c_str())]=ob.getStringField(val.c_str());
    }
    pthread_mutex_unlock(&_dbmux);

    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return RET_OK;
}

//dumpkey key对应多个value
//key->map<key,value>.
//其实dumpvals接口完全可以包含dumpkey，为了方便运用独立出来
//out 返回的key 对应的map<key,value>
//in 每个key需要对应的返回哪些字段
//key="id"
int CmongoPool::dumpvals(string& doc, map< string,map<string,string> >& rout,vector<string> in,string key){
    if(key.empty()){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p_con) {
        return RET_ERR;
    }

    BSONObjBuilder b;
    b.append(key,1);
    for(vector<string>::iterator iter=in.begin();iter!=in.end();++iter){
        b.append(*iter,1);
    }

    BSONObj p=b.obj();

    pthread_mutex_lock(&_dbmux);
    auto_ptr<DBClientCursor> cursor = p_con->query(doc,Query(),0,0,&p);
    while(cursor->more()){
        BSONObj ob=cursor->next();
        map<string,string> temp;
        for(vector<string>::iterator iter=in.begin();iter!=in.end();++iter){
            string val=*iter;
            temp[val]=ob.getStringField(val.c_str());
        }
        rout[ob.getStringField(key.c_str())]=temp;
        temp.clear();
    }
    pthread_mutex_unlock(&_dbmux);

    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return RET_OK;
}

DBClientConnection* CmongoPool::get_con() {
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    return p_con;
}

bool CmongoPool::release(DBClientConnection* p_con) {
    if (!p_con) {
        return false;
    }
    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);
    return true;
}

DBClientConnection* CmongoPool::get_con_nowait() {
    sem_trywait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    return p_con;
}

//更新接口，暂不支持key对应多条记录的更新
int CmongoPool::set(string& doc, map<string,string> in,string key,string key_val){
    //如果map没有数据，返回参数错误
    if(in.size()<=0 || key.empty() || key_val.empty()){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }
    BSONObjBuilder b;
    map<string,string>::iterator iter;
    for(iter=in.begin();iter!=in.end();++iter){
        b.append(iter->first,iter->second);
    }

    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p_con) {
        return RET_ERR;
    }

    BSONObj ob=b.obj();
    p_con->update(doc,QUERY(key<<key_val),BSON("$set"<<ob),true);

    int ret=RET_OK;
    string errmsg=p_con->getLastError();
    if(!errmsg.empty()){
        ret=RET_ERR;
    }

    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return ret;
}
//更新接口，批量更新key="id"
//  "123456":<key,value>,<key,value>
//  "123457":<key,value>,<key,value>
int CmongoPool::sets(string& doc, map< string,map<string,string> > in,string key){
    //如果map没有数据，返回参数错误
    if(in.size()<=0 || key.empty() ){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }

    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p_con) {
        return RET_ERR;
    }

    int ret=RET_OK;
    map< string,map<string,string> >::iterator iter;
    for(iter=in.begin();iter!=in.end();++iter){
        BSONObjBuilder b;
        for(map<string,string>::iterator iter2=iter->second.begin();iter2!=iter->second.end();++iter2){
            b.append(iter2->first,iter2->second);
        }
        BSONObj ob=b.obj();
        p_con->update(doc,QUERY(key<<iter->first),BSON("$set"<<ob),true);
        string errmsg=p_con->getLastError();
        if(!errmsg.empty()){
            ret=RET_ERR;
        }
    }


    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return ret;
}
//删除接口，删除记录 key=id key_val=587.即删除id="587"的记录
int CmongoPool::remove(string& doc, string key,string key_val){
    if(key.empty() || key_val.empty()){
        return RET_PARERR;
    }
    if (doc.empty()) {
        doc = m_doc;
    }
    sem_wait(&_jobsem);
    pthread_mutex_lock(&_jobmux);
    DBClientConnection* p_con = NULL;
    while(!con_queue.empty()){
        p_con = con_queue.back();
        con_queue.pop_back();
        break;
    }
    pthread_mutex_unlock(&_jobmux);
    if (!p_con) {
        return RET_ERR;
    }

    p_con->remove(doc,BSON(key << key_val));

    pthread_mutex_lock(&_jobmux);
    con_queue.push_back(p_con);
    pthread_mutex_unlock(&_jobmux);
    sem_post(&_jobsem);

    return RET_OK;
}

#endif
