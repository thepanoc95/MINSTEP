/*
 * Developer/BSD/bsd_memory.c
 *
 * Memory management for the BSD server.
 * Implements brk/sbrk and mmap/munmap by delegating to the
 * host OS memory management facilities.
 */

#include "bsd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

/* -----------------------------------------------------------------------
 *  Memory initialization
 * ----------------------------------------------------------------------- */

kern_return_t bsd_memory_init(bsd_process_t *proc)
{
    if (!proc) return KERN_INVALID_TASK;

    proc->brk_addr = sbrk(0);
    proc->mmap_base = NULL;
    proc->mmap_size = 0;

    return KERN_SUCCESS;
}

/* -----------------------------------------------------------------------
 *  brk
 * ----------------------------------------------------------------------- */

void *bsd_memory_brk(bsd_process_t *proc, void *addr)
{
    if (!proc) return (void *)-1;

    if (addr == NULL) {
        return proc->brk_addr;
    }

    void *current = sbrk(0);
    long diff = (char *)addr - (char *)current;

    if (diff > 0) {
        void *result = sbrk(diff);
        if (result == (void *)-1) return (void *)-1;
    }

    proc->brk_addr = addr;
    return proc->brk_addr;
}

/* -----------------------------------------------------------------------
 *  sbrk
 * ----------------------------------------------------------------------- */

void *bsd_memory_sbrk(bsd_process_t *proc, int incr)
{
    if (!proc) return (void *)-1;

    void *old = sbrk(0);
    void *result = sbrk(incr);

    if (result == (void *)-1) return (void *)-1;

    proc->brk_addr = sbrk(0);
    return old;
}

/* -----------------------------------------------------------------------
 *  mmap
 * ----------------------------------------------------------------------- */

void *bsd_memory_mmap(bsd_process_t *proc, void *addr, size_t len,
                      int prot, int flags, int fd, off_t offset)
{
    if (!proc) return MAP_FAILED;

    int host_fd = -1;
    if (fd >= 0 && fd < BSD_MAX_FD && proc->fd_table[fd].in_use) {
        host_fd = proc->fd_table[fd].host_fd;
    } else {
        host_fd = -1;
    }

    void *result = mmap(addr, len, prot, flags, host_fd, offset);
    if (result == MAP_FAILED) return MAP_FAILED;

    if (proc->mmap_base == NULL) {
        proc->mmap_base = result;
        proc->mmap_size = len;
    } else {
        size_t total = (char *)result + len - (char *)proc->mmap_base;
        if (total > proc->mmap_size) {
            proc->mmap_size = total;
        }
    }

    return result;
}

/* -----------------------------------------------------------------------
 *  munmap
 * ----------------------------------------------------------------------- */

int bsd_memory_munmap(bsd_process_t *proc, void *addr, size_t len)
{
    if (!proc) return -1;

    return munmap(addr, len);
}