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
// Support for PCRE_XXX modifiers added by Giuseppe Maxia, July 2005

#ifndef REGEX_H
#define REGEX_H

// C++ interface to the pcre regular-expression library.  Regex supports
// Perl-style regular expressions (with extensions like \d, \w, \s,
// ...).
//
// -----------------------------------------------------------------------
// REGEXP SYNTAX:
//
// This module is part of the pcre library and hence supports its syntax
// for regular expressions.
//
// The syntax is pretty similar to Perl's.  For those not familiar
// with Perl's regular expressions, here are some examples of the most
// commonly used extensions:
//
//   "hello (\\w+) world"  -- \w matches a "word" character
//   "version (\\d+)"      -- \d matches a digit
//   "hello\\s+world"      -- \s matches any whitespace character
//   "\\b(\\w+)\\b"        -- \b matches empty string at a word boundary
//   "(?i)hello"           -- (?i) turns on case-insensitive matching
//   "/\\*(.*?)\\*/"       -- .*? matches . minimum no. of times possible
//
// -----------------------------------------------------------------------
// MATCHING INTERFACE:
//
// The "FullMatch" operation checks that supplied text matches a
// supplied pattern exactly.
//
// Example: successful match
//    regex::Regex re("h.*o");
//    re.FullMatch("hello");
//
// Example: unsuccessful match (requires full match):
//    regex::Regex re("e");
//    !re.FullMatch("hello");
//
// Example: creating a temporary Regex object:
//    regex::Regex("h.*o").FullMatch("hello");
//
// You can pass in a "const char*" or a "std::string" for "text".  The
// examples below tend to use a const char*.
//
// You can, as in the different examples above, store the Regex object
// explicitly in a variable or use a temporary Regex object.  The
// examples below use one mode or the other arbitrarily.  Either
// could correctly be used for any of these examples.
//
// -----------------------------------------------------------------------
// MATCHING WITH SUB-STRING EXTRACTION:
//
// You can supply extra pointer arguments to extract matched subpieces.
//
// Example: extracts "ruby" into "s" and 1234 into "i"
//    int i;
//    std::string s;
//    regex::Regex re("(\\w+):(\\d+)");
//    re.FullMatch("ruby:1234", &s, &i);
//
// Example: does not try to extract any extra sub-patterns
//    re.FullMatch("ruby:1234", &s);
//
// Example: does not try to extract into NULL
//    re.FullMatch("ruby:1234", NULL, &i);
//
// Example: integer overflow causes failure
//    !re.FullMatch("ruby:1234567891234", NULL, &i);
//
// Example: fails because there aren't enough sub-patterns:
//    !regex::Regex("\\w+:\\d+").FullMatch("ruby:1234", &s);
//
// Example: fails because string cannot be stored in integer
//    !regex::Regex("(.*)").FullMatch("ruby", &i);
//
// The provided pointer arguments can be pointers to any scalar numeric
// type, or one of
//    std::string   (matched piece is copied to string)
//    StringPiece   (StringPiece is mutated to point to matched piece)
//    T             (where "bool T::ParseFrom(const char*, int)" exists)
//    NULL          (the corresponding matched sub-pattern is not copied)
//
// CAVEAT: An optional sub-pattern that does not exist in the matched
// string is assigned the empty string.  Therefore, the following will
// return false (because the empty string is not a valid number):
//    int number;
//    regex::Regex::FullMatch("abc", "[a-z]+(\\d+)?", &number);
//
// -----------------------------------------------------------------------
// DO_MATCH
//
// The matching interface supports at most 16 arguments per call.
// If you need more, consider using the more general interface
// regex::Regex::DoMatch().  See regex.h for the signature for DoMatch.
//
// -----------------------------------------------------------------------
// PARTIAL MATCHES
//
// You can use the "PartialMatch" operation when you want the pattern
// to match any substring of the text.
//
// Example: simple search for a string:
//    regex::Regex("ell").PartialMatch("hello");
//
// Example: find first number in a string:
//    int number;
//    regex::Regex re("(\\d+)");
//    re.PartialMatch("x*100 + 20", &number);
//    assert(number == 100);
//
// -----------------------------------------------------------------------
// UTF-8 AND THE MATCHING INTERFACE:
//
// By default, pattern and text are plain text, one byte per character.
// The UTF8 flag, passed to the constructor, causes both pattern
// and string to be treated as UTF-8 text, still a byte stream but
// potentially multiple bytes per character. In practice, the text
// is likelier to be UTF-8 than the pattern, but the match returned
// may depend on the UTF8 flag, so always use it when matching
// UTF8 text.  E.g., "." will match one byte normally but with UTF8
// set may match up to three bytes of a multi-byte character.
//
// Example:
//    regex::Options options;
//    options.SetUtf8();
//    regex::Regex re(utf8_pattern, options);
//    re.FullMatch(utf8_string);
//
// Example: using the convenience function UTF8():
//    regex::Regex re(utf8_pattern, regex::UTF8());
//    re.FullMatch(utf8_string);
//
// NOTE: The UTF8 option is ignored if pcre was not configured with the
//       --enable-utf8 flag.
//
// -----------------------------------------------------------------------
// PASSING MODIFIERS TO THE REGULAR EXPRESSION ENGINE
//
// PCRE defines some modifiers to change the behavior of the regular
// expression engine.
// The C++ wrapper defines an auxiliary class, Options, as a vehicle
// to pass such modifiers to a Regex class.
//
// Currently, the following modifiers are supported
//
//    modifier              description               Perl corresponding
//
//    PCRE_CASELESS         case insensitive match    /i
//    PCRE_MULTILINE        multiple lines match      /m
//    PCRE_DOTALL           dot matches newlines      /s
//    PCRE_DOLLAR_ENDONLY   $ matches only at end     N/A
//    PCRE_EXTRA            strict escape parsing     N/A
//    PCRE_EXTENDED         ignore whitespaces        /x
//    PCRE_UTF8             handles UTF8 chars        built-in
//    PCRE_UNGREEDY         reverses * and *?         N/A
//    PCRE_NO_AUTO_CAPTURE  disables matching parens  N/A (*)
//
// (For a full account on how each modifier works, please check the
// PCRE API reference manual).
//
// (*) Both Perl and PCRE allow non matching parentheses by means of the
// "?:" modifier within the pattern itself. e.g. (?:ab|cd) does not
// capture, while (ab|cd) does.
//
// For each modifier, there are two member functions whose name is made
// out of the modifier in lowercase, without the "PCRE_" prefix. For
// instance, PCRE_CASELESS is handled by
//    bool caseless(),
// which returns true if the modifier is set, and
//    Options & SetCaseless(bool),
// which sets or unsets the modifier.
//
// Moreover, PCRE_EXTRA_MATCH_LIMIT can be accessed through the
// SetMatchLimit() and MatchLimit() member functions.
// Setting MatchLimit to a non-zero value will limit the executation of
// pcre to keep it from doing bad things like blowing the stack or taking
// an eternity to return a result.  A value of 5000 is good enough to stop
// stack blowup in a 2MB thread stack.  Setting MatchLimit to zero will
// disable match limiting.  Alternately, you can set MatchLimitRecursion()
// which uses PCRE_EXTRA_MATCH_LIMIT_RECURSION to limit how much pcre
// recurses.  MatchLimit() caps the number of matches pcre does;
// match_limit_recrusion() caps the depth of recursion.
//
// Normally, to pass one or more modifiers to a Regex class, you declare
// a Options object, set the appropriate options, and pass this
// object to a Regex constructor. Example:
//
//    Options opt;
//    opt.SetCaseless(true);
//
//    if (Regex("HELLO", opt).PartialMatch("hello world")) ...
//
// Options has two constructors. The default constructor takes no
// arguments and creates a set of flags that are off by default.
//
// The optional parameter 'option_flags' is to facilitate transfer
// of legacy code from C programs.  This lets you do
//    Regex(pattern, Options(PCRE_CASELESS|PCRE_MULTILINE)).PartialMatch(str);
//
// But new code is better off doing
//    Regex(pattern,
//      Options().SetCaseless(true).SetMultiLine(true)).PartialMatch(str);
// (See below)
//
// If you are going to pass one of the most used modifiers, there are some
// convenience functions that return a Options class with the
// appropriate modifier already set:
// CASELESS(), UTF8(), MULTILINE(), DOTALL(), EXTENDED()
//
// If you need to set several options at once, and you don't want to go
// through the pains of declaring a Options object and setting several
// options, there is a parallel method that give you such ability on the
// fly. You can concatenate several set_xxxxx member functions, since each
// of them returns a reference to its class object.  e.g.: to pass
// PCRE_CASELESS, PCRE_EXTENDED, and PCRE_MULTILINE to a Regex with one
// statement, you may write
//
//    Regex(" ^ xyz \\s+ .* blah$", Options()
//                            .SetCaseless(true)
//                            .SetExtended(true)
//                            .SetMultiLine(true)).PartialMatch(sometext);
//
// -----------------------------------------------------------------------
// SCANNING TEXT INCREMENTALLY
//
// The "Consume" operation may be useful if you want to repeatedly
// match regular expressions at the front of a string and skip over
// them as they match.  This requires use of the "StringPiece" type,
// which represents a sub-range of a real string.
//
// Example: read lines of the form "var = value" from a string.
//    std::string contents = ...;                 // Fill string somehow
//    regex::StringPiece input(contents);  // Wrap in a StringPiece
//
//    std::string var;
//    int value;
//    regex::Regex re("(\\w+) = (\\d+)\n");
//    while (re.Consume(&input, &var, &value)) {
//      ...;
//    }
//
// Each successful call to "Consume" will set "var/value", and also
// advance "input" so it points past the matched text.
//
// The "FindAndConsume" operation is similar to "Consume" but does not
// anchor your match at the beginning of the string.  For example, you
// could extract all words from a string by repeatedly calling
//     regex::Regex("(\\w+)").FindAndConsume(&input, &word)
//
// -----------------------------------------------------------------------
// PARSING HEX/OCTAL/C-RADIX NUMBERS
//
// By default, if you pass a pointer to a numeric value, the
// corresponding text is interpreted as a base-10 number.  You can
// instead wrap the pointer with a call to one of the operators Hex(),
// Octal(), or CRadix() to interpret the text in another base.  The
// CRadix operator interprets C-style "0" (base-8) and "0x" (base-16)
// prefixes, but defaults to base-10.
//
// Example:
//   int a, b, c, d;
//   regex::Regex re("(.*) (.*) (.*) (.*)");
//   re.FullMatch("100 40 0100 0x40",
//                regex::Octal(&a), regex::Hex(&b),
//                regex::CRadix(&c), regex::CRadix(&d));
// will leave 64 in a, b, c, and d.
//
// -----------------------------------------------------------------------
// REPLACING PARTS OF STRINGS
//
// You can replace the first match of "pattern" in "str" with
// "rewrite".  Within "rewrite", backslash-escaped digits (\1 to \9)
// can be used to insert text matching corresponding parenthesized
// group from the pattern.  \0 in "rewrite" refers to the entire
// matching text.  E.g.,
//
//   std::string s = "yabba dabba doo";
//   regex::Regex("b+").Replace("d", &s);
//
// will leave "s" containing "yada dabba doo".  The result is true if
// the pattern matches and a replacement occurs, or false otherwise.
//
// GlobalReplace() is like Replace(), except that it replaces all
// occurrences of the pattern in the string with the rewrite.
// Replacements are not subject to Regex-matching.  E.g.,
//
//   std::string s = "yabba dabba doo";
//   regex::Regex("b+").GlobalReplace("d", &s);
//
// will leave "s" containing "yada dada doo".  It returns the number
// of replacements made.
//
// Extract() is like Replace(), except that if the pattern matches,
// "rewrite" is copied into "out" (an additional argument) with
// substitutions.  The non-matching portions of "text" are ignored.
// Returns true iff a match occurred and the extraction happened
// successfully.  If no match occurs, the string is left unaffected.


