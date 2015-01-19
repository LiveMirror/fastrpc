// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 06/13/13
// Description: redis

#ifndef IPCS_COMMON_STORAGE_REDIS_REDIS_CONNECTION_H
#define IPCS_COMMON_STORAGE_REDIS_REDIS_CONNECTION_H
#pragma once

#include "hiredis.h"
#include <vector>
#include <string>
#include "message.h"

namespace ipcs_common {

class CRedisConnection {
public:
    enum RedisResult {
        kOk = 0,
        kNullParam,
        kNoResult,
        kError
    };

public:
    CRedisConnection() :
                     m_strHost(""),
                     m_nPort(0),
                     m_nTimeOut(0),
                     m_nSocketTimeout(0),
                     m_pRedisConn(NULL) {}

    CRedisConnection(const std::string host,
                     int port,
                     int timeout,
                     int socket_timeout) :
                     m_strHost(host),
                     m_nPort(port),
                     m_nTimeOut(timeout),
                     m_nSocketTimeout(socket_timeout),
                     m_pRedisConn(NULL) {
    }

    virtual ~CRedisConnection() {}

    int Connect(const std::string host,
                int port,
                int timeout,
                int socket_timeout,
                int table_index=0);

    void DisConnect();

    int GetSetCommand(const char* strCommand, std::vector<std::string>* set);

    int SafeGetSetCommand(const char* strCommand, std::vector<std::string>* set);

    /// execute command
    int Command(const char* strCommand);

    /// support reconnecting to execute command once disconnected
    int SafeCommand(const char* strCommand);

    /// free redis reply result
    void FreeReply(redisReply* pReply);

    /// get redis connection context
    const redisContext* GetRedisConnection() const;

    /// set binary key-value
    int Set(const char* pKey, const int& nKeyLen,
            const char* pValue, const int& nValLen);

    /// safe set
    int SafeSet(const char* pKey, const int& nKeyLen,
                const char* pValue,const int& nValLen);

    /// set string key-value
    int SetStr(const char* pKey, const char* pValue);

    /// support reconnecting to set string key-value
    int SafeSetStr(const char* pKey, const char* pValue);

    /// binary get value from key
    int Get(const char* pKey, char* pValue, int &nValLen);

    /// reconnecting binary get value once disconected
    int SafeGet(const char* pKey, char* pValue, int &nValLen);

    /// get string from key
    int GetStr(const char* pKey, const int &nKeyLen, char* pValue, int &nValLen);

    /// support reconnecting
    int SafeGetStr(const char* pKey, const int &nKeyLen, char* pValue, int &nValLen);

    /// set hashtable for one field
    int HashSet(const char* pHashKey, const int& nHashKeyLen,
                const char* pFieldKey, const int& nFieldKeyLen,
                const char* pFieldVal, const int& nFieldValLen);

    /// safe set
    int SafeHashSet(const char* pHashKey, const int& nHashKeyLen,
                    const char* pFieldKey, const int& nFieldKeyLen,
                    const char* pFieldVal, const int& nFieldValLen);

    /// set hashtable for integer key
    int HashSetNum(const char* pHashKey, const int& nHashKeyLen,
                   const char* pFieldKey, const int& nFieldKeyLen,
                   const int& nFieldVal);

    /// reconnecting set
    int SafeHashSetNum(const char* pHashKey, const int& nHashKeyLen,
                       const char* pFieldKey, const int& nFieldKeyLen,
                       const int& nFieldVal);

    /// get one key from hashtable
    int HashGet(const char* pHashKey, const int& nHashKeyLen,
                const char* pFieldKey, const int& nFieldKeyLen,
                char* pFieldVal, int& nFieldValLen);

    /// safe get
    int SafeHashGet(const char* pHashKey, const int& nHashKeyLen,
                    const char* pFieldKey, const int& nFieldKeyLen,
                    char* pFieldVal, int& nFieldValLen);

    /// get hash all keys
    int HashKeys(const char* pHashKey, const int& nHashKeyLen,
                 std::vector<std::string>* hkeys);

    int SafeHashKeys(const char* pHashKey, const int& nHashKeyLen,
                     std::vector<std::string>* hkeys);

    /// hash del key
    int HashDelKey(const char* pHashKey, const int& nHashKeyLen,
                   const char* pFieldKey, const int& nFieldKeyLen);

    int SafeHashDelKey(const char* pHashKey, const int& nHashKeyLen,
                       const char* pFieldKey, const int& nFieldKeyLen);

    /// increase one field in hashtable, default by 1
    int HashIncr(const char* pHashKey, const int& nHashKeyLen,
                 const char* pFieldKey, const int& nFieldKeyLen,
                 const int& nIncrValue = 1);

    /// safe increase
    int SafeHashIncr(const char* pHashKey, const int& nHashKeyLen,
                     const char* pFieldKey, const int& nFieldKeyLen,
                     const int& nIncrValue = 1);

    /// increase one field in set
    int ZsetIncr(const char* pZsetKey, const int &nZsetKeyLen,
                 const char* pMember, const int &nMemberLen,
                 const int& nIncrValue = 1);

    /// safe increase
    int SafeZsetIncr(const char* pZsetKey,const int &nZsetKeyLen,
        const char* pMember,const int &nMemberLen,const int& nIncrValue = 1);

    ///
    /// returns 0 - succ
    ///        -1 - failed to serialize proto message
    ///        -2 - failed to write to redis cache
    ///        -3 - failed to write debug message
    int SetProtoMessage(
        const std::string& key,
        const google::protobuf::Message& message_value,
        bool dump_debug_string = true,
        const std::string& debug_prefix = "DEBUG_");

    ///
    /// returns 0  - succ
    ///        -1  - failed to get message for key
    ///       bytes_out - should be passed into it, should be allocated size outside
    ///bytes_length_out - should be passed into it, should be larger enough,
    ///                  the actual value will be returned.
    /// NOTE!!! - the value of bytes_length_out that passed into
    ///           this method should be larger than that of the
    ///           protocol buffer message.
    int ReadProtoMessage(
        const std::string& key,
        char* bytes_out,
        int& bytes_length_out);

    ///
    /// returns 0 - succ
    ///        -1 - failed to get message for key
    /// user should set the default max message size right !!!
    int ReadProtoMessage(
        const std::string& key,
        google::protobuf::Message* message,
        int default_max_message_size = 0);
private:
    int ReConnect(int table_index=0);

private:
    std::string m_strHost;      /// host ip
    int m_nPort;                /// redis host port
    int m_nTimeOut;             /// timeout
    int m_nSocketTimeout;       /// socket timeout
    int m_table_index;          /// table index
    redisContext* m_pRedisConn; /// redis connection object context
};

} // namespace ipcs_common

#endif // IPCS_COMMON_STORAGE_REDIS_REDIS_CONNECTION_H
