/*
 * mango/kal/posix/kal_posix_ipc.c
 *
 * POSIX backend for KAL IPC / socket primitives.
 */

#include "../kal_ipc.h"

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

int kal_socket_pair(kal_fd_t *out_fd0, kal_fd_t *out_fd1)
{
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0)
        return -1;
    *out_fd0 = (kal_fd_t)fds[0];
    *out_fd1 = (kal_fd_t)fds[1];
    return 0;
}

int kal_close(kal_fd_t fd)
{
    return close((int)fd);
}

ssize_t kal_write(kal_fd_t fd, const void *buf, size_t count)
{
    return write((int)fd, buf, count);
}

ssize_t kal_read(kal_fd_t fd, void *buf, size_t count)
{
    return read((int)fd, buf, count);
}

int kal_poll(kal_pollfd_t *fds, unsigned int nfds, int timeout_ms)
{
    struct pollfd pfds[64];
    unsigned int n = nfds;
    if (n > 64) n = 64;

    for (unsigned int i = 0; i < n; i++) {
        pfds[i].fd = (int)fds[i].fd;
        pfds[i].events = (short)fds[i].events;
        pfds[i].revents = 0;
    }

    int ret = poll(pfds, n, timeout_ms);

    for (unsigned int i = 0; i < n; i++) {
        fds[i].revents = (uint32_t)pfds[i].revents;
    }

    return ret;
}