#include <string>
#include "config.h"
#include "pcre.h"
#include " defines the ValueParser class
// This isn't technically needed here, but we include it
// anyway so folks who include regex.h don't have to.
#include "string_piece.h"

#define REGEX_SET_BOOL_OPTION(b, o) \
    if (b) m_all_options |= (o); else m_all_options &= ~(o); \
    return *this

#define REGEX_OPTION_IS_SET(o)  \
    (m_all_options & o) == o

// namespace common {

/***** Compiling regular expressions: the Regex class *****/
// Interface for regular expression matching.  Also corresponds to a
// pre-compiled regular expression.  An "Regex" object is safe for
// concurrent use by multiple threads.
class REGEX_EXPORT_DEFINE Regex {
public:
    // Options allow you to set options to be passed along to pcre,
    // along with other options we put on top of pcre.
    // Only 9 modifiers, plus MatchLimit and MatchLimitRecursion,
    // are supported now.
    class REGEX_EXPORT_DEFINE Options {
    public:
        // constructor
        Options() : m_match_limit(0), m_match_limit_recursion(0), m_all_options(0) {}

        // alternative constructor.
        // To facilitate transfer of legacy code from C programs
        //
        // This lets you do
        //    Regex(pattern, Options(PCRE_CASELESS|PCRE_MULTILINE)).PartialMatch(str);
        // But new code is better off doing
        //    Regex(pattern,
        //      Options().SetCaseless(true).SetMultiLine(true)).PartialMatch(str);
        Options(int option_flags) : m_match_limit(0), m_match_limit_recursion(0),
        m_all_options(option_flags) {}
        // we'Regex fine with the default destructor, copy constructor, etc.

