// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
// This code is based on Lua 5.x implementation licensed under MIT License; see lua_LICENSE.txt for details
#pragma once

#include <math.h>
#include <cstdint>

#define luai_numadd(a, b) ((a) + (b))
#define luai_numsub(a, b) ((a) - (b))
#define luai_nummul(a, b) ((a) * (b))
#define luai_numdiv(a, b) ((a) / (b))
#define luai_numpow(a, b) (pow(a, b))

// A power by repeated squaring, for a whole exponent: plain multiplications, which every platform
// rounds alike, where the C library's pow() need not. False, computing nothing, for an exponent that
// is not whole, or past 2^53, as rounding those the same everywhere has no cheap answer.
static inline bool luai_wholepow(double a, double b, double* result)
{
    if (!(b == floor(b)) || !(fabs(b) <= 9007199254740992.0))
        return false;

    unsigned long long n = (unsigned long long)fabs(b);
    double base = a;
    double r = 1.0;

    while (n)
    {
        if (n & 1)
            r *= base;
        n >>= 1;
        if (n)
            base *= base;
    }

    *result = b < 0 ? 1.0 / r : r;
    return true;
}

#define LUAU_FRACTIONAL_POWER "a power takes a whole exponent: a fractional one rounds differently from one platform to another"
#define luai_numunm(a) (-(a))
#define luai_numisnan(a) ((a) != (a))
#define luai_numeq(a, b) ((a) == (b))
#define luai_numlt(a, b) ((a) < (b))
#define luai_numle(a, b) ((a) <= (b))
#define luai_inteq(a, b) ((a) == (b))

inline bool luai_veceq(const float* a, const float* b)
{
#if LUA_VECTOR_SIZE == 4
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
#else
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
#endif
}

inline bool luai_veceq(const double* a, const double* b)
{
#if LUA_VECTOR_SIZE == 4
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
#else
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
#endif
}

inline bool luai_vecisnan(const float* a)
{
#if LUA_VECTOR_SIZE == 4
    return a[0] != a[0] || a[1] != a[1] || a[2] != a[2] || a[3] != a[3];
#else
    return a[0] != a[0] || a[1] != a[1] || a[2] != a[2];
#endif
}

inline bool luai_vecisnan(const double* a)
{
#if LUA_VECTOR_SIZE == 4
    return a[0] != a[0] || a[1] != a[1] || a[2] != a[2] || a[3] != a[3];
#else
    return a[0] != a[0] || a[1] != a[1] || a[2] != a[2];
#endif
}

inline float luai_signf(float v)
{
    return v > 0.0f ? 1.0f : v < 0.0f ? -1.0f : 0.0f;
}

inline double luai_signd(double v)
{
    return v > 0.0 ? 1.0 : v < 0.0 ? -1.0 : 0.0;
}

inline float luai_clampf(float v, float min, float max)
{
    float r = v < min ? min : v;
    return r > max ? max : r;
}

inline double luai_clampd(double v, double min, double max)
{
    double r = v < min ? min : v;
    return r > max ? max : r;
}

LUAU_FASTMATH_BEGIN
inline double luai_nummod(double a, double b)
{
    return a - floor(a / b) * b;
}
LUAU_FASTMATH_END

LUAU_FASTMATH_BEGIN
inline double luai_numidiv(double a, double b)
{
    return floor(a / b);
}
LUAU_FASTMATH_END

// Numbers are whole in this fork, within 2^53 - 1 either way: the whole numbers a double holds
// exactly. A sum, difference or product of two of them is exact while its true value is within the
// bound, and rounds to 2^53 or past it when it is not, so checking each result is enough to keep
// every number exact. A division, floor division or remainder of two of them is worked out in
// integers instead, as a quotient that is not whole can round to one, and floor(a / b) can round
// to the wrong side. Anything else, a fraction, an infinity or NaN, is refused where it is made.
#define LUAI_MAXWHOLE 9007199254740991.0

#define LUAU_NOT_WHOLE "numbers are whole, within 2^53 either way: this would be a fraction, infinite or past 2^53 (divide with //, or use the integer types)"

inline bool luai_iswhole(double n)
{
    return fabs(n) <= LUAI_MAXWHOLE && n == floor(n);
}

// a / b when it divides evenly, else NaN, which the caller refuses; a fraction from the host
// divides as a double and is checked like any result
inline double luai_wholediv(double a, double b)
{
    if (!luai_iswhole(a) || !luai_iswhole(b))
        return a / b;
    if (b == 0)
        return NAN;

    int64_t x = int64_t(a), y = int64_t(b);
    return x % y == 0 ? double(x / y) : NAN;
}

// floor(a / b), exactly
inline double luai_wholeidiv(double a, double b)
{
    if (!luai_iswhole(a) || !luai_iswhole(b))
        return luai_numidiv(a, b);
    if (b == 0)
        return NAN;

    int64_t x = int64_t(a), y = int64_t(b);
    int64_t q = x / y;
    if (x % y != 0 && ((x < 0) != (y < 0)))
        q -= 1;
    return double(q);
}

// a - floor(a / b) * b, exactly: the remainder takes the divisor's sign, as Lua's does
inline double luai_wholemod(double a, double b)
{
    if (!luai_iswhole(a) || !luai_iswhole(b))
        return luai_nummod(a, b);
    if (b == 0)
        return NAN;

    int64_t x = int64_t(a), y = int64_t(b);
    int64_t r = x % y;
    if (r != 0 && ((r < 0) != (y < 0)))
        r += y;
    return double(r);
}

inline float luai_lerpf(float a, float b, float t)
{
    return (t == 1.0f) ? b : a + (b - a) * t;
}

inline double luai_lerpd(double a, double b, double t)
{
    return (t == 1.0) ? b : a + (b - a) * t;
}

#define luai_num2int(i, d) ((i) = (int)(d))

#define luai_num2long(i, d) ((i) = (int64_t)(d))

#define luai_fabs condvectordouble(fabs, fabsf)
#define luai_sqrt condvectordouble(sqrt, sqrtf)
#define luai_floor condvectordouble(floor, floorf)
#define luai_ceil condvectordouble(ceil, ceilf)
#define luai_sign condvectordouble(luai_signd, luai_signf)
#define luai_clamp condvectordouble(luai_clampd, luai_clampf)
#define luai_lerp condvectordouble(luai_lerpd, luai_lerpf)

// On MSVC in 32-bit, double to unsigned cast compiles into a call to __dtoui3, so we invoke x87->int64 conversion path manually
#if defined(_MSC_VER) && defined(_M_IX86)
#define luai_num2unsigned(i, n) \
    { \
        __int64 l; \
        __asm { __asm fld n __asm fistp l} \
        ; \
        i = (unsigned int)l; \
    }
#else
#define luai_num2unsigned(i, n) ((i) = (unsigned)(long long)(n))
#endif

#define LUAI_MAXNUM2STR 48
#define LUAI_MAXINT2STR 30

LUAI_FUNC char* luai_num2str(char* buf, double n);
LUAI_FUNC char* luai_int2str(char* buf, int64_t n);

#define luai_str2num(s, p) strtod((s), (p))
#define luai_str2long(s, p, base) strtoll((s), (p), base)
