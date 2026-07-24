/*
 * mango/loader/mach_loader.c
 *
 * .mach binary loader for the Mango nanokernel.
 */

#include "mach_loader.h"
#include "../mach/klog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#define MACH_LOADER_TMPDIR "/tmp/mango-loader"

kern_return_t mango_loader_init(void)
{
    mkdir(MACH_LOADER_TMPDIR, 0755);
    klog_info("binary loader ready (tmpdir: %s)\n", MACH_LOADER_TMPDIR);
    return KERN_SUCCESS;
}

kern_return_t mango_loader_validate(const char *mach_path,
                                    mach_binary_header_t *out_header)
{
    if (!mach_path || !out_header) return KERN_INVALID_ARGUMENT;

    FILE *f = fopen(mach_path, "rb");
    if (!f) {
        klog_err("cannot open: %s\n", mach_path);
        return KERN_INVALID_OBJECT;
    }

    mach_binary_header_t hdr;
    size_t nread = fread(&hdr, sizeof(hdr), 1, f);
    fclose(f);

    if (nread != 1) {
        klog_err("truncated header: %s\n", mach_path);
        return KERN_INVALID_OBJECT;
    }

    if (hdr.magic != MACH_BINARY_MAGIC_BE &&
        hdr.magic != MACH_BINARY_MAGIC_LE) {
        klog_err("bad magic in %s: 0x%08x (expected 0x%08x)\n",
                 mach_path, hdr.magic, MACH_BINARY_MAGIC_BE);
        return KERN_INVALID_OBJECT;
    }

    if (hdr.version != MACH_BINARY_VERSION_1) {
        klog_err("unsupported format version: %u\n", hdr.version);
        return KERN_INVALID_ARGUMENT;
    }

    if (out_header) *out_header = hdr;
    return KERN_SUCCESS;
}

kern_return_t mango_loader_extract_binary(const char *mach_path,
                                          const mach_binary_header_t *header,
                                          char *out_tmp_path,
                                          size_t tmp_path_size)
{
    FILE *src = fopen(mach_path, "rb");
    if (!src) return KERN_FAILURE;

    char template[256];
    snprintf(template, sizeof(template), "%s/mach.XXXXXX", MACH_LOADER_TMPDIR);

    int tmpfd = mkstemp(template);
    if (tmpfd < 0) {
        fclose(src);
        return KERN_FAILURE;
    }

    if (fseek(src, header->binary_offset, SEEK_SET) != 0) {
        close(tmpfd);
        fclose(src);
        unlink(template);
        return KERN_FAILURE;
    }

    char buf[4096];
    uint32_t remaining = header->binary_size;

    while (remaining > 0) {
        size_t to_read = remaining > sizeof(buf) ? sizeof(buf) : remaining;
        size_t nread = fread(buf, 1, to_read, src);
        if (nread == 0) break;
        ssize_t nwritten = write(tmpfd, buf, nread);
        if (nwritten < 0) break;
        remaining -= (uint32_t)nwritten;
    }

    fclose(src);
    close(tmpfd);

    if (remaining > 0) {
        klog_err("incomplete extraction: %u bytes remaining\n", remaining);
        unlink(template);
        return KERN_FAILURE;
    }

    chmod(template, 0755);
    strncpy(out_tmp_path, template, tmp_path_size - 1);
    out_tmp_path[tmp_path_size - 1] = '\0';

    return KERN_SUCCESS;
}

kern_return_t mango_loader_exec(mango_task_t *task,
                                const char *mach_path,
                                const mach_binary_header_t *header)
{
    if (!task || !mach_path || !header) return KERN_INVALID_ARGUMENT;

    char tmp_path[512];
    kern_return_t kr = mango_loader_extract_binary(mach_path, header,
                                                   tmp_path, sizeof(tmp_path));
    if (kr != KERN_SUCCESS) {
        klog_err("failed to extract binary from %s\n", mach_path);
        return kr;
    }

    klog_info("extracted embedded binary to %s\n", tmp_path);

    if (header->metadata_offset > 0 && header->metadata_size > 0) {
        FILE *f = fopen(mach_path, "rb");
        if (f) {
            mach_metadata_t meta;
            fseek(f, header->metadata_offset, SEEK_SET);
            size_t n = fread(&meta, 1, sizeof(meta), f);
            fclose(f);
            if (n >= sizeof(uint32_t)) {
                klog_info("app: %s v%s\n", meta.app_name, meta.version);
            }
        }
    }

    strncpy(task->binary_path, mach_path, sizeof(task->binary_path) - 1);

    pid_t pid = fork();
    if (pid < 0) {
        klog_err("fork failed: %s\n", strerror(errno));
        unlink(tmp_path);
        return KERN_FAILURE;
    }

    if (pid == 0) {
        char port_str[32];
        snprintf(port_str, sizeof(port_str), "%d", task->bootstrap_port);
        setenv("MANGO_BOOTSTRAP_PORT", port_str, 1);

        char id_str[32];
        snprintf(id_str, sizeof(id_str), "%d", task->id);
        setenv("MANGO_TASK_ID", id_str, 1);

        if (task->userfs_root[0]) {
            setenv("USERFSROOT", task->userfs_root, 1);
        }

        execl(tmp_path, tmp_path, (char *)NULL);
        klog_err("exec failed: %s\n", strerror(errno));
        _exit(1);
    }

    task->host_pid = pid;
    task->running = TRUE;

    klog_info("loaded: %s (pid %d, task id %d)\n",
              mach_path, pid, task->id);

    return KERN_SUCCESS;
}

kern_return_t mango_loader_load(const char *mach_path,
                                mango_task_t **out_task)
{
    if (!mach_path) return KERN_INVALID_ARGUMENT;

    mach_binary_header_t header;
    kern_return_t kr = mango_loader_validate(mach_path, &header);
    if (kr != KERN_SUCCESS) return kr;

    mango_task_t *task = NULL;
    kr = mango_task_create(&task, mach_path);
    if (kr != KERN_SUCCESS) return kr;

    kr = mango_loader_exec(task, mach_path, &header);
    if (kr != KERN_SUCCESS) {
        mango_task_terminate(task);
        return kr;
    }

    if (out_task) *out_task = task;
    return KERN_SUCCESS;
}

kern_return_t mango_loader_cleanup(mango_task_t *task)
{
    if (!task) return KERN_INVALID_TASK;
    if (task->host_pid > 0) {
        kill(task->host_pid, SIGTERM);
        waitpid(task->host_pid, NULL, WNOHANG);
    }
    return KERN_SUCCESS;
}
