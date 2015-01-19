// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#ifndef IPCS_COMMON_TEXT_WILDCARD_H
#define IPCS_COMMON_TEXT_WILDCARD_H

#include <string>

/// wildcard match static class
struct Wildcard
{
private:
    Wildcard();
    ~Wildcard();
public:
    enum MATCH_FLAGS
    {
        /// wildcards in pattern cannot match `/', match filename only
        /// *.c not match dir/name.c
        MATCH_FILE_NAME_ONLY = 1<<0,

        /// Don't treat the `\' character as escape
        NO_ESCAPE = 1<<1,

        /// not match `.' as the first character of string
        MATCH_PERIOD = 1<<2,

        /// match leading dir part
        MATCH_LEADING_DIR = 1<<3,

        /// ignore case
        /// *.c match filename.C
        IGNORE_CASE = 1<<4,
    };
public:
    /// @brief wildcard match
    /// @param pattetn wildcard pattern to be matched
    /// @param string string to match
    /// @param flags MATCH_FLAGS combination
    /// @return whether match success
    static bool Match(const char* pattern, const char* string, int flags = 0);

    static bool Match(
        const std::string& pattern,
        const std::string& string,
        int flags = 0
    )
    {
        return Match(pattern.c_str(), string.c_str(), flags);
    }

    static bool Match(
        const char* pattern,
        const std::string& string,
        int flags = 0
    )
    {
        return Match(pattern, string.c_str(), flags);
    }

    static bool Match(
        const std::string& pattern,
        const char* string,
        int flags = 0
    )
    {
        return Match(pattern.c_str(), string, flags);
    }
private:
    static const char* rangematch(const char *pattern, char test, int flags);
    static int fnmatch(const char *pattern, const char *string, int flags);
};

#endif // IPCS_COMMON_TEXT_WILDCARD_H

