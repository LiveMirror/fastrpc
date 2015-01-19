// Copyright (c) 2013, Jack.
// All rights reserved.
//
// Author: Jack <lapsangx@gmail.com>
// Created: 05/08/13
// Description:

#include "wildcard.h"

#if __unix__
// fnmatch was defined by ISO/IEC 9945-2: 1993 (POSIX.2)
#include <fnmatch.h>
#else
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#endif


#if __unix__

bool Wildcard::Match(const char* pattern, const char* string, int flags)
{
    return ::fnmatch(pattern, string, flags) == 0;
}

#else

// match range, eg. [A-Z]
// function name keep original lower case
const char* Wildcard::rangematch(const char *pattern, char test, int flags)
{
    int negate, ok;
    char c, c2;

    /*
     * A bracket expression starting with an unquoted circumflex
     * character produces unspecified results (IEEE 1003.2-1992,
     * 3.13.2).  This implementation treats it like '!', for
     * consistency with the regular expression syntax.
     * J.T. Conklin <conklin@ngai.kaleida.com>
     */
    if ( (negate = (*pattern == '!' || *pattern == '^')) )
        ++pattern;

    if (flags & IGNORE_CASE)
        test = tolower((unsigned char)test);

    for (ok = 0; (c = *pattern++) != ']';) {
        if (c == '\\' && !(flags & NO_ESCAPE))
            c = *pattern++;
        if (c == '\0')
            return NULL;

        if (flags & IGNORE_CASE)
            c = tolower((unsigned char)c);

        if (*pattern == '-'
            && (c2 = *(pattern+1)) != '\0' && c2 != ']') {
            pattern += 2;
            if (c2 == '\\' && !(flags & NO_ESCAPE))
                c2 = *pattern++;
            if (c2 == '\0')
                return NULL;

            if (flags & IGNORE_CASE)
                c2 = tolower((unsigned char)c2);

            if ((unsigned char)c <= (unsigned char)test &&
                (unsigned char)test <= (unsigned char)c2)
                ok = 1;
        } else if (c == test) {
            ok = 1;
        }
    }
    return ok == negate ? NULL : pattern;
}

// substitute fnmatch function implementation
// function name keep original lower case
int Wildcard::fnmatch(const char *pattern, const char *string, int flags)
{
    const char *stringstart;
    char c, test;

    for (stringstart = string;;)
    {
        switch (c = *pattern++) {
        case '\0':
            if ((flags & MATCH_LEADING_DIR) && *string == '/')
                return 0;
            return *string == '\0' ? 0 : -1;
        case '?':
            if (*string == '\0')
                return -1;
            if (*string == '/' && (flags & MATCH_FILE_NAME_ONLY))
                return -1;
            if (*string == '.' && (flags & MATCH_PERIOD) &&
                (string == stringstart ||
                 ((flags & MATCH_FILE_NAME_ONLY) && *(string - 1) == '/')))
                return -1;
            ++string;
            break;
        case '*':
            c = *pattern;
            /* Collapse multiple stars. */
            while (c == '*')
                c = *++pattern;

            if (*string == '.' && (flags & MATCH_PERIOD) &&
                (string == stringstart ||
                 ((flags & MATCH_FILE_NAME_ONLY) && *(string - 1) == '/'))) {
                return -1;
            }

            /* Optimize for pattern with * at end or before /. */
            if (c == '\0')
                if (flags & MATCH_FILE_NAME_ONLY)
                    return ((flags & MATCH_LEADING_DIR) ||
                            strchr(string, '/') == NULL ?
                            0 : -1);
                else
                    return 0;
            else if (c == '/' && flags & MATCH_FILE_NAME_ONLY) {
                if ((string = strchr(string, '/')) == NULL)
                    return -1;
                break;
            }

            /* General case, use recursion. */
            while ((test = *string) != '\0') {
                if (!fnmatch(pattern, string, flags & ~MATCH_PERIOD))
                    return (0);
                if (test == '/' && flags & MATCH_FILE_NAME_ONLY)
                    break;
                ++string;
            }
            return -1;
        case '[':
            if (*string == '\0')
                return -1;
            if (*string == '/' && flags & MATCH_FILE_NAME_ONLY)
                return -1;
            if ((pattern =
                 rangematch(pattern, *string, flags)) == NULL)
                return -1;
            ++string;
            break;
        case '\\':
            if (!(flags & NO_ESCAPE)) {
                if ((c = *pattern++) == '\0') {
                    c = '\\';
                    --pattern;
                }
            }
            /* FALLTHROUGH */
        default:
            if (c == *string) {
            }
            else if ((flags & IGNORE_CASE) &&
                     (tolower((unsigned char)c) ==
                      tolower((unsigned char)*string))) {
            }
            else if ((flags & MATCH_LEADING_DIR) && *string == '\0' &&
                     ((c == '/' && string != stringstart) ||
                      (string == stringstart+1 && *stringstart == '/'))) {
                return 0;
            } else {
                return -1;
            }
            string++;
            break;
        }
    }
    return -1;
}

bool Wildcard::Match(const char* pattern, const char* string, int flags)
{
    return Wildcard::fnmatch(pattern, string, flags) == 0;
}

#endif

