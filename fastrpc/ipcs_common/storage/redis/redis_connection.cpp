// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 06/13/13
// Description: redis connection impl

#include "redis_connection.h"
#include <sstream>
#include "logging.h"

namespace ipcs_common {

int CRedisConnection::Connect(const std::string strHost, int nPort,
                              int nTimeOut, int nRedisSocketTimeout, int table_index) {
    if (strHost.empty() || nPort <= 0) {
        return -1;
    }

    m_strHost = strHost;
    m_nPort = nPort;
    m_nTimeOut = nTimeOut;
    m_nSocketTimeout = nRedisSocketTimeout;
    m_table_index = table_index;

    return ReConnect(table_index);
}

int CRedisConnection::ReConnect(int table_index) {
    DisConnect();

    if (m_nTimeOut <= 0) {
        m_pRedisConn = redisConnect(m_strHost.c_str(), m_nPort);
    } else {
        struct timeval timeOut;
        timeOut.tv_sec = m_nTimeOut;
        timeOut.tv_usec = (m_nTimeOut % 1000) * 1000;
        m_pRedisConn = redisConnectWithTimeout(m_strHost.c_str(), m_nPort, timeOut);
    }

    if (NULL == m_pRedisConn || m_pRedisConn->err) {
        LOG(ERROR) << "connect to redis server "
                   << m_strHost << ":" << m_nPort
                   << " failed : " << m_pRedisConn->errstr;
        return -1;
    }

    if (m_nSocketTimeout > 0) {
        struct timeval timeOut;
        timeOut.tv_sec = m_nSocketTimeout;
        timeOut.tv_usec = (m_nSocketTimeout % 1000) * 1000;
        redisSetTimeout(m_pRedisConn, timeOut);
    }

    std::stringstream ss;
    ss << m_table_index;
    std::string select_table = "select " + ss.str();
    int cmd_ret = this->SafeCommand(select_table.c_str());

    LOG(INFO) << "connect to redis server "
              << m_strHost << ":" << m_nPort << " successfully"
              << ", select table : " << table_index
              << ", select table result : " << cmd_ret;

    return 0;
}

void CRedisConnection::DisConnect() {
    if (NULL != m_pRedisConn) {
        redisFree(m_pRedisConn);
        m_pRedisConn = NULL;
    }
}

int CRedisConnection::GetSetCommand(const char* strCommand, std::vector<std::string>* set) {
    if (NULL == strCommand) {
        LOG(ERROR) << "redis command is nil";
        return -1;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisConn, strCommand);
    if (NULL != pReply) {
        if (pReply->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i < pReply->elements; ++i) {
                redisReply* child = pReply->element[i];
                if (child->type == REDIS_REPLY_STRING) {
                    set->push_back(std::string(child->str));
                }
            }
        }
        freeReplyObject(pReply);
        return 0;
    }
    return -2;
}

int CRedisConnection::SafeGetSetCommand(const char* strCommand, std::vector<std::string>* set) {
    if (NULL == strCommand) {
        LOG(ERROR) << "redis command is nil";
        return -1;
    }

    int nRet = GetSetCommand(strCommand, set);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed in safe command";
            return -1;
        }

        return GetSetCommand(strCommand, set);
    }
    return nRet;
}

int CRedisConnection::Command(const char* strCommand) {
    if (NULL == strCommand) {
        LOG(ERROR) << "redis command is nil";
        return -1;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisConn, strCommand);
    if (NULL != pReply) {
        freeReplyObject(pReply);
        return 0;
    }
    return -2;
}

int CRedisConnection::SafeCommand(const char* strCommand) {
    if (NULL == strCommand) {
        LOG(ERROR) << "redis command is nil";
        return -1;
    }

    int nRet = Command(strCommand);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed in safe command";
            return -1;
        }

        return Command(strCommand);
    }

    return nRet;
}

void CRedisConnection::FreeReply(redisReply* pReply) {
    if (NULL == pReply) {
        LOG(WARNING) << "free nil redis reply";
        return;
    }

    freeReplyObject(pReply);
}

