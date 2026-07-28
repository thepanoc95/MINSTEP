/*
 * mxwl/kal/posix/kal_posix_terminal.c
 *
 * POSIX backend for KAL terminal I/O.
 */

#include "../kal_terminal.h"

#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
    struct termios saved;
    int            valid;
} kal_posix_terminal_t;

int kal_terminal_save(kal_terminal_t *t)
{
    kal_posix_terminal_t *impl = (kal_posix_terminal_t *)calloc(1, sizeof(*impl));
    if (!impl) return -1;

    if (tcgetattr(STDIN_FILENO, &impl->saved) == 0) {
        impl->valid = 1;
    }

    t->_impl = impl;
    return 0;
}

int kal_terminal_restore(kal_terminal_t *t)
{
    if (!t || !t->_impl) return -1;
    kal_posix_terminal_t *impl = (kal_posix_terminal_t *)t->_impl;

    if (impl->valid) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &impl->saved);
    }

    free(impl);
    t->_impl = NULL;
    return 0;
}

int kal_terminal_raw(kal_terminal_t *t)
{
    (void)t;
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &raw) != 0) return -1;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    return tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
