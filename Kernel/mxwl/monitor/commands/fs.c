#include "../monitor.h"
#include "fs.h"
#include "../../kal/kal.h"

static char _cwd[MX_MONITOR_MAX_LINE] = "/";

static kern_return_t _ls_handler(int argc, char **argv,
                                  char *out, size_t out_size)
{
    const char *path = (argc > 1) ? argv[1] : _cwd;

    size_t pos = libkern_snprintf(out, out_size,
                                  "Contents of %s:\n", path);

    kal_file_t *f = kal_fopen(path, "r");
    if (!f) {
        pos += libkern_snprintf(out + pos, out_size - pos,
                                "  (cannot read)\n");
        return KERN_FAILURE;
    }

    pos += libkern_snprintf(out + pos, out_size - pos,
                            "  (file listing not available without dir support)\n");
    kal_fclose(f);

    return KERN_SUCCESS;
}

static kern_return_t _cd_handler(int argc, char **argv,
                                  char *out, size_t out_size)
{
    (void)out; (void)out_size;

    if (argc < 2) {
        libkern_strncpy(_cwd, "/", sizeof(_cwd));
        return KERN_SUCCESS;
    }

    libkern_strncpy(_cwd, argv[1], sizeof(_cwd));
    return KERN_SUCCESS;
}

static kern_return_t _pwd_handler(int argc, char **argv,
                                   char *out, size_t out_size)
{
    (void)argc; (void)argv;
    libkern_snprintf(out, out_size, "%s\n", _cwd);
    return KERN_SUCCESS;
}

static kern_return_t _cat_handler(int argc, char **argv,
                                   char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size, "Usage: cat <file>\n");
        return KERN_INVALID_ARGUMENT;
    }

    const char *path = argv[1];
    kal_file_t *f = kal_fopen(path, "r");
    if (!f) {
        libkern_snprintf(out, out_size, "cat: %s: No such file\n", path);
        return KERN_FAILURE;
    }

    size_t pos = 0;
    char buf[128];
    size_t n;
    while ((n = kal_fread(buf, 1, sizeof(buf), f)) > 0 && pos < out_size - 1) {
        size_t remain = out_size - 1 - pos;
        size_t copy = (n < remain) ? n : remain;
        libkern_memcpy(out + pos, buf, copy);
        pos += copy;
    }
    out[pos] = '\0';

    kal_fclose(f);

    if (pos == 0)
        libkern_snprintf(out, out_size, "(empty file)\n");

    return KERN_SUCCESS;
}

static kern_return_t _mount_handler(int argc, char **argv,
                                     char *out, size_t out_size)
{
    if (argc < 3) {
        libkern_snprintf(out, out_size,
                         "Usage: mount <device> <path>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size,
                     "Mount %s at %s (stub)\n", argv[1], argv[2]);
    return KERN_SUCCESS;
}

static kern_return_t _unmount_handler(int argc, char **argv,
                                       char *out, size_t out_size)
{
    if (argc < 2) {
        libkern_snprintf(out, out_size,
                         "Usage: unmount <path>\n");
        return KERN_INVALID_ARGUMENT;
    }

    libkern_snprintf(out, out_size,
                     "Unmount %s (stub)\n", argv[1]);
    return KERN_SUCCESS;
}

mx_cmd_t _ls_cmd = {
    .name       = "ls",
    .help_short = "List directory contents",
    .help_long  = "Usage: ls [path]\n"
                  "List files and directories at the given path.",
    .handler    = _ls_handler,
};

mx_cmd_t _cd_cmd = {
    .name       = "cd",
    .help_short = "Change current directory",
    .help_long  = "Usage: cd [path]\n"
                  "Change the current working directory.",
    .handler    = _cd_handler,
};

mx_cmd_t _pwd_cmd = {
    .name       = "pwd",
    .help_short = "Print working directory",
    .help_long  = "Usage: pwd\n"
                  "Show the current working directory.",
    .handler    = _pwd_handler,
};

mx_cmd_t _cat_cmd = {
    .name       = "cat",
    .help_short = "Display file contents",
    .help_long  = "Usage: cat <file>\n"
                  "Print the contents of a file to the console.",
    .handler    = _cat_handler,
};

mx_cmd_t _mount_cmd = {
    .name       = "mount",
    .help_short = "Mount a filesystem",
    .help_long  = "Usage: mount <device> <path>\n"
                  "Mount a filesystem from a device at the given path.",
    .handler    = _mount_handler,
};

mx_cmd_t _unmount_cmd = {
    .name       = "unmount",
    .help_short = "Unmount a filesystem",
    .help_long  = "Usage: unmount <path>\n"
                  "Unmount a previously mounted filesystem.",
    .handler    = _unmount_handler,
};
