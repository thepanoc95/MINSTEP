/*
 * mxwl/kal/kal_ipc.h
 *
 * Kernel Abstraction Layer -- IPC / socket primitives.
 *
 * Abstracts the socket pair and poll operations used to implement
 * Mach port message passing on top of the host OS.
 */

#ifndef MXWL_KAL_IPC_H
#define MXWL_KAL_IPC_H

#include "kal_platform.h"
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 *  Types
 * ----------------------------------------------------------------------- */

typedef int kal_fd_t;

#define KAL_FD_INVALID  (-1)

/* -----------------------------------------------------------------------
 *  Socket pairs
 * ----------------------------------------------------------------------- */

/* Create a connected pair of file descriptors.
 * Returns 0 on success, -1 on error. */
int kal_socket_pair(kal_fd_t *out_fd0, kal_fd_t *out_fd1);

/* Close a file descriptor. */
int kal_close(kal_fd_t fd);

/* -----------------------------------------------------------------------
 *  I/O
 * ----------------------------------------------------------------------- */

/* Write data to a file descriptor.  Returns bytes written, -1 on error. */
ssize_t kal_write(kal_fd_t fd, const void *buf, size_t count);

/* Read data from a file descriptor.  Returns bytes read, 0 on EOF, -1 on error. */
ssize_t kal_read(kal_fd_t fd, void *buf, size_t count);

/* -----------------------------------------------------------------------
 *  Polling
 * ----------------------------------------------------------------------- */

#define KAL_POLLIN     0x0001
#define KAL_POLLOUT    0x0004
#define KAL_POLLERR    0x0008
#define KAL_POLLHUP    0x0010

typedef struct kal_pollfd {
    kal_fd_t    fd;
    uint32_t    events;
    uint32_t    revents;
} kal_pollfd_t;

/* Poll file descriptors.  Returns number of ready fds, 0 on timeout, -1 on error. */
int kal_poll(kal_pollfd_t *fds, unsigned int nfds, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* MXWL_KAL_IPC_H */
