#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MARGIN_X 4
#define HEADER_HEIGHT 2

// Array simulating the exact visual log text from your image
const char *mach_boot_lines[] = {
    "physical memory = 256.00 megabytes.",
    "using 255 buffers containing 1.99 megabytes of memory",
    "available memory = 248.87 megabytes. vm_page_free_count = 7c70",
    "PnP: Plug and Play support enabled",
    "ISA/EISA bus support enabled",
    "PCI Ver=2.10 BusCount=1 Features=[ BIOS16 CM1 ]",
    "Found PCI 2.0 device: ID=0x12378086 at Dev=0 Func=0 Bus=0",
    "Found PCI 2.0 device: ID=0x70008086 at Dev=1 Func=0 Bus=0",
    "Found PCI 2.0 device: ID=0x70108086 at Dev=1 Func=1 Bus=0",
    "Found PCI 2.0 device: ID=0xcafed0ee at Dev=2 Func=0 Bus=0",
    "Found PCI 2.1 device: ID=0x100e8086/0x001e8086 at Dev=3 Func=0 Bus=0",
    "Found PCI 2.0 device: ID=0xcafe80ee at Dev=4 Func=0 Bus=0",
    "Found PCI 2.0 device: ID=0x71138086 at Dev=7 Func=0 Bus=0",
    "PCI bus support enabled",
    "ISA bus",
    "DriverKit version 420",
    "hc0: device detected at port 0x1f0 irq 14",
    "hc0: Checking for ATA drive 0... Detected",
    "hc0: Checking for ATA drive 1...",
    "hc0: Checking for ATAPI drive 1...",
    "hc0: Resetting drives...",
    "Registering: hc0",
    "hd0: VBOX HARDDISK 1.0",
    "hd0: 4161 cylinders, 16 heads, 63 spt (disk geometry)",
    "hd0: using multisector (128) transfers.",
    "Registering: hd0",
    "hd0: Device Block Size: 512 bytes",
    "hd0: Device Capacity: 2047 MB",
    "hd0: Disk Label: Disk",
    "hc1: device detected at port 0x170 irq 15",
    "hc1: Checking for ATA drive 0...",
    "hc1: Checking for ATAPI drive 0... Detected",
    NULL
};

void run_mach_binary() {
    int pipefd[2];
    if (pipe(pipefd) == -1) return;

    pid_t pid = fork();
    if (pid == 0) {
        // Child process: redirect standard output to pipe and execute binary
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        execl("./mach", "./mach", NULL);
        // If execution fails, print error to pipe and exit
        perror("Execution of ./mach failed");
        exit(1);
    } else {
        // Parent process: read outputs from pipe
        close(pipefd[1]);
        FILE *stream = fdopen(pipefd[0], "r");
        char buffer[256];
        int current_y = HEADER_HEIGHT + 1;

        while (fgets(buffer, sizeof(buffer), stream) != NULL) {
            // Strip newline character
            buffer[strcspn(buffer, "\n")] = 0;
            wattron(stdscr, COLOR_PAIR(2));
            mvwprintw(stdscr, current_y++, MARGIN_X, "%s", buffer);
            wattroff(stdscr, COLOR_PAIR(2));
            
            // Re-draw blinking block cursor at the new position
            wattron(stdscr, COLOR_PAIR(1));
            mvwprintw(stdscr, current_y, MARGIN_X, " ");
            wattroff(stdscr, COLOR_PAIR(1));
            
            refresh();
            usleep(200000); // 200ms delay between outputs
        }
        fclose(stream);
        wait(NULL);
    }
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0); // Hide physical system cursor

    // Setup OPENSTEP/NeXT Grayscale palette
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK); // White text, Black bg (Header & Cursor)
    init_pair(2, COLOR_BLACK, COLOR_WHITE); // Black text, White bg (Console text)
    init_pair(3, COLOR_WHITE, COLOR_BLUE);  // Outer edge padding color boundary

    // Set background layout matching image dimensions
    wbkgd(stdscr, COLOR_PAIR(2));
    refresh();

    // Draw NeXT Mach window header strip
    wattron(stdscr, COLOR_PAIR(1));
    for (int i = 0; i < COLS; i++) {
        mvwaddch(stdscr, 1, i, ' ');
    }
    const char *header_text = "MINSTEP Mach Operating System";
    mvwprintw(stdscr, 1, (COLS - strlen(header_text)) / 2, "%s", header_text);
    wattroff(stdscr, COLOR_PAIR(1));
    refresh();

    // Print the replica boot log lines onto the white viewport canvas
    int line_index = 0;
    int y_pos = HEADER_HEIGHT + 1;
    wattron(stdscr, COLOR_PAIR(2));
    
    while (mach_boot_lines[line_index] != NULL) {
        mvwprintw(stdscr, y_pos++, MARGIN_X, "%s", mach_boot_lines[line_index]);
        line_index++;
        refresh();
        usleep(40000); // Quick sequential scrolling simulation
    }
    wattroff(stdscr, COLOR_PAIR(2));

    // Handle running external binary code and append logs
    run_mach_binary();

    // Idle loop simulating system runtime keeping window open
    while(1) {
        // Toggle simulated terminal block cursor flash states
        wattron(stdscr, COLOR_PAIR(1));
        mvwprintw(stdscr, y_pos, MARGIN_X, " ");
        wattroff(stdscr, COLOR_PAIR(1));
        refresh();
        usleep(400000);

        wattron(stdscr, COLOR_PAIR(2));
        mvwprintw(stdscr, y_pos, MARGIN_X, " ");
        wattroff(stdscr, COLOR_PAIR(2));
        refresh();
        usleep(400000);
    }

    endwin();
    return 0;
}
