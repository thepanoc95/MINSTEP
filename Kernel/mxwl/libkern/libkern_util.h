#ifndef MXWL_LIBKERN_UTIL_H
#define MXWL_LIBKERN_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include "libkern_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline int is_digit(int c)
{
    return c >= '0' && c <= '9';
}

static inline int is_xdigit(int c)
{
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static inline int is_alpha(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int is_alnum(int c)
{
    return is_alpha(c) || is_digit(c);
}

static inline int is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static inline int is_print(int c)
{
    return c >= 32 && c <= 126;
}

static inline int is_upper(int c)
{
    return c >= 'A' && c <= 'Z';
}

static inline int is_lower(int c)
{
    return c >= 'a' && c <= 'z';
}

static inline int to_lower(int c)
{
    return is_upper(c) ? c + 32 : c;
}

static inline int to_upper(int c)
{
    return is_lower(c) ? c - 32 : c;
}

static inline int digit_val(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static inline uint16_t swap16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

static inline uint32_t swap32(uint32_t x)
{
    return ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00) |
           ((x << 8) & 0xFF0000) | (x << 24);
}

static inline uint64_t swap64(uint64_t x)
{
    uint32_t lo = (uint32_t)(x);
    uint32_t hi = (uint32_t)(x >> 32);
    return ((uint64_t)swap32(lo) << 32) | swap32(hi);
}

static inline int sign_extend32(uint32_t x, int bits)
{
    int shift = 32 - bits;
    return (int)((int32_t)(x << shift) >> shift);
}

static inline int64_t sign_extend64(uint64_t x, int bits)
{
    int shift = 64 - bits;
    return (int64_t)((int64_t)(x << shift) >> shift);
}

size_t libkern_hexdump(char *buf, size_t n, const void *data, size_t len);
int    libkern_snprintf(char *buf, size_t n, const char *fmt, ...)
       __attribute__((format(printf, 3, 4)));
int    libkern_atoi(const char *s);
long   libkern_atol(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_UTIL_H */
