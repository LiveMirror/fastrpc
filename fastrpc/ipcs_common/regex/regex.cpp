// Copyright (c) 2010, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Sanjay Ghemawat

#include "regex.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>      /* for SHRT_MIN, USHRT_MAX, etc */
#include <assert.h>
#include <errno.h>
#include <string>
#include <algorithm>

#include "regex_internal.h"
#include "string_piece.h"

#include "config.h"
#include "pcre.h"

// namespace common {

// Special object that stands-in for no argument
ValueParser Regex::no_more_args((void*)NULL);

#if 0
// This is for ABI compatibility with old versions of pcre (pre-7.6),
// which defined a global no_more_args variable instead of putting it in the
// Regex class.  This works on GCC >= 3, at least.  It definitely works
// for ELF, but may not for other object formats (Mach-O, for
// instance, does not support aliases.)  We could probably have a more
// inclusive test if we ever needed it.  (Note that not only the
// __attribute__ syntax, but also __USER_LABEL_PREFIX__, are
// gnu-specific.)
#if defined(__GNUC__) && __GNUC__ >= 3 && defined(__ELF__)
# define ULP_AS_STRING(x)            ULP_AS_STRING_INTERNAL(x)
# define ULP_AS_STRING_INTERNAL(x)   #x
# define USER_LABEL_PREFIX_STR       ULP_AS_STRING(__USER_LABEL_PREFIX__)
extern ValueParser no_more_args
    __attribute__((alias(USER_LABEL_PREFIX_STR "_ZN7regex2RE6no_more_argsE")));
#endif
#endif

// If a regular expression has no error, its m_error field points here
static const std::string empty_string;

// If the user doesn't ask for any options, we just use this one
static Regex::Options default_options;

void Regex::Init(const std::string& pat, const Options* options) {
    m_pattern = pat;
    if (options == NULL) {
        m_options = default_options;
    } else {
        m_options = *options;
    }
    m_error = &empty_string;
    m_re_full = NULL;
    m_re_partial = NULL;

    m_re_partial = Compile(UNANCHORED);
    if (m_re_partial != NULL) {
        m_re_full = Compile(ANCHOR_BOTH);
    }
}

void Regex::Cleanup() {
    if (m_re_full != NULL) {
        (*pcre_free)(m_re_full);
        m_re_full = NULL;
    }

    if (m_re_partial != NULL) {
        (*pcre_free)(m_re_partial);
        m_re_partial = NULL;
    }

    if (m_error != &empty_string) {
        delete m_error;
        m_error = NULL;
    }
}


Regex::~Regex() {
    Cleanup();
}

// Copy constructor & assignment - note that these are expensive
// because they recompile the expression.
Regex::Regex(const Regex& regex) {
    Init(regex.m_pattern, &regex.m_options);
}

const Regex& Regex::operator=(const Regex& re) {
    if (this != &re) {
        Cleanup();

        // This is the code that originally came from Google
        // Init(re.m_pattern.c_str(), &re.m_options);

        // This is the replacement from Ari Pollak
        Init(re.m_pattern, &re.m_options);
    }
    return *this;
}


pcre* Regex::Compile(Anchor anchor) {
    // First, convert Options into pcre options
    int pcre_options = 0;
    pcre_options = m_options.AllOptions();

    // Special treatment for anchoring.  This is needed because at
    // runtime pcre only provides an option for anchoring at the
    // beginning of a string (unless you use offset).
    //
    // There are three types of anchoring we want:
    //    UNANCHORED      Compile the original pattern, and use
    //                    a pcre unanchored match.
    //    ANCHOR_START    Compile the original pattern, and use
    //                    a pcre anchored match.
    //    ANCHOR_BOTH     Tack a "\z" to the end of the original pattern
    //                    and use a pcre anchored match.

    const char* compile_error;
    int eoffset;
    pcre* re;
    if (anchor != ANCHOR_BOTH) {
        re = pcre_compile(m_pattern.c_str(), pcre_options,
                          &compile_error, &eoffset, NULL);
    } else {
        // Tack a '\z' at the end of Regex.  Parenthesize it first so that
        // the '\z' applies to all top-level alternatives in the regexp.
        std::string wrapped = "(?:";  // A non-counting grouping operator
        wrapped += m_pattern;
        wrapped += ")\\z";
        re = pcre_compile(wrapped.c_str(), pcre_options,
                          &compile_error, &eoffset, NULL);
    }
    if (re == NULL) {
        if (m_error == &empty_string) m_error = new std::string(compile_error);
    }
    return re;
}