const redisContext* CRedisConnection::GetRedisConnection() const {
    return m_pRedisConn;
}

int CRedisConnection::SafeSet(const char* pKey, const int& nKeyLen,
                              const char* pValue, const int& nValLen) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    int nRet = Set(pKey, nKeyLen, pValue, nValLen);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return Set(pKey, nKeyLen, pValue, nValLen);
    }

    return nRet;
}

int CRedisConnection::Set(const char* pKey,const int& nKeyLen,
                          const char* pValue,const int& nValLen) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"SET %b %b",pKey,nKeyLen,pValue,nValLen);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    int nErrNo = -1;
    if (REDIS_REPLY_STATUS == pReply->type && 0 == strcmp(pReply->str, "OK")) {
        nErrNo = 0;
    } else {
        LOG(ERROR) << "set key value " << pKey << ":" << pValue
                   << ", error info: " << pReply->str;
    }

    freeReplyObject(pReply);
    return nErrNo;
}

int CRedisConnection::SafeSetStr(const char* pKey, const char* pValue) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    int nRet = SetStr(pKey, pValue);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return SetStr(pKey,pValue);
    }

    return nRet;
}

int CRedisConnection::SetStr(const char* pKey, const char* pValue) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn, "SET %s %s", pKey, pValue);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -1;
    }

    int nErrNo = -1;
    if (REDIS_REPLY_STATUS == pReply->type && 0 == strcmp(pReply->str, "OK")) {
        nErrNo = 0;
    } else {
        LOG(ERROR) << "set key value " << pKey << ":" << pValue
                   << ", error info: " << pReply->str;
    }

    freeReplyObject(pReply);
    return nErrNo;
}

int CRedisConnection::SafeGet(const char* pKey, char* pValue, int &nValLen) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    int nRet = Get(pKey,pValue,nValLen);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return Get(pKey,pValue,nValLen);
    }

    return nRet;
}

int CRedisConnection::Get(const char* pKey, char* pValue, int &nValLen) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"GET %s", pKey);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    if (pReply->type == REDIS_REPLY_STRING) {
        if (nValLen >= pReply->len) {
            memmove(pValue, pReply->str, pReply->len);
            nValLen = pReply->len;

            freeReplyObject(pReply);
            return 0;
        } else {
            LOG(ERROR) << "get key " << pKey << " failed, length " << pReply->len
                       << " replied but " << nValLen << " should get";
            freeReplyObject(pReply);
            return -3;
        }
    } else if (pReply->type == REDIS_REPLY_NIL) {
        LOG(WARNING) << "get key " << pKey << ", nil result, error info: "
                     << pReply->str;
        freeReplyObject(pReply);
        return -4;
    } else {
        LOG(ERROR) << "get key " << pKey << ", error info: " << pReply->str;
        freeReplyObject(pReply);
        return -4;
    }
    return 0;
}

int CRedisConnection::SafeGetStr(const char* pKey, const int &nKeyLen,
                                 char* pValue, int &nValLen) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    int nRet = GetStr(pKey, nKeyLen, pValue, nValLen);
    if (nRet == -2) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return GetStr(pKey,nKeyLen,pValue,nValLen);
    }

    return nRet;
}

int CRedisConnection::GetStr(const char* pKey, const int &nKeyLen,
                             char* pValue, int &nValLen) {
    if (NULL == pKey || NULL == pValue) {
        LOG(ERROR) << "pKey or pValue is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"GET %b",pKey,nKeyLen);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    if (pReply->type == REDIS_REPLY_STRING) {
        if (nValLen >= pReply->len) {
            memmove(pValue, pReply->str, pReply->len);
            nValLen = pReply->len;

            freeReplyObject(pReply);
            return 0;
        } else {
            LOG(ERROR) << "get key " << pKey << " failed, length " << pReply->len
                       << " replied, but " << nValLen << " should be returned";
            freeReplyObject(pReply);
            return -3;
        }
    } else if (pReply->type == REDIS_REPLY_NIL) {
        LOG(WARNING) << "get key " << pKey  << ", with no result";
        freeReplyObject(pReply);
        return -4;
    } else {
        LOG(ERROR) << "get key " << pKey << ", error info : "
                   << pReply->str;
        freeReplyObject(pReply);
        return -5;
    }
    return -1;
}

