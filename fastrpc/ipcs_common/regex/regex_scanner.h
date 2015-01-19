// Copyright (c) 2005, Google Inc.
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
//
// Regular-expression based scanner for parsing an input stream.
//
// Example 1: parse a sequence of "var = number" entries from input:
//
//      RegexScaner scanner(input);
//      string var;
//      int number;
//      scanner.SetSkipExpression("\\s+"); // Skip any white space we encounter
//      while (scanner.Consume("(\\w+) = (\\d+)", &var, &number)) {
//        ...;
//      }

#ifndef REGEX_SCANNER_H
#define REGEX_SCANNER_H

#include <assert.h>
#include <string>
#include <vector>

#include "regex.h"
#include "string_piece.h"

// namespace common {

class REGEX_EXPORT_DEFINE RegexScaner {
public:
    RegexScaner();
    explicit RegexScaner(const std::string& input);
    ~RegexScaner();

    // Return current line number.  The returned line-number is
    // one-based.  I.e. it returns 1 + the number of consumed newlines.
    //
    // Note: this method may be slow.  It may take time proportional to
    // the size of the input.
    int LineNumber() const;

    // Return the byte-offset that the scanner is looking in the
    // input data;
    int Offset() const;

    // Return true iff the start of the remaining input matches "re"
    bool LookingAt(const Regex& re) const;

    // Return true iff all of the following are true
    //    a. the start of the remaining input matches "re",
    //    b. if any arguments are supplied, matched sub-patterns can be
    //       parsed and stored into the arguments.
    // If it returns true, it skips over the matched input and any
    // following input that matches the "skip" regular expression.
    bool Consume(const Regex& re,
                 const ValueParser& capture_ptr0 = Regex::no_more_args,
                 const ValueParser& capture_ptr1 = Regex::no_more_args,
                 const ValueParser& capture_ptr2 = Regex::no_more_args,
                 const ValueParser& capture_ptr3 = Regex::no_more_args,
                 const ValueParser& capture_ptr4 = Regex::no_more_args,
                 const ValueParser& capture_ptr5 = Regex::no_more_args,
                 const ValueParser& capture_ptr6 = Regex::no_more_args,
                 const ValueParser& capture_ptr7 = Regex::no_more_args,
                 const ValueParser& capture_ptr8 = Regex::no_more_args,
                 const ValueParser& capture_ptr9 = Regex::no_more_args,
                 const ValueParser& capture_ptr10 = Regex::no_more_args,
                 const ValueParser& capture_ptr11 = Regex::no_more_args,
                 const ValueParser& capture_ptr12 = Regex::no_more_args,
                 const ValueParser& capture_ptr13 = Regex::no_more_args,
                 const ValueParser& capture_ptr14 = Regex::no_more_args,
                 const ValueParser& capture_ptr15 = Regex::no_more_args
    );

    // Set the "skip" regular expression.  If after consuming some data,
    // a prefix of the input matches this Regex, it is automatically
    // skipped.  For example, a programming language scanner would use
    // a skip Regex that matches white space and comments.
    //
    //    scanner.SetSkipExpression("\\s+|//.*|/[*](.|\n)*?[*]/");
    //
    // Skipping repeats as long as it succeeds.  We used to let people do
    // this by writing "(...)*" in the regular expression, but that added
    // up to lots of recursive calls within the pcre library, so now we
    // control repetition explicitly via the function call API.
    //
    // You can pass NULL for "re" if you do not want any data to be skipped.
    void Skip(const char* re);   // DEPRECATED; does *not* repeat
    void SetSkipExpression(const char* re);

    // Temporarily pause "skip"ing. This
    //   Skip("Foo"); code ; DisableSkip(); code; EnableSkip()
    // is similar to
    //   Skip("Foo"); code ; Skip(NULL); code ; Skip("Foo");
    // but avoids creating/deleting new Regex objects.
    void DisableSkip();

    // Reenable previously paused skipping.  Any prefix of the input
    // that matches the skip pattern is immediately dropped.
    void EnableSkip();

    /***** Special wrappers around SetSkip() for some common idioms *****/

    // Arranges to skip whitespace, C comments, C++ comments.
    // The overall Regex is a disjunction of the following REs:
    //    \\s                     whitespace
    //    //.*\n                  C++ comment
    //    /[*](.|\n)*?[*]/        C comment (x*? means minimal repetitions of x)
    // We get repetition via the semantics of SetSkipExpression, not by using *
    void SkipCxxComments() {
        SetSkipExpression("\\s|//.*\n|/[*](?:\n|.)*?[*]/");
    }

    void SetSaveComments(bool save_comments = true) {
        m_save_comments = save_comments;
    }

    bool IsSaveComments() {
        return m_save_comments;
    }

    // Append to vector ranges the comments found in the
    // byte range [start,end] (inclusive) of the input data.
    // Only comments that were extracted entirely within that
    // range are returned: no range splitting of atomically-extracted
    // comments is performed.
    void GetComments(int start, int end, std::vector<StringPiece> *ranges);

    // Append to vector ranges the comments added
    // since the last time this was called. This
    // functionality is provided for efficiency when
    // interleaving scanning with parsing.
    void GetNextComments(std::vector<StringPiece> *ranges);

private:
    std::string   m_data;          // All the input data
    StringPiece   m_input;         // Unprocessed input
    Regex*        m_skip;          // If non-NULL, Regex for skipping input
    bool          m_should_skip;   // If true, use m_skip
    bool          m_skip_repeat;   // If true, repeat m_skip as long as it works
    bool          m_save_comments; // If true, aggregate the skip expression

    // the skipped comments
    // TODO: later consider requiring that the StringPieces be added
    // in order by their start position
    std::vector<StringPiece> *m_comments;

    // the offset into m_comments that has been returned by GetNextComments
    int           m_comments_offset;

    // helper function to consume *m_skip and honour
    // m_save_comments
    void ConsumeSkip();
};

// } // namespace common

#endif // REGEX_SCANNER_H