        // accessors and mutators
        int MatchLimit() const { return m_match_limit; };
        Options& SetMatchLimit(int limit) {
            m_match_limit = limit;
            return *this;
        }

        int MatchLimitRecursion() const {
            return m_match_limit_recursion;
        }
        Options& SetMatchLimitRecursion(int limit) {
            m_match_limit_recursion = limit;
            return *this;
        }

        bool Caseless() const {
            return REGEX_OPTION_IS_SET(PCRE_CASELESS);
        }
        Options& SetCaseless(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_CASELESS);
        }

        bool Multiline() const {
            return REGEX_OPTION_IS_SET(PCRE_MULTILINE);
        }
        Options& SetMultiLine(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_MULTILINE);
        }

        bool DotAll() const {
            return REGEX_OPTION_IS_SET(PCRE_DOTALL);
        }
        Options& SetDotAll(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_DOTALL);
        }

        bool Extended() const {
            return REGEX_OPTION_IS_SET(PCRE_EXTENDED);
        }
        Options& SetExtended(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_EXTENDED);
        }

        bool DollarEndOnly() const {
            return REGEX_OPTION_IS_SET(PCRE_DOLLAR_ENDONLY);
        }
        Options& SetDollarEndOnly(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_DOLLAR_ENDONLY);
        }

        bool Extra() const {
            return REGEX_OPTION_IS_SET(PCRE_EXTRA);
        }
        Options& SetExtra(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_EXTRA);
        }

        bool Ungreedy() const {
            return REGEX_OPTION_IS_SET(PCRE_UNGREEDY);
        }
        Options& SetUngreedy(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_UNGREEDY);
        }

        bool Utf8() const {
            return REGEX_OPTION_IS_SET(PCRE_UTF8);
        }
        Options& SetUtf8(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_UTF8);
        }

        bool NoAutoCapture() const {
            return REGEX_OPTION_IS_SET(PCRE_NO_AUTO_CAPTURE);
        }
        Options& SetNoAutoCapture(bool value = true) {
            REGEX_SET_BOOL_OPTION(value, PCRE_NO_AUTO_CAPTURE);
        }

        Options& SetAllOptions(int option) {
            m_all_options = option;
            return *this;
        }
        int AllOptions() const {
            return m_all_options ;
        }

        // TODO: add other pcre flags

    private:
        int m_match_limit;
        int m_match_limit_recursion;
        int m_all_options;
    };

    // These functions return some common Options
    static inline Options UTF8() {
        return Options().SetUtf8(true);
    }

    static inline Options CASELESS() {
        return Options().SetCaseless(true);
    }
    static inline Options MULTI_LINE() {
        return Options().SetMultiLine(true);
    }

    static inline Options DOT_ALL() {
        return Options().SetDotAll(true);
    }

    static inline Options EXTENDED() {
        return Options().SetExtended(true);
    }