/***** Matching interfaces *****/

bool Regex::FullMatch(const StringPiece& text,
                      const ValueParser& capture_ptr1,
                      const ValueParser& capture_ptr2,
                      const ValueParser& capture_ptr3,
                      const ValueParser& capture_ptr4,
                      const ValueParser& capture_ptr5,
                      const ValueParser& capture_ptr6,
                      const ValueParser& capture_ptr7,
                      const ValueParser& capture_ptr8,
                      const ValueParser& capture_ptr9,
                      const ValueParser& capture_ptr10,
                      const ValueParser& capture_ptr11,
                      const ValueParser& capture_ptr12,
                      const ValueParser& capture_ptr13,
                      const ValueParser& capture_ptr14,
                      const ValueParser& capture_ptr15,
                      const ValueParser& capture_ptr16) const {
    const ValueParser* args[kMaxArgs];
    int nargs = 0;
    if (&capture_ptr1  == &no_more_args) goto done; args[nargs++] = &capture_ptr1;
    if (&capture_ptr2  == &no_more_args) goto done; args[nargs++] = &capture_ptr2;
    if (&capture_ptr3  == &no_more_args) goto done; args[nargs++] = &capture_ptr3;
    if (&capture_ptr4  == &no_more_args) goto done; args[nargs++] = &capture_ptr4;
    if (&capture_ptr5  == &no_more_args) goto done; args[nargs++] = &capture_ptr5;
    if (&capture_ptr6  == &no_more_args) goto done; args[nargs++] = &capture_ptr6;
    if (&capture_ptr7  == &no_more_args) goto done; args[nargs++] = &capture_ptr7;
    if (&capture_ptr8  == &no_more_args) goto done; args[nargs++] = &capture_ptr8;
    if (&capture_ptr9  == &no_more_args) goto done; args[nargs++] = &capture_ptr9;
    if (&capture_ptr10 == &no_more_args) goto done; args[nargs++] = &capture_ptr10;
    if (&capture_ptr11 == &no_more_args) goto done; args[nargs++] = &capture_ptr11;
    if (&capture_ptr12 == &no_more_args) goto done; args[nargs++] = &capture_ptr12;
    if (&capture_ptr13 == &no_more_args) goto done; args[nargs++] = &capture_ptr13;
    if (&capture_ptr14 == &no_more_args) goto done; args[nargs++] = &capture_ptr14;
    if (&capture_ptr15 == &no_more_args) goto done; args[nargs++] = &capture_ptr15;
    if (&capture_ptr16 == &no_more_args) goto done; args[nargs++] = &capture_ptr16;
done:

    int consumed;
    int vec[kVectorSize];
    return DoMatchImpl(text, ANCHOR_BOTH, &consumed, args, nargs, vec, kVectorSize);
}