int CRedisConnection::SafeHashGet(const char* pHashKey, const int& nHashKeyLen,
                                  const char* pFieldKey, const int& nFieldKeyLen,
                                  char* pFieldVal, int& nFieldValLen) {
    if (NULL == pHashKey || NULL == pFieldKey || NULL == pFieldVal) {
        LOG(ERROR) << "pHashKey || pFieldKey || pFieldVal is nil";
        return -1;
    }

    int nRet = HashGet(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,pFieldVal,nFieldValLen);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return HashGet(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,pFieldVal,nFieldValLen);
    }

    return nRet;
}

int CRedisConnection::HashGet(const char* pHashKey, const int& nHashKeyLen,
                              const char* pFieldKey, const int& nFieldKeyLen,
                              char* pFieldVal, int& nFieldValLen) {
    if (NULL == pHashKey || NULL == pFieldKey || NULL == pFieldVal) {
        LOG(ERROR) << "pHashKey || pFieldKey || pFieldVal is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"HGET %b %b",pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    if (pReply->type == REDIS_REPLY_STRING) {
        if (nFieldValLen >= pReply->len) {
            memmove(pFieldVal, pReply->str, pReply->len);
            nFieldValLen = pReply->len;

            freeReplyObject(pReply);
            return 0;
        } else {
            LOG(ERROR) << "HashGet failed, hash key : " << pHashKey << ", field key : "
                       << pFieldKey;
            freeReplyObject(pReply);
            return -3;
        }
    } else {
        LOG(ERROR) << "HashGet failed, hash key : " << pHashKey << ", field key : "
                   << pFieldKey;
        freeReplyObject(pReply);
        return -3;
    }

    return -1;
}

int CRedisConnection::SafeHashSet(const char* pHashKey, const int& nHashKeyLen,
                                  const char* pFieldKey, const int& nFieldKeyLen,
                                  const char* pFieldVal, const int& nFieldValLen) {
    if (NULL == pHashKey || NULL == pFieldKey || NULL == pFieldVal) {
        LOG(ERROR) << "pHashKey || pFieldKey || pFieldVal is nil";
        return -1;
    }

    int nRet = HashSet(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,pFieldVal,nFieldValLen);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -1;
        }

        return HashSet(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,pFieldVal,nFieldValLen);
    }

    return nRet;
}

int CRedisConnection::HashSet(const char* pHashKey, const int& nHashKeyLen,
                              const char* pFieldKey, const int& nFieldKeyLen,
                              const char* pFieldVal, const int& nFieldValLen) {
    if (NULL == pHashKey || NULL == pFieldKey || NULL == pFieldVal) {
        LOG(ERROR) << "pHashKey || pFieldKey || pFieldVal is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"HSET %s %s %b",
                                                   pHashKey, pFieldKey,
                                                   pFieldVal, nFieldValLen);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    int nErrNo = -1;
    if (REDIS_REPLY_INTEGER == pReply->type) {
        nErrNo = 0;
    } else {
        LOG(ERROR) << "HashSet HashKey : " << pHashKey
                   << " pFieldKey : " << pFieldKey
                   << " pFieldVal : " << pFieldVal
                   << " error info: " << pReply->str;
    }

    freeReplyObject(pReply);
    return nErrNo;
}

int CRedisConnection::SafeHashSetNum(const char* pHashKey, const int& nHashKeyLen,
                                     const char* pFieldKey, const int& nFieldKeyLen,
                                     const int& nFieldVal) {
    if (NULL == pHashKey || NULL == pFieldKey) {
        LOG(ERROR) << "pHashKey || pFieldKey";
        return -1;
    }

    int nRet = HashSetNum(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,nFieldVal);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return HashSetNum(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,nFieldVal);
    }

    return nRet;
}

