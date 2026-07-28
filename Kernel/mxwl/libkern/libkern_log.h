#ifndef MXWL_LIBKERN_LOG_H
#define MXWL_LIBKERN_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_EMERG     0
#define LOG_CRIT      1
#define LOG_ERR       2
#define LOG_WARN      3
#define LOG_NOTICE    4
#define LOG_INFO      5
#define LOG_DEBUG     6

void libkern_log(int level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

void libkern_log_set_level(int level);

int  libkern_log_get_level(void);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_LOG_H */