bool Regex::PartialMatch(const StringPiece& text,
                         const ValueParser& capture_ptr1,
                         const ValueParser& capture_ptr2,
                         const ValueParser& capture_ptr3,
                         const ValueParser& capture_ptr4,
                         const ValueParser& capture_ptr5,
                         const ValueParser& capture_ptr6,
                         const ValueParser& capture_ptr7,
                         const ValueParser& capture_ptr8,
                         const ValueParser& capture_ptr9,
                         const ValueParser& capture_ptr10,
                         const ValueParser& capture_ptr11,
                         const ValueParser& capture_ptr12,
                         const ValueParser& capture_ptr13,
                         const ValueParser& capture_ptr14,
                         const ValueParser& capture_ptr15,
                         const ValueParser& capture_ptr16) const {
    const ValueParser* args[kMaxArgs];
    int nargs = 0;
    if (&capture_ptr1  == &no_more_args) goto done; args[nargs++] = &capture_ptr1;
    if (&capture_ptr2  == &no_more_args) goto done; args[nargs++] = &capture_ptr2;
    if (&capture_ptr3  == &no_more_args) goto done; args[nargs++] = &capture_ptr3;
    if (&capture_ptr4  == &no_more_args) goto done; args[nargs++] = &capture_ptr4;
    if (&capture_ptr5  == &no_more_args) goto done; args[nargs++] = &capture_ptr5;
    if (&capture_ptr6  == &no_more_args) goto done; args[nargs++] = &capture_ptr6;
    if (&capture_ptr7  == &no_more_args) goto done; args[nargs++] = &capture_ptr7;
    if (&capture_ptr8  == &no_more_args) goto done; args[nargs++] = &capture_ptr8;
    if (&capture_ptr9  == &no_more_args) goto done; args[nargs++] = &capture_ptr9;
    if (&capture_ptr10 == &no_more_args) goto done; args[nargs++] = &capture_ptr10;
    if (&capture_ptr11 == &no_more_args) goto done; args[nargs++] = &capture_ptr11;
    if (&capture_ptr12 == &no_more_args) goto done; args[nargs++] = &capture_ptr12;
    if (&capture_ptr13 == &no_more_args) goto done; args[nargs++] = &capture_ptr13;
    if (&capture_ptr14 == &no_more_args) goto done; args[nargs++] = &capture_ptr14;
    if (&capture_ptr15 == &no_more_args) goto done; args[nargs++] = &capture_ptr15;
    if (&capture_ptr16 == &no_more_args) goto done; args[nargs++] = &capture_ptr16;
done:

    int consumed;
    int vec[kVectorSize];
    return DoMatchImpl(text, UNANCHORED, &consumed, args, nargs, vec, kVectorSize);
}

bool Regex::Consume(StringPiece* input,
                    const ValueParser& capture_ptr1,
                    const ValueParser& capture_ptr2,
                    const ValueParser& capture_ptr3,
                    const ValueParser& capture_ptr4,
                    const ValueParser& capture_ptr5,
                    const ValueParser& capture_ptr6,
                    const ValueParser& capture_ptr7,
                    const ValueParser& capture_ptr8,
                    const ValueParser& capture_ptr9,
                    const ValueParser& capture_ptr10,
                    const ValueParser& capture_ptr11,
                    const ValueParser& capture_ptr12,
                    const ValueParser& capture_ptr13,
                    const ValueParser& capture_ptr14,
                    const ValueParser& capture_ptr15,
                    const ValueParser& capture_ptr16) const {
    const ValueParser* args[kMaxArgs];
    int nargs = 0;
    if (&capture_ptr1  == &no_more_args) goto done; args[nargs++] = &capture_ptr1;
    if (&capture_ptr2  == &no_more_args) goto done; args[nargs++] = &capture_ptr2;
    if (&capture_ptr3  == &no_more_args) goto done; args[nargs++] = &capture_ptr3;
    if (&capture_ptr4  == &no_more_args) goto done; args[nargs++] = &capture_ptr4;
    if (&capture_ptr5  == &no_more_args) goto done; args[nargs++] = &capture_ptr5;
    if (&capture_ptr6  == &no_more_args) goto done; args[nargs++] = &capture_ptr6;
    if (&capture_ptr7  == &no_more_args) goto done; args[nargs++] = &capture_ptr7;
    if (&capture_ptr8  == &no_more_args) goto done; args[nargs++] = &capture_ptr8;
    if (&capture_ptr9  == &no_more_args) goto done; args[nargs++] = &capture_ptr9;
    if (&capture_ptr10 == &no_more_args) goto done; args[nargs++] = &capture_ptr10;
    if (&capture_ptr11 == &no_more_args) goto done; args[nargs++] = &capture_ptr11;
    if (&capture_ptr12 == &no_more_args) goto done; args[nargs++] = &capture_ptr12;
    if (&capture_ptr13 == &no_more_args) goto done; args[nargs++] = &capture_ptr13;
    if (&capture_ptr14 == &no_more_args) goto done; args[nargs++] = &capture_ptr14;
    if (&capture_ptr15 == &no_more_args) goto done; args[nargs++] = &capture_ptr15;
    if (&capture_ptr16 == &no_more_args) goto done; args[nargs++] = &capture_ptr16;
done:

    int consumed;
    int vec[kVectorSize];
    if (DoMatchImpl(*input, ANCHOR_START, &consumed,
                    args, nargs, vec, kVectorSize)) {
        input->remove_prefix(consumed);
        return true;
    } else {
        return false;
    }
}

