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

void jump2mach(void)
{
    /* Try to exec the mango-kernel binary */
    for (const char **p = mango_kernel_paths; *p; p++) {
        if (access(*p, X_OK) == 0) {
            fprintf(stderr, "Loading Mango kernel: %s\n", *p);
            execvp(*p, (char *[]){ (char *)*p, NULL });
            /* exec only returns on error */
            perror("exec failed");
        }
    }

    /* If we get here, no kernel binary was found */
    fprintf(stderr, "ERROR: mango-kernel not found.\n");
    fprintf(stderr, "Ensure mango-kernel is in PATH or in the current directory.\n");
}

#pragma mark @KERNEL_ENTRY :: END
