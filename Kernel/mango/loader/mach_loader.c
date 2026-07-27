/*
 * mango/loader/mach_loader.c
 *
 * .mach binary loader for the Mango nanokernel.
 */

#include "mach_loader.h"
#include "../mach/klog.h"
#include "../kal/kal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MACH_LOADER_TMPDIR "/tmp/mango-loader"

kern_return_t mango_loader_init(void)
{
    kal_mkdir(MACH_LOADER_TMPDIR, 0755);
    klog_sub_info("loader", "ready (tmpdir: %s)\n", MACH_LOADER_TMPDIR);
    return KERN_SUCCESS;
}

kern_return_t mango_loader_validate(const char *mach_path,
                                    mach_binary_header_t *out_header)
{
    if (!mach_path || !out_header) return KERN_INVALID_ARGUMENT;

    kal_file_t *f = kal_fopen(mach_path, "rb");
    if (!f) {
        klog_sub_err("loader", "cannot open: %s\n", mach_path);
        return KERN_INVALID_OBJECT;
    }

    mach_binary_header_t hdr;
    size_t nread = kal_fread(&hdr, sizeof(hdr), 1, f);
    kal_fclose(f);

    if (nread != 1) {
        klog_sub_err("loader", "truncated header: %s\n", mach_path);
        return KERN_INVALID_OBJECT;
    }

    if (hdr.magic != MACH_BINARY_MAGIC_BE &&
        hdr.magic != MACH_BINARY_MAGIC_LE) {
        klog_sub_err("loader", "bad magic in %s: 0x%08x (expected 0x%08x)\n",
                 mach_path, hdr.magic, MACH_BINARY_MAGIC_BE);
        return KERN_INVALID_OBJECT;
    }

    if (hdr.version != MACH_BINARY_VERSION_1) {
        klog_sub_err("loader", "unsupported format version: %u\n", hdr.version);
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
    kal_file_t *src = kal_fopen(mach_path, "rb");
    if (!src) return KERN_FAILURE;

    char template[256];
    snprintf(template, sizeof(template), "%s/mach.XXXXXX", MACH_LOADER_TMPDIR);

    int tmpfd = kal_mkstemp(template);
    if (tmpfd < 0) {
        kal_fclose(src);
        return KERN_FAILURE;
    }

    if (kal_fseek(src, header->binary_offset, KAL_SEEK_SET) != 0) {
        kal_fd_close(tmpfd);
        kal_fclose(src);
        kal_unlink(template);
        return KERN_FAILURE;
    }

    char buf[4096];
    uint32_t remaining = header->binary_size;

    while (remaining > 0) {
        size_t to_read = remaining > sizeof(buf) ? sizeof(buf) : remaining;
        size_t nread = kal_fread(buf, 1, to_read, src);
        if (nread == 0) break;
        ssize_t nwritten = kal_fd_write(tmpfd, buf, nread);
        if (nwritten < 0) break;
        remaining -= (uint32_t)nwritten;
    }

    kal_fclose(src);
    kal_fd_close(tmpfd);

    if (remaining > 0) {
        klog_sub_err("loader", "incomplete extraction: %u bytes remaining\n", remaining);
        kal_unlink(template);
        return KERN_FAILURE;
    }

    kal_chmod(template, 0755);
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
        klog_sub_err("loader", "failed to extract binary from %s\n", mach_path);
        return kr;
    }

    klog_sub_info("loader", "extracted embedded binary to %s\n", tmp_path);

    if (header->metadata_offset > 0 && header->metadata_size > 0) {
        kal_file_t *f = kal_fopen(mach_path, "rb");
        if (f) {
            mach_metadata_t meta;
            kal_fseek(f, header->metadata_offset, KAL_SEEK_SET);
            size_t n = kal_fread(&meta, 1, sizeof(meta), f);
            kal_fclose(f);
            if (n >= sizeof(uint32_t)) {
                klog_sub_info("loader", "app: %s v%s\n", meta.app_name, meta.version);
            }
        }
    }

    strncpy(task->binary_path, mach_path, sizeof(task->binary_path) - 1);

    /* Set environment for the child */
    char port_str[32];
    snprintf(port_str, sizeof(port_str), "%d", task->bootstrap_port);
    kal_env_set("MANGO_BOOTSTRAP_PORT", port_str, 1);

    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%d", task->id);
    kal_env_set("MANGO_TASK_ID", id_str, 1);

    if (task->userfs_root[0]) {
        kal_env_set("USERFSROOT", task->userfs_root, 1);
    }

    const char *argv[] = { tmp_path, NULL };
    kal_spawn_args_t spawn = {
        .exec_path = tmp_path,
        .argv = argv,
        .envp = NULL   /* inherit current environment */
    };

    kal_pid_t pid;
    if (kal_process_spawn(&spawn, &pid) < 0) {
        klog_sub_err("loader", "fork failed\n");
        kal_unlink(tmp_path);
        return KERN_FAILURE;
    }

    task->host_pid = pid;
    task->running = TRUE;

    klog_sub_info("loader", "loaded: %s (pid %d, task id %d)\n",
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
        kal_process_kill(task->host_pid, KAL_SIGTERM);
        kal_process_wait(task->host_pid, NULL);
    }
    return KERN_SUCCESS;
}