bool Regex::FindAndConsume(StringPiece* input,
                           const ValueParser& capture_ptr1,
                           const ValueParser& capture_ptr2,
                           const ValueParser& capture_ptr3,
                           const ValueParser& capture_ptr4,
                           const ValueParser& capture_ptr5,
                           const ValueParser& capture_ptr6,
                           const ValueParser& capture_ptr7,
                           const ValueParser& capture_ptr8,
                           const ValueParser& capture_ptr9,
                           const ValueParser& capture_ptr10,
                           const ValueParser& capture_ptr11,
                           const ValueParser& capture_ptr12,
                           const ValueParser& capture_ptr13,
                           const ValueParser& capture_ptr14,
                           const ValueParser& capture_ptr15,
                           const ValueParser& capture_ptr16) const {
    const ValueParser* args[kMaxArgs];
    int nargs = 0;
    if (&capture_ptr1  == &no_more_args) goto done; args[nargs++] = &capture_ptr1;
    if (&capture_ptr2  == &no_more_args) goto done; args[nargs++] = &capture_ptr2;
    if (&capture_ptr3  == &no_more_args) goto done; args[nargs++] = &capture_ptr3;
    if (&capture_ptr4  == &no_more_args) goto done; args[nargs++] = &capture_ptr4;
    if (&capture_ptr5  == &no_more_args) goto done; args[nargs++] = &capture_ptr5;
    if (&capture_ptr6  == &no_more_args) goto done; args[nargs++] = &capture_ptr6;
    if (&capture_ptr7  == &no_more_args) goto done; args[nargs++] = &capture_ptr7;
    if (&capture_ptr8  == &no_more_args) goto done; args[nargs++] = &capture_ptr8;
    if (&capture_ptr9  == &no_more_args) goto done; args[nargs++] = &capture_ptr9;
    if (&capture_ptr10 == &no_more_args) goto done; args[nargs++] = &capture_ptr10;
    if (&capture_ptr11 == &no_more_args) goto done; args[nargs++] = &capture_ptr11;
    if (&capture_ptr12 == &no_more_args) goto done; args[nargs++] = &capture_ptr12;
    if (&capture_ptr13 == &no_more_args) goto done; args[nargs++] = &capture_ptr13;
    if (&capture_ptr14 == &no_more_args) goto done; args[nargs++] = &capture_ptr14;
    if (&capture_ptr15 == &no_more_args) goto done; args[nargs++] = &capture_ptr15;
    if (&capture_ptr16 == &no_more_args) goto done; args[nargs++] = &capture_ptr16;
done:

    int consumed;
    int vec[kVectorSize];
    if (DoMatchImpl(*input, UNANCHORED, &consumed,
                    args, nargs, vec, kVectorSize)) {
        input->remove_prefix(consumed);
        return true;
    } else {
        return false;
    }
}

bool Regex::Replace(const StringPiece& rewrite,
                    std::string *str) const {
    int vec[kVectorSize];
    int matches = TryMatch(*str, 0, UNANCHORED, true, vec, kVectorSize);
    if (matches == 0)
        return false;

    std::string s;
    if (!Rewrite(&s, rewrite, *str, vec, matches))
        return false;

    assert(vec[0] >= 0);
    assert(vec[1] >= 0);
    str->replace(vec[0], vec[1] - vec[0], s);
    return true;
}