int CRedisConnection::HashSetNum(const char* pHashKey, const int& nHashKeyLen,
                                 const char* pFieldKey, const int& nFieldKeyLen,
                                 const int& nFieldVal) {
    if (NULL == pHashKey || NULL == pFieldKey) {
        LOG(ERROR) << "pHashKey || pFieldKey";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"HSET %b %b %d",\
        pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,nFieldVal);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    int nErrNo = -1;
    if (REDIS_REPLY_INTEGER == pReply->type) {
        nErrNo = 0;
    } else {
        LOG(ERROR) << "HashSet hashkey : " << pHashKey
                   << " pFieldKey : " << pFieldKey
                   << " nFieldVal : " << nFieldVal
                   << " error info : " << pReply->str;
    }

    freeReplyObject(pReply);
    return nErrNo;
}

int CRedisConnection::HashKeys(
                 const char* pHashKey, const int& nHashKeyLen,
                 std::vector<std::string>* hkeys) {
    if (pHashKey == NULL || hkeys == NULL) {
        LOG(ERROR) << "pHashKey || hkeys is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn, "HKEYS %b", pHashKey, nHashKeyLen);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    if (pReply->type == REDIS_REPLY_ARRAY) {
        for (unsigned int i = 0; i < pReply->elements; ++i) {
            redisReply* child_reply = pReply->element[i];
            if (child_reply->type == REDIS_REPLY_STRING) {
                hkeys->push_back(std::string(child_reply->str));
            }
        }

        freeReplyObject(pReply);
        return 0;
    } else {
        LOG(ERROR) << "HKEYS failed, hash key : " << pHashKey;
        freeReplyObject(pReply);
        return -3;
    }

    return -1;
}

int CRedisConnection::SafeHashKeys(
                     const char* pHashKey, const int& nHashKeyLen,
                     std::vector<std::string>* hkeys) {
    hkeys->clear();
    int nRet = HashKeys(pHashKey, nHashKeyLen, hkeys);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnecting failed";
            return -2;
        }
        return HashKeys(pHashKey, nHashKeyLen, hkeys);
    }
    return nRet;
}

int CRedisConnection::HashDelKey(
                   const char* pHashKey, const int& nHashKeyLen,
                   const char* pFieldKey, const int& nFieldKeyLen) {
    if (pHashKey == NULL || pFieldKey == NULL) {
        LOG(ERROR) << "pHashKey || pFieldKey is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn, "HDEL %b %b",
                                                   pHashKey, nHashKeyLen,
                                                   pFieldKey, nFieldKeyLen);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }
    int error_no = -1;
    if (pReply->type == REDIS_REPLY_INTEGER) {
        error_no = 0;
    } else {
        LOG(ERROR) << "HDEL returns not integer object";
        error_no = -3;
    }
    freeReplyObject(pReply);
    return error_no;
}

int CRedisConnection::SafeHashDelKey(
                       const char* pHashKey, const int& nHashKeyLen,
                       const char* pFieldKey, const int& nFieldKeyLen) {
    int nRet = HashDelKey(pHashKey, nHashKeyLen, pFieldKey, nFieldKeyLen);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnecting failed";
            return -1;
        }
        return HashDelKey(pHashKey, nHashKeyLen, pFieldKey, nFieldKeyLen);
    }
    return nRet;
}

int CRedisConnection::SafeHashIncr(const char* pHashKey, const int& nHashKeyLen,
                                   const char* pFieldKey, const int& nFieldKeyLen,
                                   const int& nIncrValue) {
    if (NULL == pHashKey || NULL == pFieldKey) {
        LOG(ERROR) << "pHashKey || pFieldKey || pFieldVal is nil";
        return -1;
    }

    int nRet = HashIncr(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,nIncrValue);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return HashIncr(pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,nIncrValue);
    }

    return nRet;
}

