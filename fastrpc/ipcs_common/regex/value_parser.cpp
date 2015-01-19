#include "value_parser.h"
#include <limits.h>
#include <errno.h>

bool ValueParser::parse_null(const char* str, int n, void* dest) {
    // We fail if somebody asked us to store into a non-NULL void* pointer
    return (dest == NULL);
}

bool ValueParser::parse_string(const char* str, int n, void* dest) {
    if (dest == NULL) return true;
    reinterpret_cast<std::string*>(dest)->assign(str, n);
    return true;
}

bool ValueParser::parse_stringpiece(const char* str, int n, void* dest) {
    if (dest == NULL) return true;
    reinterpret_cast<StringPiece*>(dest)->set(str, n);
    return true;
}

bool ValueParser::parse_char(const char* str, int n, void* dest) {
    if (n != 1) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<char*>(dest)) = str[0];
    return true;
}

bool ValueParser::parse_uchar(const char* str, int n, void* dest) {
    if (n != 1) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<unsigned char*>(dest)) = str[0];
    return true;
}

// Largest number spec that we are willing to parse
static const int kMaxNumberLength = 32;

// REQUIRES "buf" must have length at least kMaxNumberLength+1
// REQUIRES "n > 0"
// Copies "str" into "buf" and null-terminates if necessary.
// Returns one of:
//      a. "str" if no termination is needed
//      b. "buf" if the string was copied and null-terminated
//      c. "" if the input was invalid and has no hope of being parsed
static const char* TerminateNumber(char* buf, const char* str, int n) {
    if ((n > 0) && isspace(*str)) {
        // We are less forgiving than the strtoxxx() routines and do not
        // allow leading spaces.
        return "";
    }

    // See if the character right after the input text may potentially
    // look like a digit.
    if (isdigit(str[n]) ||
        ((str[n] >= 'a') && (str[n] <= 'f')) ||
        ((str[n] >= 'A') && (str[n] <= 'F'))) {
        if (n > kMaxNumberLength) return ""; // Input too big to be a valid number
        memcpy(buf, str, n);
        buf[n] = '\0';
        return buf;
    } else {
        // We can parse right out of the supplied string, so return it.
        return str;
    }
}

bool ValueParser::parse_long_radix(const char* str,
                                   int n,
                                   void* dest,
                                   int radix) {
    if (n == 0) return false;
    char buf[kMaxNumberLength+1];
    str = TerminateNumber(buf, str, n);
    char* end;
    errno = 0;
    long r = strtol(str, &end, radix);
    if (end != str + n) return false;   // Leftover junk
    if (errno) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<long*>(dest)) = r;
    return true;
}

bool ValueParser::parse_ulong_radix(const char* str,
                                    int n,
                                    void* dest,
                                    int radix) {
    if (n == 0) return false;
    char buf[kMaxNumberLength+1];
    str = TerminateNumber(buf, str, n);
    if (str[0] == '-') return false;    // strtoul() on a negative number?!
    char* end;
    errno = 0;
    unsigned long r = strtoul(str, &end, radix);
    if (end != str + n) return false;   // Leftover junk
    if (errno) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<unsigned long*>(dest)) = r;
    return true;
}

bool ValueParser::parse_short_radix(const char* str,
                                    int n,
                                    void* dest,
                                    int radix) {
    long r;
    if (!parse_long_radix(str, n, &r, radix)) return false; // Could not parse
    if (r < SHRT_MIN || r > SHRT_MAX) return false;       // Out of range
    if (dest == NULL) return true;
    *(reinterpret_cast<short*>(dest)) = static_cast<short>(r);
    return true;
}

bool ValueParser::parse_ushort_radix(const char* str,
                                     int n,
                                     void* dest,
                                     int radix) {
    unsigned long r;
    if (!parse_ulong_radix(str, n, &r, radix)) return false; // Could not parse
    if (r > USHRT_MAX) return false;                      // Out of range
    if (dest == NULL) return true;
    *(reinterpret_cast<unsigned short*>(dest)) = static_cast<unsigned short>(r);
    return true;
}

bool ValueParser::parse_int_radix(const char* str,
                                  int n,
                                  void* dest,
                                  int radix) {
    long r;
    if (!parse_long_radix(str, n, &r, radix)) return false; // Could not parse
    if (r < INT_MIN || r > INT_MAX) return false;         // Out of range
    if (dest == NULL) return true;
    *(reinterpret_cast<int*>(dest)) = r;
    return true;
}

