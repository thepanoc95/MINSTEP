#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <sys/utsname.h>

const char *boot1ver = "v1.0.0-rc1";

/* Forward declaration -- implemented in mach.c */
void jump2mach(const char *kernel_path);

#define BOOT1_VERSION boot1ver
#define MAX_INPUT 1024
#define PROMPT "boot: "

static const char *opt_kernel_path = NULL;

struct termios orig_termios;

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void setup_raw_terminal(void) {
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &orig_termios) < 0) return;
    atexit(restore_terminal);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void MainPrompt(void)
{
    char buffer[MAX_INPUT] = {0};
    int buf_idx = 0;
    struct pollfd pfd;

    setup_raw_terminal();

    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;

    fprintf(stderr, "%s", PROMPT);
    fflush(stderr);

    while (1) {
        int ret = poll(&pfd, 1, 1000);

        if (ret < 0) break;

        if (ret == 0) continue;

        if (pfd.revents & POLLIN) {
            char c;
            if (read(STDIN_FILENO, &c, 1) <= 0) break;

            if (c == '\n' || c == '\r') {
                fprintf(stderr, "\n");
                if (strcmp(buffer, "-v") == 0) {
                    // jump2mach(verbose_);
                    break;
                } else if (strcmp(buffer, "?") == 0) {
                    printf("\nAdvanced Options:\n  -v\tDiagnostic boot\n  -s\tSingle-user mode\n  -kernel <file>\tLoad specified kernel binary\n\n");
                    memset(buffer, 0, sizeof(buffer));
                    buf_idx = 0;
                    fprintf(stderr, "%s", PROMPT);
                    fflush(stderr);
                } else if (strncmp(buffer, "-kernel ", 8) == 0) {
                    const char *path = buffer + 8;
                    while (*path == ' ') path++;
                    if (strlen(path) > 0) {
                        opt_kernel_path = path;
                        jump2mach(opt_kernel_path);
                    } else {
                        fprintf(stderr, "Usage: -kernel <filename>\n");
                    }
                    break;
                } else {
                    if (strlen(buffer) > 0) {
                        perror("Not Implemented!\n");
                    } else {
                        jump2mach(opt_kernel_path);
                    }
                    break;
                }
            }
            else if (c == 127 || c == 8) {
                if (buf_idx > 0) {
                    buf_idx--;
                    buffer[buf_idx] = '\0';
                    fprintf(stderr, "\b \b");
                    fflush(stderr);
                }
            }
            else if (c >= 32 && c <= 126 && buf_idx < (MAX_INPUT - 1)) {
                buffer[buf_idx++] = c;
                buffer[buf_idx] = '\0';
                fputc(c, stderr);
                fflush(stderr);
            }
        }
    }

    restore_terminal();
}

void TotalMemCount(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("Failed to get page size!");
        return;
    }

    long total_pages = sysconf(_SC_PHYS_PAGES);
    long avail_pages = sysconf(_SC_AVPHYS_PAGES);
    long long total_memory = (long long)total_pages * page_size;
    long long avail_memory = (long long)avail_pages * page_size;

    printf("%lldk conventional / %lldk total memory\n", avail_memory / 1024, total_memory / 1024);
}

void size_memory(void)
{
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("obj.size_memory() failed!\n");
        return;
    }

    long total_pages = sysconf(_SC_PHYS_PAGES);
    long long total_memory = (long long)total_pages * page_size;

    printf("Sizing memory... %lldk\n", total_memory / 1024);
}

void getMachineType(void)
{
    struct utsname sys_info;
    if (uname(&sys_info) == -1) {
        perror("[boot1] ERROR: could not identify the machine type!\nMinSTEP might get unstable!\n");
        return;
    }

    printf("[boot1] INFO: sys.machine is %s\n", sys_info.machine);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-kernel") == 0 && i + 1 < argc) {
            opt_kernel_path = argv[++i];
        }
    }

    printf("MINSTEP boot1 version %s \n", BOOT1_VERSION);
    printf(".......\n");
    size_memory();
    TotalMemCount();
    printf("\n");

    MainPrompt();
    return 0;
}
