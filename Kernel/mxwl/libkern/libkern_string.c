#include "libkern_string.h"

#include <string.h>

void libkern_memcpy(void *dst, const void *src, size_t n)
{
    memcpy(dst, src, n);
}

void libkern_memmove(void *dst, const void *src, size_t n)
{
    memmove(dst, src, n);
}

void *libkern_memset(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

int libkern_memcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

size_t libkern_strlen(const char *s)
{
    return strlen(s);
}

size_t libkern_strnlen(const char *s, size_t maxlen)
{
    size_t n = 0;
    while (n < maxlen && s[n])
        n++;
    return n;
}

char *libkern_strcpy(char *dst, const char *src)
{
    return strcpy(dst, src);
}

char *libkern_strncpy(char *dst, const char *src, size_t n)
{
    return strncpy(dst, src, n);
}

int libkern_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

int libkern_strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n);
}

char *libkern_strcat(char *dst, const char *src)
{
    return strcat(dst, src);
}

char *libkern_strncat(char *dst, const char *src, size_t n)
{
    return strncat(dst, src, n);
}

char *libkern_strchr(const char *s, int c)
{
    return strchr(s, c);
}

char *libkern_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

char *libkern_strstr(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

char *libkern_strdup(const char *s)
{
    return strdup(s);
}

size_t libkern_strlcpy(char *dst, const char *src, size_t n)
{
    size_t slen = libkern_strlen(src);
    if (n > 0) {
        size_t copy = (slen >= n) ? n - 1 : slen;
        memcpy(dst, src, copy);
        dst[copy] = '\0';
    }
    return slen;
}

size_t libkern_strlcat(char *dst, const char *src, size_t n)
{
    size_t dlen = libkern_strnlen(dst, n);
    if (dlen == n)
        return n + libkern_strlen(src);
    return dlen + libkern_strlcpy(dst + dlen, src, n - dlen);
}

int libkern_strcasecmp(const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        int c1 = *s1;
        int c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int libkern_strncasecmp(const char *s1, const char *s2, size_t n)
{
    while (n > 0 && *s1 && *s2) {
        int c1 = *s1;
        int c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return c1 - c2;
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}