bool ValueParser::parse_uint_radix(const char* str,
                                   int n,
                                   void* dest,
                                   int radix) {
    unsigned long r;
    if (!parse_ulong_radix(str, n, &r, radix)) return false; // Could not parse
    if (r > UINT_MAX) return false;                       // Out of range
    if (dest == NULL) return true;
    *(reinterpret_cast<unsigned int*>(dest)) = r;
    return true;
}

bool ValueParser::parse_longlong_radix(const char* str,
                                       int n,
                                       void* dest,
                                       int radix) {
    if (n == 0) return false;
    char buf[kMaxNumberLength+1];
    str = TerminateNumber(buf, str, n);
    char* end;
    errno = 0;
#if defined __BSD__
    long long r = strtoq(str, &end, radix);
#elif defined __unix__
    long long r = strtoll(str, &end, radix);
#elif defined _WIN32
    long long r = _strtoi64(str, &end, radix);
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
    long long r = strtoimax(str, &end, radix);
#else
#error parse_longlong_radix: cannot convert input to a long-long
#endif
    if (end != str + n) return false;   // Leftover junk
    if (errno) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<long long*>(dest)) = r;
    return true;
}

bool ValueParser::parse_ulonglong_radix(const char* str,
                                        int n,
                                        void* dest,
                                        int radix) {
    if (n == 0) return false;
    char buf[kMaxNumberLength+1];
    str = TerminateNumber(buf, str, n);
    if (str[0] == '-') return false;    // strtoull() on a negative number?!
    char* end;
    errno = 0;
#if defined __BSD__
    unsigned long long r = strtouq(str, &end, radix);
#elif defined __unix__
    unsigned long long r = strtoull(str, &end, radix);
#elif defined _WIN32
    unsigned long long r = _strtoui64(str, &end, radix);
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
    unsigned long long r = strtoumax(str, &end, radix);
#else
#error parse_ulonglong_radix: cannot convert input to a long-long
#endif
    if (end != str + n) return false;   // Leftover junk
    if (errno) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<unsigned long long*>(dest)) = r;
    return true;
}

bool ValueParser::parse_double(const char* str, int n, void* dest) {
    if (n == 0) return false;
    static const int kMaxLength = 200;
    char buf[kMaxLength];
    if (n >= kMaxLength) return false;
    memcpy(buf, str, n);
    buf[n] = '\0';
    errno = 0;
    char* end;
    double r = strtod(buf, &end);
    if (end != buf + n) return false;   // Leftover junk
    if (errno) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<double*>(dest)) = r;
    return true;
}

bool ValueParser::parse_float(const char* str, int n, void* dest) {
    double r;
    if (!parse_double(str, n, &r)) return false;
    if (dest == NULL) return true;
    *(reinterpret_cast<float*>(dest)) = static_cast<float>(r);
    return true;
}


#define DEFINE_INTEGER_PARSERS(name)                                               \
    bool ValueParser::parse_##name(const char* str, int n, void* dest) {          \
        return parse_##name##_radix(str, n, dest, 10);                                 \
    }                                                                                \
bool ValueParser::parse_##name##_hex(const char* str, int n, void* dest) {    \
    return parse_##name##_radix(str, n, dest, 16);                                 \
}                                                                                \
bool ValueParser::parse_##name##_octal(const char* str, int n, void* dest) {  \
    return parse_##name##_radix(str, n, dest, 8);                                  \
}                                                                                \
bool ValueParser::parse_##name##_cradix(const char* str, int n, void* dest) { \
    return parse_##name##_radix(str, n, dest, 0);                                  \
}

DEFINE_INTEGER_PARSERS(short)      /*                                   */
DEFINE_INTEGER_PARSERS(ushort)     /*                                   */
DEFINE_INTEGER_PARSERS(int)        /* Don't use semicolons after these  */
DEFINE_INTEGER_PARSERS(uint)       /* statements because they can cause */
DEFINE_INTEGER_PARSERS(long)       /* compiler warnings if the checking */
DEFINE_INTEGER_PARSERS(ulong)      /* level is turned up high enough.   */
DEFINE_INTEGER_PARSERS(longlong)   /*                                   */
DEFINE_INTEGER_PARSERS(ulonglong)  /*                                   */

#undef DEFINE_INTEGER_PARSERS