// Returns PCRE_NEWLINE_CRLF, PCRE_NEWLINE_CR, or PCRE_NEWLINE_LF.
// Note that PCRE_NEWLINE_CRLF is defined to be P_N_CR | P_N_LF.
// Modified by PH to add PCRE_NEWLINE_ANY and PCRE_NEWLINE_ANYCRLF.

static int NewlineMode(int pcre_options) {
    // TODO: if we can make it threadsafe, cache this var
    int newline_mode = 0;
    /* if (newline_mode) return newline_mode; */  // do this once it's cached
    if (pcre_options & (PCRE_NEWLINE_CRLF|PCRE_NEWLINE_CR|PCRE_NEWLINE_LF|
                        PCRE_NEWLINE_ANY|PCRE_NEWLINE_ANYCRLF)) {
        newline_mode = (pcre_options &
                        (PCRE_NEWLINE_CRLF|PCRE_NEWLINE_CR|PCRE_NEWLINE_LF|
                         PCRE_NEWLINE_ANY|PCRE_NEWLINE_ANYCRLF));
    } else {
        int newline;
        pcre_config(PCRE_CONFIG_NEWLINE, &newline);
        if (newline == 10)
            newline_mode = PCRE_NEWLINE_LF;
        else if (newline == 13)
            newline_mode = PCRE_NEWLINE_CR;
        else if (newline == 3338)
            newline_mode = PCRE_NEWLINE_CRLF;
        else if (newline == -1)
            newline_mode = PCRE_NEWLINE_ANY;
        else if (newline == -2)
            newline_mode = PCRE_NEWLINE_ANYCRLF;
        else
            assert(NULL == "Unexpected return value from pcre_config(NEWLINE)");
    }
    return newline_mode;
}

int Regex::GlobalReplace(const StringPiece& rewrite,
                         std::string *str) const {
    int count = 0;
    int vec[kVectorSize];
    std::string out;
    int start = 0;
    int lastend = -1;
    bool last_match_was_empty_string = false;

    while (start <= static_cast<int>(str->length())) {
        // If the previous match was for the empty string, we shouldn't
        // just match again: we'll match in the same way and get an
        // infinite loop.  Instead, we do the match in a special way:
        // anchored -- to force another try at the same position --
        // and with a flag saying that this time, ignore empty matches.
        // If this special match returns, that means there's a non-empty
        // match at this position as well, and we can continue.  If not,
        // we do what perl does, and just advance by one.
        // Notice that perl prints '@@@' for this;
        //    perl -le '$_ = "aa"; s/b*|aa/@/g; print'
        int matches;
        if (last_match_was_empty_string) {
            matches = TryMatch(*str, start, ANCHOR_START, false, vec, kVectorSize);
            if (matches <= 0) {
                int matchend = start + 1;     // advance one character.
                // If the current char is CR and we'Regex in CRLF mode, skip LF too.
                // Note it's better to call pcre_fullinfo() than to examine
                // AllOptions(), since m_options could have changed bewteen
                // compile-time and now, but this is simpler and safe enough.
                // Modified by PH to add ANY and ANYCRLF.
                if (matchend < static_cast<int>(str->length()) &&
                    (*str)[start] == '\r' && (*str)[matchend] == '\n' &&
                    (NewlineMode(m_options.AllOptions()) == PCRE_NEWLINE_CRLF ||
                     NewlineMode(m_options.AllOptions()) == PCRE_NEWLINE_ANY ||
                     NewlineMode(m_options.AllOptions()) == PCRE_NEWLINE_ANYCRLF)) {
                    matchend++;
                }
                // We also need to advance more than one char if we'Regex in utf8 mode.
#ifdef SUPPORT_UTF8
                if (m_options.Utf8()) {
                    while (matchend < static_cast<int>(str->length()) &&
                           ((*str)[matchend] & 0xc0) == 0x80)
                        matchend++;
                }
#endif
                if (start < static_cast<int>(str->length()))
                    out.append(*str, start, matchend - start);
                start = matchend;
                last_match_was_empty_string = false;
                continue;
            }
        } else {
            matches = TryMatch(*str, start, UNANCHORED, true, vec, kVectorSize);
            if (matches <= 0)
                break;
        }
        int matchstart = vec[0], matchend = vec[1];
        assert(matchstart >= start);
        assert(matchend >= matchstart);
        out.append(*str, start, matchstart - start);
        Rewrite(&out, rewrite, *str, vec, matches);
        start = matchend;
        lastend = matchend;
        count++;
        last_match_was_empty_string = (matchstart == matchend);
    }

    if (count == 0)
        return 0;

    if (start < static_cast<int>(str->length()))
        out.append(*str, start, str->length() - start);
    swap(out, *str);
    return count;
}

