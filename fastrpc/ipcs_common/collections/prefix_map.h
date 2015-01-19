// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description: prefix map

#ifndef IPCS_COMMON_COLLECTION_PREFIX_MAP_H
#define IPCS_COMMON_COLLECTION_PREFIX_MAP_H

#include <stdint.h>
#include <string.h>

#include <cassert>
#include <functional>
#include <map>
#include <utility>

#include "string_piece.h"
#include "md5.h"
#include "logging.h"

namespace ipcs_common {

struct PrefixMap_DefaultCleaner
{
    template<typename T>
    static void Delete(T p) {}
};

struct PrefixMap_PointerCleaner
{
    template<typename T>
    static void Delete(T p) {
        delete p;
    }
};

template <typename MappedType,
         typename Cleaner = PrefixMap_DefaultCleaner>
class PrefixMap
{
    struct PrefixValue
    {
        bool inclusive;
        MappedType value;
    };
    typedef std::map<uint64_t, PrefixValue> MapType;

public:
    ~PrefixMap()
    {
        typename std::map<uint64_t, PrefixValue>::iterator iter = m_map.begin();
        for (; iter != m_map.end(); ++iter) {
            m_cleaner.Delete(iter->second.value);
        }
    }
    // Insert prefix string, include the string itself.
    // Return true if the string not exist, false if already exist.
    // Note: new string will cover the old string.
    bool InsertInclusive(const StringPiece& str, const MappedType& value)
    {
        return InternalInsert(str, value, true);
    }

    // Insert prefix string, exclude the string itself.
    // Return true if the string not exist, false if already exist.
    // Note: new string will cover the old string.
    bool InsertExclusive(const StringPiece& str, const MappedType& value)
    {
        return InternalInsert(str, value, false);
    }

    // remove prefix
    bool Remove(const StringPiece& str)
    {
        uint64_t digest = GetDigest(str);
        typename MapType::iterator i = m_map.find(digest);
        if (i != m_map.end()) {
            m_cleaner.Delete(i->second.value);
            m_map.erase(i);
            typename LengthMapType::iterator j = m_length_map.find(str.size());
            assert(j != m_length_map.end());
            if (--j->second == 0)
                m_length_map.erase(j);
            return true;
        }
        return false;
    }

    bool Find(const StringPiece& str, MappedType* result) const
    {
        return FindMatchLength(str, result) >= 0;
    }

    // return matched length
    int FindMatchLength(const StringPiece& str, MappedType* result) const
    {
        if (!m_length_map.empty()) {
            typename LengthMapType::const_iterator i = m_length_map.lower_bound(str.size());
            if (i == m_length_map.end())
                return -1;

            size_t prefix_length = i->first;
            uint64_t digest;
            typename MapType::const_iterator j;

            if (prefix_length == str.size()) {
                digest = GetDigest(str);
                j = m_map.find(digest);
                if (j != m_map.end() && j->second.inclusive) {
                    *result = j->second.value;
                    return prefix_length;
                }
            }

            for (; i != m_length_map.end(); ++i) {
                prefix_length = i->first;
                digest = GetDigest(str.substr(0, prefix_length));
                j = m_map.find(digest);
                if (j != m_map.end()) {
                    *result = j->second.value;
                    return prefix_length;
                }
            }
        }
        return -1;
    }

    void Clear()
    {
        m_length_map.clear();
        m_map.clear();
    }

    bool Empty()
    {
        assert(m_length_map.empty() == m_map.empty());
        return m_length_map.empty();
    }

private:
    static uint64_t GetDigest(const StringPiece& str)
    {
        return ipcs_common::MD5::Digest64(str.data(), str.size());
    }

    // Internal insert
    bool InternalInsert(const StringPiece& str, const MappedType& value, bool inclusive)
    {
        uint64_t digest = GetDigest(str);
        PrefixValue pv = {inclusive, value};
        std::pair<typename MapType::iterator, bool> r =
            m_map.insert(std::make_pair(digest, pv));

        if (r.second) {
            ++m_length_map[str.size()];
        }  else {
            (*r.first).second.inclusive = inclusive;
            (*r.first).second.value = value;
        }
        return r.second;
    }

    Cleaner m_cleaner;
    MapType m_map;

    typedef std::map<size_t, int, std::greater<size_t> > LengthMapType;
    LengthMapType m_length_map;
};

#endif // IPCS_COMMON_COLLECTION_PREFIX_MAP_H

} // namespace ipcs_common
