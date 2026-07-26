/*
 * mango/main.c
 *
 * Entry point for the mango-kernel binary.
 * When executed standalone (not via boot1), this provides main().
 */

#include "mach/mach_kernel.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char init_path[1024];
    init_path[0] = '\0';

    /* Check for --help */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            fprintf(stderr, "Usage: mach [options]\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "MINSTEP Operating System\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "  --help              Show this help\n");
            fprintf(stderr, "  -v                  Verbose boot\n");
            fprintf(stderr, "  -s                  Single-user mode (no init)\n");
            fprintf(stderr, "  -init=<path>        Run <path> as init process\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Environment:\n");
            fprintf(stderr, "  USERFSROOT          User filesystem root (default: /)\n");
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            setenv("MANGO_BOOT_VERBOSE", "1", 1);
        } else if (strcmp(argv[i], "-s") == 0) {
            setenv("MANGO_BOOT_SINGLE_USER", "1", 1);
        } else if (strncmp(argv[i], "-init=", 6) == 0) {
            strncpy(init_path, argv[i] + 6, sizeof(init_path) - 1);
            init_path[sizeof(init_path) - 1] = '\0';
        }
    }

    mango_kernel_main(init_path[0] ? init_path : NULL);
    return 0;
}