bool Regex::Extract(const StringPiece& rewrite,
                    const StringPiece& text,
                    std::string *out) const {
    int vec[kVectorSize];
    int matches = TryMatch(text, 0, UNANCHORED, true, vec, kVectorSize);
    if (matches == 0)
        return false;
    out->erase();
    return Rewrite(out, rewrite, text, vec, matches);
}

/*static*/
std::string Regex::QuoteMeta(const StringPiece& unquoted) {
    std::string result;

    // Escape any ascii character not in [A-Za-z_0-9].
    //
    // Note that it's legal to escape a character even if it has no
    // special meaning in a regular expression -- so this function does
    // that.  (This also makes it identical to the perl function of the
    // same name; see `perldoc -f quotemeta`.)  The one exception is
    // escaping NUL: rather than doing backslash + NUL, like perl does,
    // we do '\0', because pcre itself doesn't take embedded NUL chars.
    for (size_t ii = 0; ii < unquoted.size(); ++ii) {
        // Note that using 'isalnum' here raises the benchmark time from
        // 32ns to 58ns:
        if (unquoted[ii] == '\0') {
            result += "\\0";
        } else if ((unquoted[ii] < 'a' || unquoted[ii] > 'z') &&
                   (unquoted[ii] < 'A' || unquoted[ii] > 'Z') &&
                   (unquoted[ii] < '0' || unquoted[ii] > '9') &&
                   unquoted[ii] != '_' &&
                   // If this is the part of a UTF8 or Latin1 character, we need
                   // to copy this byte without escaping.  Experimentally this is
                   // what works correctly with the regexp library.
                   !(unquoted[ii] & 128)) {
            result += '\\';
            result += unquoted[ii];
        } else {
            result += unquoted[ii];
        }
    }

    return result;
}

/***** Actual matching and rewriting code *****/

int Regex::TryMatch(const StringPiece& text,
                    int startpos,
                    Anchor anchor,
                    bool empty_ok,
                    int *vec,
                    int vecsize) const {
    pcre* re = (anchor == ANCHOR_BOTH) ? m_re_full : m_re_partial;
    if (re == NULL) {
        //fprintf(stderr, "Matching against invalid Regex: %s\n", m_error->c_str());
        return 0;
    }

    pcre_extra extra = { 0, 0, 0, 0, 0, 0 };
    if (m_options.MatchLimit() > 0) {
        extra.flags |= PCRE_EXTRA_MATCH_LIMIT;
        extra.match_limit = m_options.MatchLimit();
    }
    if (m_options.MatchLimitRecursion() > 0) {
        extra.flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
        extra.match_limit_recursion = m_options.MatchLimitRecursion();
    }

    int options = 0;
    if (anchor != UNANCHORED)
        options |= PCRE_ANCHORED;
    if (!empty_ok)
        options |= PCRE_NOTEMPTY;

    int rc = pcre_exec(re,              // The regular expression object
                       &extra,
                       (text.data() == NULL) ? "" : text.data(),
                       text.size(),
                       startpos,
                       options,
                       vec,
                       vecsize);

    // Handle errors
    if (rc == PCRE_ERROR_NOMATCH) {
        return 0;
    } else if (rc < 0) {
        //fprintf(stderr, "Unexpected return code: %d when matching '%s'\n",
        //        re, m_pattern.c_str());
        return 0;
    } else if (rc == 0) {
        // pcre_exec() returns 0 as a special case when the number of
        // capturing subpatterns exceeds the size of the vector.
        // When this happens, there is a match and the output vector
        // is filled, but we miss out on the positions of the extra subpatterns.
        rc = vecsize / 2;
    }

    return rc;
}

