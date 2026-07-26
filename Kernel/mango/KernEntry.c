/*
 * mach.c
 *
 * Kernel entry point for boot1.
 * jump2mach() is called by boot1 when the user presses Enter
 * at the boot prompt.  It exec's the mango-kernel binary.
 */

#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Path to the Mango kernel binary (searched in PATH or relative) */
static const char *mango_kernel_paths[] = {
    "./mango-kernel",
    "/usr/local/bin/mango-kernel",
    "/usr/bin/mango-kernel",
    NULL
};

#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#pragma mark @KERNEL_ENTRY :: START

void jump2mach(const char *kernel_path)
{
    const char *path = kernel_path;

    if (!path) {
        /* No explicit path given, search defaults */
        for (const char **p = mango_kernel_paths; *p; p++) {
            if (access(*p, X_OK) == 0) {
                path = *p;
                break;
            }
        }
    }

    if (!path) {
        fprintf(stderr, "ERROR: mango-kernel not found.\n");
        fprintf(stderr, "Ensure mango-kernel is in PATH or in the current directory.\n");
        return;
    }

    if (access(path, X_OK) != 0) {
        fprintf(stderr, "ERROR: kernel '%s' is not executable or not found.\n", path);
        return;
    }

    fprintf(stderr, "Now loading kernel: %s\n", path);
    execvp(path, (char *[]){ (char *)path, NULL });
    perror("exec failed");
}

#pragma mark @KERNEL_ENTRY :: END
