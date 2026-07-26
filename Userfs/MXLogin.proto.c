#include <ncurses.h>
#include <string.h>

#define WIDTH 45
#define HEIGHT 11

void draw_openstep_window(WINDOW *win) {
    int w, h;
    getmaxyx(win, h, w);

    // Draw the main dark gray border and light gray background
    wbkgd(win, COLOR_PAIR(1));
    box(win, 0, 0);

    // Draw the classic NeXTSTEP/OPENSTEP top title bar
    wattron(win, COLOR_PAIR(2) | A_BOLD);
    for (int i = 1; i < w - 1; i++) {
        mvwaddch(win, 1, i, ' ');
    }
    mvwprintw(win, 1, (w - 12) / 2, "MINSTEP");
    wattroff(win, COLOR_PAIR(2) | A_BOLD);

    // Draw fields and labels
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 4, 4, " Username: [                      ]");
    mvwprintw(win, 6, 4, " Password: [                      ]");
    
    // Draw bottom helper hint
    mvwprintw(win, h - 2, (w - 26) / 2, "Press ENTER to authenticate");
    wattroff(win, COLOR_PAIR(1));

    wrefresh(win);
}

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Setup NeXTSTEP grayscale-inspired color scheme
    start_color();
    // Pair 1: Light gray background, dark text (Window body)
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    // Pair 2: Dark charcoal background, white text (Title bar)
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

    // Refresh background screen
    refresh();

    // Center the login dialog on the screen
    int start_y = (LINES - HEIGHT) / 2;
    int start_x = (COLS - WIDTH) / 2;
    WINDOW *login_win = newwin(HEIGHT, WIDTH, start_y, start_x);

    draw_openstep_window(login_win);

    char username[21] = {0};
    char password[21] = {0};
    int ch;
    
    // Input Handling: Username Field
    int u_pos = 0;
    curs_set(1); // Show cursor
    wmove(login_win, 4, 16);
    wrefresh(login_win);

    while (u_pos < 20) {
        ch = wgetch(login_win);
        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && u_pos > 0) {
            u_pos--;
            username[u_pos] = '\0';
            mvwaddch(login_win, 4, 16 + u_pos, ' ');
            wmove(login_win, 4, 16 + u_pos);
        } else if (ch >= 32 && ch <= 126) {
            username[u_pos] = ch;
            mvwaddch(login_win, 4, 16 + u_pos, ch);
            u_pos++;
        }
        wrefresh(login_win);
    }

    // Input Handling: Password Field (Masked with *)
    int p_pos = 0;
    wmove(login_win, 6, 16);
    wrefresh(login_win);

    while (p_pos < 20) {
        ch = wgetch(login_win);
        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && p_pos > 0) {
            p_pos--;
            password[p_pos] = '\0';
            mvwaddch(login_win, 6, 16 + p_pos, ' ');
            wmove(login_win, 6, 16 + p_pos);
        } else if (ch >= 32 && ch <= 126) {
            password[p_pos] = ch;
            mvwaddch(login_win, 6, 16 + p_pos, '*');
            p_pos++;
        }
        wrefresh(login_win);
    }

    // Clear and closing sequence
    curs_set(0);
    wattron(login_win, COLOR_PAIR(2));
    mvwprintw(login_win, HEIGHT - 2, 1, " Authenticating... Please wait.             ");
    wattroff(login_win, COLOR_PAIR(2));
    wrefresh(login_win);
    
    getch(); // Wait for one last keypress before exiting
    endwin();
    return 0;
}
