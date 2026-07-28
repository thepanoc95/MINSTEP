#ifndef MXWL_LIBKERN_STRING_H
#define MXWL_LIBKERN_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void  libkern_memcpy(void *dst, const void *src, size_t n);
void  libkern_memmove(void *dst, const void *src, size_t n);
void *libkern_memset(void *s, int c, size_t n);
int   libkern_memcmp(const void *s1, const void *s2, size_t n);

size_t libkern_strlen(const char *s);
size_t libkern_strnlen(const char *s, size_t maxlen);
char  *libkern_strcpy(char *dst, const char *src);
char  *libkern_strncpy(char *dst, const char *src, size_t n);
int    libkern_strcmp(const char *s1, const char *s2);
int    libkern_strncmp(const char *s1, const char *s2, size_t n);
char  *libkern_strcat(char *dst, const char *src);
char  *libkern_strncat(char *dst, const char *src, size_t n);
char  *libkern_strchr(const char *s, int c);
char  *libkern_strrchr(const char *s, int c);
char  *libkern_strstr(const char *haystack, const char *needle);
char  *libkern_strdup(const char *s);

size_t libkern_strlcpy(char *dst, const char *src, size_t n);
size_t libkern_strlcat(char *dst, const char *src, size_t n);

int    libkern_strcasecmp(const char *s1, const char *s2);
int    libkern_strncasecmp(const char *s1, const char *s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_STRING_H */