public:
    // We provide implicit conversions from strings so that users can
    // pass in a string or a "const char*" wherever an "Regex" is expected.
    Regex(const std::string& pat) {
        Init(pat, NULL);
    }
    Regex(const std::string& pat, const Options& option) {
        Init(pat, &option);
    }

    Regex(const char* pat) { Init(pat, NULL); }
    Regex(const char* pat, const Options& option) {
        Init(pat, &option);
    }

    Regex(const unsigned char* pat) {
        Init(reinterpret_cast<const char*>(pat), NULL);
    }
    Regex(const unsigned char* pat, const Options& option) {
        Init(reinterpret_cast<const char*>(pat), &option);
    }

    // Copy constructor & assignment - note that these are expensive
    // because they recompile the expression.
    Regex(const Regex& regex);
    const Regex& operator=(const Regex& re);

    ~Regex();

    // The string specification for this Regex.  E.g.
    //   Regex re("ab*c?d+");
    //   re.Pattern();    // "ab*c?d+"
    const std::string& Pattern() const { return m_pattern; }

    // If Regex could not be created properly, returns an error string.
    // Else returns the empty string.
    const std::string& Error() const { return *m_error; }

    /***** The useful part: the matching interface *****/

    // This is provided so one can do pattern.ReplaceAll() just as
    // easily as ReplaceAll(pattern-text, ....)

    bool FullMatch(const StringPiece& text,
                   const ValueParser& capture_ptr1 = no_more_args,
                   const ValueParser& capture_ptr2 = no_more_args,
                   const ValueParser& capture_ptr3 = no_more_args,
                   const ValueParser& capture_ptr4 = no_more_args,
                   const ValueParser& capture_ptr5 = no_more_args,
                   const ValueParser& capture_ptr6 = no_more_args,
                   const ValueParser& capture_ptr7 = no_more_args,
                   const ValueParser& capture_ptr8 = no_more_args,
                   const ValueParser& capture_ptr9 = no_more_args,
                   const ValueParser& capture_ptr10 = no_more_args,
                   const ValueParser& capture_ptr11 = no_more_args,
                   const ValueParser& capture_ptr12 = no_more_args,
                   const ValueParser& capture_ptr13 = no_more_args,
                   const ValueParser& capture_ptr14 = no_more_args,
                   const ValueParser& capture_ptr15 = no_more_args,
                   const ValueParser& capture_ptr16 = no_more_args) const;

    bool PartialMatch(const StringPiece& text,
                      const ValueParser& capture_ptr1 = no_more_args,
                      const ValueParser& capture_ptr2 = no_more_args,
                      const ValueParser& capture_ptr3 = no_more_args,
                      const ValueParser& capture_ptr4 = no_more_args,
                      const ValueParser& capture_ptr5 = no_more_args,
                      const ValueParser& capture_ptr6 = no_more_args,
                      const ValueParser& capture_ptr7 = no_more_args,
                      const ValueParser& capture_ptr8 = no_more_args,
                      const ValueParser& capture_ptr9 = no_more_args,
                      const ValueParser& capture_ptr10 = no_more_args,
                      const ValueParser& capture_ptr11 = no_more_args,
                      const ValueParser& capture_ptr12 = no_more_args,
                      const ValueParser& capture_ptr13 = no_more_args,
                      const ValueParser& capture_ptr14 = no_more_args,
                      const ValueParser& capture_ptr15 = no_more_args,
                      const ValueParser& capture_ptr16 = no_more_args) const;

    bool Consume(StringPiece* input,
                 const ValueParser& capture_ptr1 = no_more_args,
                 const ValueParser& capture_ptr2 = no_more_args,
                 const ValueParser& capture_ptr3 = no_more_args,
                 const ValueParser& capture_ptr4 = no_more_args,
                 const ValueParser& capture_ptr5 = no_more_args,
                 const ValueParser& capture_ptr6 = no_more_args,
                 const ValueParser& capture_ptr7 = no_more_args,
                 const ValueParser& capture_ptr8 = no_more_args,
                 const ValueParser& capture_ptr9 = no_more_args,
                 const ValueParser& capture_ptr10 = no_more_args,
                 const ValueParser& capture_ptr11 = no_more_args,
                 const ValueParser& capture_ptr12 = no_more_args,
                 const ValueParser& capture_ptr13 = no_more_args,
                 const ValueParser& capture_ptr14 = no_more_args,
                 const ValueParser& capture_ptr15 = no_more_args,
                 const ValueParser& capture_ptr16 = no_more_args) const;

    bool FindAndConsume(StringPiece* input,
                        const ValueParser& capture_ptr1 = no_more_args,
                        const ValueParser& capture_ptr2 = no_more_args,
                        const ValueParser& capture_ptr3 = no_more_args,
                        const ValueParser& capture_ptr4 = no_more_args,
                        const ValueParser& capture_ptr5 = no_more_args,
                        const ValueParser& capture_ptr6 = no_more_args,
                        const ValueParser& capture_ptr7 = no_more_args,
                        const ValueParser& capture_ptr8 = no_more_args,
                        const ValueParser& capture_ptr9 = no_more_args,
                        const ValueParser& capture_ptr10 = no_more_args,
                        const ValueParser& capture_ptr11 = no_more_args,
                        const ValueParser& capture_ptr12 = no_more_args,
                        const ValueParser& capture_ptr13 = no_more_args,
                        const ValueParser& capture_ptr14 = no_more_args,
                        const ValueParser& capture_ptr15 = no_more_args,
                        const ValueParser& capture_ptr16 = no_more_args) const;

    bool Replace(const StringPiece& rewrite,
                 std::string *str) const;

    int GlobalReplace(const StringPiece& rewrite,
                      std::string *str) const;

    bool Extract(const StringPiece &rewrite,
                 const StringPiece &text,
                 std::string *out) const;

    // Escapes all potentially meaningful regexp characters in
    // 'unquoted'.  The returned string, used as a regular expression,
    // will exactly match the original string.  For example,
    //           1.5-2.0?
    // may become:
    //           1\.5\-2\.0\?
    // Note QuoteMeta behaves the same as perl's QuoteMeta function,
    // *except* that it escapes the NUL character (\0) as backslash + 0,
    // rather than backslash + NUL.
    static std::string QuoteMeta(const StringPiece& unquoted);


    /***** Generic matching interface *****/

    // Type of match (TODO: Should be restructured as part of Options)
    enum Anchor {
        UNANCHORED,         // No anchoring
        ANCHOR_START,       // Anchor at start only
        ANCHOR_BOTH         // Anchor at start and end
    };

    // General matching routine.  Stores the length of the match in
    // "*consumed" if successful.
    bool DoMatch(const StringPiece& text,
                 Anchor anchor,
                 int* consumed,
                 const ValueParser* const* args, int nargs) const;

    // Return the number of capturing subpatterns, or -1 if the
    // regexp wasn't valid on construction.
    int NumberOfCapturingGroups() const;

    // The default value for an argument, to indicate the end of the argument
    // list. This must be used only in optional argument defaults. It should NOT
    // be passed explicitly. Some people have tried to use it like this:
    //
    //   FullMatch(x, y, &z, no_more_args, &w);
    //
    // This is a mistake, and will not work.
    static ValueParser no_more_args;

