#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MARGIN_X 4
#define HEADER_HEIGHT 2

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
            buffer[strcspn(buffer, "\n")] = 0;
            wattron(stdscr, COLOR_PAIR(2));
            mvwprintw(stdscr, current_y++, MARGIN_X, "%s", buffer);
            wattroff(stdscr, COLOR_PAIR(2));
            
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
