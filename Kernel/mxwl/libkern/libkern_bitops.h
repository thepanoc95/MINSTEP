#ifndef MXWL_LIBKERN_BITOPS_H
#define MXWL_LIBKERN_BITOPS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define bit_ffs(x)     __builtin_ffs(x)
#define bit_clz(x)     __builtin_clz(x)
#define bit_ctz(x)     __builtin_ctz(x)
#define bit_popcount(x) __builtin_popcount(x)
#define bit_ffsl(x)    __builtin_ffsl(x)
#define bit_clzl(x)    __builtin_clzl(x)
#define bit_ctzl(x)    __builtin_ctzl(x)
#define bit_popcountl(x) __builtin_popcountl(x)

static inline void bit_set(uint32_t *bits, int n)
{
    bits[n / 32] |= (1U << (n % 32));
}

static inline void bit_clear(uint32_t *bits, int n)
{
    bits[n / 32] &= ~(1U << (n % 32));
}

static inline int bit_test(const uint32_t *bits, int n)
{
    return (bits[n / 32] >> (n % 32)) & 1;
}

static inline void bit_set64(uint64_t *bits, int n)
{
    bits[n / 64] |= (1ULL << (n % 64));
}

static inline void bit_clear64(uint64_t *bits, int n)
{
    bits[n / 64] &= ~(1ULL << (n % 64));
}

static inline int bit_test64(const uint64_t *bits, int n)
{
    return (bits[n / 64] >> (n % 64)) & 1;
}

static inline uint32_t bit_ror32(uint32_t x, int r)
{
    return (x >> r) | (x << (32 - r));
}

static inline uint32_t bit_rol32(uint32_t x, int r)
{
    return (x << r) | (x >> (32 - r));
}

static inline uint64_t bit_ror64(uint64_t x, int r)
{
    return (x >> r) | (x << (64 - r));
}

static inline uint64_t bit_rol64(uint64_t x, int r)
{
    return (x << r) | (x >> (64 - r));
}

static inline int bit_is_pow2(uint32_t x)
{
    return x && !(x & (x - 1));
}

static inline uint32_t bit_next_pow2(uint32_t x)
{
    if (x == 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_BITOPS_H */