private:
    void Init(const std::string& pattern, const Options* options);
    void Cleanup();

    // Match against "text", filling in "vec" (up to "vecsize" * 2/3) with
    // pairs of integers for the beginning and end positions of matched
    // text.  The first pair corresponds to the entire matched text;
    // subsequent pairs correspond, in order, to parentheses-captured
    // matches.  Returns the number of pairs (one more than the number of
    // the last subpattern with a match) if matching was successful
    // and zero if the match failed.
    // I.e. for Regex("(foo)|(bar)|(baz)") it will return 2, 3, and 4 when matching
    // against "foo", "bar", and "baz" respectively.
    // When matching Regex("(foo)|hello") against "hello", it will return 1.
    // But the values for all subpattern are filled in into "vec".
    int TryMatch(const StringPiece& text,
                 int startpos,
                 Anchor anchor,
                 bool empty_ok,
                 int *vec,
                 int vecsize) const;

    // Append the "rewrite" string, with backslash subsitutions from "text"
    // and "vec", to string "out".
    bool Rewrite(std::string *out,
                 const StringPiece& rewrite,
                 const StringPiece& text,
                 int *vec,
                 int veclen) const;

    // internal implementation for DoMatch
    bool DoMatchImpl(const StringPiece& text,
                     Anchor anchor,
                     int* consumed,
                     const ValueParser* const args[],
                     int nargs,
                     int* vec,
                     int vecsize) const;

    // Compile the regexp for the specified anchoring mode
    pcre* Compile(Anchor anchor);
public:
    // Maximum number of args we can set
    static const int kMaxArgs = 16;
    static const int kVectorSize = (1 + kMaxArgs) * 3;  // results + PCRE workspace
private:
    std::string        m_pattern;
    Options       m_options;
    pcre*         m_re_full;       // For full matches
    pcre*         m_re_partial;    // For partial matches
    const std::string* m_error;         // Error indicator (or points to empty string)
};

// } // namespace common

#undef REGEX_SET_BOOL_OPTION
#undef REGEX_OPTION_IS_SET

#endif // REGEX_H
