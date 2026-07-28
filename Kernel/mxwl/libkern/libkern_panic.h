#ifndef MXWL_LIBKERN_PANIC_H
#define MXWL_LIBKERN_PANIC_H

#ifdef __cplusplus
extern "C" {
#endif

void libkern_panic(const char *fmt, ...) __attribute__((noreturn));
void libkern_panic_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_PANIC_H */