bool Regex::DoMatchImpl(const StringPiece& text,
                        Anchor anchor,
                        int* consumed,
                        const ValueParser* const* args,
                        int nargs,
                        int* vec,
                        int vecsize) const {
    assert((1 + nargs) * 3 <= vecsize);  // results + PCRE workspace
    int matches = TryMatch(text, 0, anchor, true, vec, vecsize);
    assert(matches >= 0);  // TryMatch never returns negatives
    if (matches == 0)
        return false;

    *consumed = vec[1];

    if (nargs == 0 || args == NULL) {
        // We are not interested in results
        return true;
    }

    if (NumberOfCapturingGroups() < nargs) {
        // Regex has fewer capturing groups than number of arg pointers passed in
        return false;
    }

    // If we got here, we must have matched the whole pattern.
    // We do not need (can not do) any more checks on the value of 'matches' here
    // -- see the comment for TryMatch.
    for (int i = 0; i < nargs; i++) {
        const int start = vec[2*(i+1)];
        const int limit = vec[2*(i+1)+1];
        if (!args[i]->Parse(text.data() + start, limit-start)) {
            // TODO: Should we indicate what the error was?
            return false;
        }
    }

    return true;
}

bool Regex::DoMatch(const StringPiece& text,
                    Anchor anchor,
                    int* consumed,
                    const ValueParser* const args[],
                    int nargs) const {
    assert(nargs >= 0);
    size_t const vecsize = (1 + nargs) * 3;  // results + PCRE workspace
    // (as for kVectorSize)
    int space[21];   // use stack allocation for small vecsize (common case)
    int* vec = vecsize <= 21 ? space : new int[vecsize];
    bool retval = DoMatchImpl(text, anchor, consumed, args, nargs, vec, (int)vecsize);
    if (vec != space) delete [] vec;
    return retval;
}

bool Regex::Rewrite(std::string *out, const StringPiece &rewrite,
                    const StringPiece &text, int *vec, int veclen) const {
    for (const char *s = rewrite.data(), *end = s + rewrite.size();
         s < end; s++) {
        int c = *s;
        if (c == '\\') {
            c = *++s;
            if (isdigit(c)) {
                int n = (c - '0');
                if (n >= veclen) {
                    //fprintf(stderr, requested group %d in regexp %.*s\n",
                    //        n, rewrite.size(), rewrite.data());
                    return false;
                }
                int start = vec[2 * n];
                if (start >= 0)
                    out->append(text.data() + start, vec[2 * n + 1] - start);
            } else if (c == '\\') {
                *out += '\\';
            } else {
                //fprintf(stderr, "invalid rewrite pattern: %.*s\n",
                //        rewrite.size(), rewrite.data());
                return false;
            }
        } else {
            *out += c;
        }
    }
    return true;
}

// Return the number of capturing subpatterns, or -1 if the
// regexp wasn't valid on construction.
int Regex::NumberOfCapturingGroups() const {
    if (m_re_partial == NULL) return -1;

    int result;
    int pcre_retval = pcre_fullinfo(m_re_partial,  // The regular expression object
                                    NULL,         // We did not study the pattern
                                    PCRE_INFO_CAPTURECOUNT,
                                    &result);
    assert(pcre_retval == 0);
    (void) pcre_retval;
    return result;
}

// } // namespace common
