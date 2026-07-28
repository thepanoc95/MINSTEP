#ifndef MXWL_LIBKERN_MACROS_H
#define MXWL_LIBKERN_MACROS_H

#define MXWL_INLINE static inline __attribute__((always_inline))
#define MXWL_UNUSED  __attribute__((unused))
#define MXWL_USED    __attribute__((used))
#define MXWL_PACKED  __attribute__((packed))
#define MXWL_ALIGNED(x) __attribute__((aligned(x)))

#define MIN(a, b)     ((a) < (b) ? (a) : (b))
#define MAX(a, b)     ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) (((x) < (lo)) ? (lo) : (((x) > (hi)) ? (hi) : (x)))

#define COUNT_OF(x)   (sizeof(x) / sizeof((x)[0]))

#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - __builtin_offsetof(type, member)))

#define BIT(n)        (1UL << (n))
#define BIT_MASK(n)   (BIT(n) - 1)
#define BIT_SET(x, n)   ((x) |= BIT(n))
#define BIT_CLR(x, n)   ((x) &= ~BIT(n))
#define BIT_ISSET(x, n) ((x) & BIT(n))

#define ROUND_DOWN(x, align) ((x) & ~((align) - 1))
#define ROUND_UP(x, align)   (((x) + (align) - 1) & ~((align) - 1))

#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

#define BARRIER()     __asm__ __volatile__("" ::: "memory")

#endif /* MXWL_LIBKERN_MACROS_H */
