/*
 * mxwl/kal/posix/kal_posix_filesys.c
 *
 * POSIX backend for KAL filesystem operations.
 */

#include "../kal_filesys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

/* -----------------------------------------------------------------------
 *  File handle wraps FILE*
 * ----------------------------------------------------------------------- */

kal_file_t *kal_fopen(const char *path, const char *mode)
{
    FILE *f = fopen(path, mode);
    if (!f) return NULL;

    kal_file_t *kf = (kal_file_t *)malloc(sizeof(kal_file_t));
    if (!kf) {
        fclose(f);
        return NULL;
    }
    kf->_impl = f;
    return kf;
}

int kal_fclose(kal_file_t *f)
{
    if (!f || !f->_impl) return -1;
    int ret = fclose((FILE *)f->_impl);
    free(f);
    return ret;
}

size_t kal_fread(void *buf, size_t size, size_t count, kal_file_t *f)
{
    if (!f || !f->_impl) return 0;
    return fread(buf, size, count, (FILE *)f->_impl);
}

size_t kal_fwrite(const void *buf, size_t size, size_t count, kal_file_t *f)
{
    if (!f || !f->_impl) return 0;
    return fwrite(buf, size, count, (FILE *)f->_impl);
}

int kal_fseek(kal_file_t *f, long offset, int whence)
{
    if (!f || !f->_impl) return -1;

    int native_whence;
    switch (whence) {
    case KAL_SEEK_SET: native_whence = SEEK_SET; break;
    case KAL_SEEK_CUR: native_whence = SEEK_CUR; break;
    case KAL_SEEK_END: native_whence = SEEK_END; break;
    default: return -1;
    }

    return fseek((FILE *)f->_impl, offset, native_whence);
}

/* -----------------------------------------------------------------------
 *  Low-level I/O
 * ----------------------------------------------------------------------- */

int kal_open(const char *path, int flags, int mode)
{
    return open(path, flags, mode);
}

ssize_t kal_fd_write(int fd, const void *buf, size_t count)
{
    return write(fd, buf, count);
}

int kal_fd_close(int fd)
{
    return close(fd);
}

/* -----------------------------------------------------------------------
 *  Temporary files
 * ----------------------------------------------------------------------- */

int kal_mkstemp(char *template)
{
    return mkstemp(template);
}

/* -----------------------------------------------------------------------
 *  Directory
 * ----------------------------------------------------------------------- */

int kal_mkdir(const char *path, int mode)
{
    return mkdir(path, (mode_t)mode);
}

/* -----------------------------------------------------------------------
 *  File metadata
 * ----------------------------------------------------------------------- */

int kal_chmod(const char *path, int mode)
{
    return chmod(path, (mode_t)mode);
}

int kal_unlink(const char *path)
{
    return unlink(path);
}

int kal_access(const char *path, int mode)
{
    return access(path, mode);
}
