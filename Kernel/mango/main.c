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
    (void)argc;
    (void)argv;

    /* Check for --help */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            fprintf(stderr, "Usage: vmmango [--help] [--verbose] [--single-user]\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "MINSTEP Operating System\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "  --help          Show this help\n");
            fprintf(stderr, "  -v              Verbose boot\n");
            fprintf(stderr, "  -s              Single-user mode (no init)\n");
            fprintf(stderr, "\n");
            fprintf(stderr, "Environment:\n");
            fprintf(stderr, "  USERFSROOT      User filesystem root (default: /)\n");
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            setenv("MANGO_BOOT_VERBOSE", "1", 1);
        } else if (strcmp(argv[i], "-s") == 0) {
            setenv("MANGO_BOOT_SINGLE_USER", "1", 1);
        }
    }

    mango_kernel_main();
    return 0;
}
