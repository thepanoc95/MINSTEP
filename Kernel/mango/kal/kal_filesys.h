/*
 * mango/kal/kal_filesys.h
 *
 * Kernel Abstraction Layer -- filesystem operations.
 *
 * Abstracts file I/O, temporary files, directory creation,
 * and permissions so the binary loader is portable.
 */

#ifndef MANGO_KAL_FILESYS_H
#define MANGO_KAL_FILESYS_H

#include "kal_platform.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  File handle
 * ----------------------------------------------------------------------- */

typedef struct kal_file {
    void *_impl;
} kal_file_t;

/* -----------------------------------------------------------------------
 *  File open / close
 * ----------------------------------------------------------------------- */

/* Open a file.  mode follows fopen conventions ("rb", "wb", etc.). */
kal_file_t *kal_fopen(const char *path, const char *mode);

/* Close a file. */
int kal_fclose(kal_file_t *f);

/* -----------------------------------------------------------------------
 *  File read / write
 * ----------------------------------------------------------------------- */

/* Read count elements of size bytes each.  Returns number of elements read. */
size_t kal_fread(void *buf, size_t size, size_t count, kal_file_t *f);

/* Write count elements of size bytes each.  Returns number of elements written. */
size_t kal_fwrite(const void *buf, size_t size, size_t count, kal_file_t *f);

/* -----------------------------------------------------------------------
 *  File seeking
 * ----------------------------------------------------------------------- */

#define KAL_SEEK_SET    0
#define KAL_SEEK_CUR    1
#define KAL_SEEK_END    2

int kal_fseek(kal_file_t *f, long offset, int whence);

/* -----------------------------------------------------------------------
 *  Low-level file I/O
 * ----------------------------------------------------------------------- */

/* Open a file descriptor.  flags use host conventions. */
int kal_open(const char *path, int flags, int mode);

/* Write bytes to a file descriptor.  Returns bytes written. */
ssize_t kal_fd_write(int fd, const void *buf, size_t count);

/* Close a file descriptor. */
int kal_fd_close(int fd);

/* -----------------------------------------------------------------------
 *  Temporary files
 * ----------------------------------------------------------------------- */

/* Create a temporary file.  template is modified in place (mkstemp-style). */
int kal_mkstemp(char *template);

/* -----------------------------------------------------------------------
 *  Directory operations
 * ----------------------------------------------------------------------- */

/* Create a directory.  Returns 0 on success. */
int kal_mkdir(const char *path, int mode);

/* -----------------------------------------------------------------------
 *  File metadata
 * ----------------------------------------------------------------------- */

/* Change file permissions.  Returns 0 on success. */
int kal_chmod(const char *path, int mode);

/* Remove (unlink) a file.  Returns 0 on success. */
int kal_unlink(const char *path);

/* Check file existence / accessibility.  Returns 0 on success. */
int kal_access(const char *path, int mode);

#define KAL_OK          0
#define KAL_X_OK        1
#define KAL_F_OK        0

#ifdef __cplusplus
}
#endif

#endif /* MANGO_KAL_FILESYS_H */