int CRedisConnection::HashIncr(const char* pHashKey, const int& nHashKeyLen,
                               const char* pFieldKey, const int& nFieldKeyLen,
                               const int& nIncrValue) {
    if (NULL == pHashKey || NULL == pFieldKey) {
        LOG(ERROR) << "pHashKey || pFieldKey || pFieldVal is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"HINCRBY %b %b %d",\
        pHashKey,nHashKeyLen,pFieldKey,nFieldKeyLen,nIncrValue);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is NULL";
        return -2;
    }

    int nErrNo = -1;
    if (REDIS_REPLY_INTEGER == pReply->type) {
        nErrNo = 0;
    } else {
        LOG(ERROR) << "HashIncr hashkey : " << pHashKey
                   << " pFieldKey : " << pFieldKey
                   << " error info : " << pReply->str;
    }

    freeReplyObject(pReply);
    return nErrNo;
}

int CRedisConnection::SafeZsetIncr(const char* pZsetKey, const int &nZsetKeyLen,
                                   const char* pMember, const int &nMemberLen,
                                   const int& nIncrValue) {
    if (NULL == pZsetKey || NULL == pMember){
        LOG(ERROR) << "pZsetKey || pMember is nil";
        return -1;
    }

    int nRet = ZsetIncr(pZsetKey,nZsetKeyLen,pMember,nMemberLen,nIncrValue);
    if (nRet < 0) {
        if (ReConnect() < 0) {
            LOG(ERROR) << "reconnect failed";
            return -2;
        }

        return ZsetIncr(pZsetKey,nZsetKeyLen,pMember,nMemberLen,nIncrValue);
    }

    return nRet;
}

int CRedisConnection::ZsetIncr(const char* pZsetKey, const int &nZsetKeyLen,
                               const char* pMember, const int &nMemberLen,
                               const int& nIncrValue) {
    if (NULL == pZsetKey || NULL == pMember) {
        LOG(ERROR) << "pZsetKey || pMember is nil";
        return -1;
    }

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisConn,"ZINCRBY %b %d %b",\
        pZsetKey,nZsetKeyLen,nIncrValue,pMember,nMemberLen);
    if (NULL == pReply) {
        LOG(ERROR) << "pReply is nil";
        return -2;
    }

    int nErrNo = -1;
    if (REDIS_REPLY_STRING == pReply->type) {
        nErrNo = 0;
    } else {
        LOG(ERROR) << "HashSetIncr setkey : " << pZsetKey
                   << " member : " << pMember;
    }

    freeReplyObject(pReply);
    return nErrNo;
}

int CRedisConnection::SetProtoMessage(
        const std::string& key,
        const google::protobuf::Message& message_value,
        bool dump_debug_string,
        const std::string& debug_prefix) {
    int sz = message_value.ByteSize();
    char* array = new char[sz];
    bzero(array, sz);

    bool serialized = message_value.SerializeToArray(array, sz);
    if (!serialized) {
        delete[] array;
        return -1;
    }

    int cmd_ret = this->SafeSet(key.c_str(), key.size(), array, sz);
    if (cmd_ret != 0) {
        delete[] array;
        return -2;
    }

    if (dump_debug_string) {
        const std::string& debug_key = debug_prefix + key;
        std::string debug_content = message_value.ShortDebugString();
        cmd_ret = this->SafeSetStr(debug_key.c_str(), debug_content.c_str());
        if (cmd_ret != 0) {
            delete[] array;
            return -3;
        }
    }

    delete[] array;
    return 0;
}

int CRedisConnection::ReadProtoMessage(
        const std::string& key,
        char* bytes_out,
        int& bytes_length_out) {
    int err = 0;
    int cmd_ret = this->SafeGetStr(key.c_str(), key.size(),
                                   bytes_out, bytes_length_out);
    if (cmd_ret != 0) {
        err = cmd_ret;
    }
    return err;
}

int CRedisConnection::ReadProtoMessage(
        const std::string& key,
        google::protobuf::Message* message,
        int default_max_message_size) {
    int err = 0;
    char* buf = new char[default_max_message_size + 1];
    bzero(buf, default_max_message_size + 1);
    int sz = default_max_message_size;
    err = this->ReadProtoMessage(key, buf, sz);
    if (err == 0) {
        message->ParseFromArray(buf, sz);
    }
    delete[] buf;
    return err;
}

} // namespace ipcs_common

