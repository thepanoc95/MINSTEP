#ifndef MXWL_LIBKERN_ASSERT_H
#define MXWL_LIBKERN_ASSERT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void libkern_assert_fail(const char *expr, const char *file, int line, const char *func);

#ifdef NDEBUG
#define ASSERT(expr)          ((void)0)
#define ASSERT_MSG(expr, msg) ((void)0)
#else
#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            libkern_assert_fail(#expr, __FILE__, __LINE__, __func__); \
        } \
    } while (0)

#define ASSERT_MSG(expr, msg) \
    do { \
        if (!(expr)) { \
            libkern_assert_fail(msg, __FILE__, __LINE__, __func__); \
        } \
    } while (0)
#endif

#define STATIC_ASSERT(cond, msg) \
    typedef char _static_assert_##msg[(cond) ? 1 : -1]

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_ASSERT_H */
