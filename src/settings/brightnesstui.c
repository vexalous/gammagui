#define _POSIX_C_SOURCE 200809L
#include "settings.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

static const char *TITLE = "Gammatui - Brightness Settings";
static const char *UI_FIELDS[] = { "Output", "Gamma max", "Brightness max" };
enum { FI_OUTPUT = 0, FI_GAMMA_MAX = 1, FI_BRIGHT_MAX = 2, FI_COUNT = 3 };

static void show_message(WINDOW *w, const char *title, const char *msg) {
    werase(w);
    box(w, 0, 0);
    mvwprintw(w, 1, 2, "%s", title);
    mvwprintw(w, 3, 2, "%s", msg);
    mvwprintw(w, 5, 2, "Press any key to return");
    wnoutrefresh(w);
    doupdate();
    wgetch(w);
}

static void edit_string(WINDOW *w, int row, int col, char *buf, size_t bufsz) {
    char scratch[OUTPUT_LEN];
    if (bufsz > sizeof(scratch)) bufsz = sizeof(scratch);
    snprintf(scratch, sizeof(scratch), "%s", buf);
    int pos = (int)strlen(scratch);

    curs_set(1);
    keypad(w, TRUE);
    nodelay(w, FALSE);
    noecho();

    while (1) {
        mvwprintw(w, row, col, "%-*s", (int)bufsz-1, "");
        mvwprintw(w, row, col, "%s", scratch);
        wmove(w, row, col + pos);
        wrefresh(w);

        int ch = wgetch(w);
        if (ch == 27) {
            break;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            snprintf(buf, bufsz, "%s", scratch);
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (pos > 0) {
                pos--;
                scratch[pos] = '\0';
            }
        } else if (isprint(ch)) {
            if (pos < (int)bufsz - 1) {
                scratch[pos++] = (char)ch;
                scratch[pos] = '\0';
            }
        } else if (ch == KEY_LEFT) {
            if (pos > 0) pos--;
        } else if (ch == KEY_RIGHT) {
            if (pos < (int)strlen(scratch)) pos++;
        }
    }

    curs_set(0);
    keypad(w, TRUE);
    noecho();
}

static void edit_double(WINDOW *w, int row, int col, double *val) {
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%.3f", *val);
    char orig[64];
    strncpy(orig, tmp, sizeof(orig));
    edit_string(w, row, col, tmp, sizeof(tmp));
    if (strcmp(tmp, orig) != 0) {
        double v;
        if (sscanf(tmp, "%lf", &v) == 1) *val = v;
    }
}

static void draw_form(WINDOW *w, const struct cfg *c, int highlight) {
    werase(w);
    box(w, 0, 0);
    mvwprintw(w, 1, 2, "%s", TITLE);
    mvwprintw(w, 2, 2, "Use Up/Down to move, Enter to edit, S to save, Q to quit");
    int start = 5;
    int field_col = 20;
    for (int i = 0; i < FI_COUNT; ++i) {
        if (i == highlight) wattron(w, A_REVERSE);
        mvwprintw(w, start + i*2, 4, "%s:", UI_FIELDS[i]);
        wattroff(w, A_REVERSE);
        if (i == FI_OUTPUT) {
            mvwprintw(w, start + i*2, field_col, "%s", c->output[0] ? c->output : "(empty)");
        } else if (i == FI_GAMMA_MAX) {
            mvwprintw(w, start + i*2, field_col, "%.3f", c->gamma_max);
        } else if (i == FI_BRIGHT_MAX) {
            mvwprintw(w, start + i*2, field_col, "%.3f", c->bright_max);
        }
    }
    wnoutrefresh(w);
    doupdate();
}

int main(int argc, char **argv) {
    char cfgpath[PATH_MAX];
    if (!config_path_for_exe(cfgpath, sizeof(cfgpath), (argc>0)?argv[0]:NULL)) {
        fprintf(stderr, "Unable to determine config path for config.json\n");
        return 1;
    }

    struct cfg cur;
    if (!load_config(&cur, cfgpath)) {
        cur.output[0] = '\0';
        strncpy(cur.output, "eDP-1", OUTPUT_LEN-1);
        cur.output[OUTPUT_LEN-1] = '\0';
        cur.gamma_max = 3.0;
        cur.bright_max = 2.0;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    WINDOW *win = newwin(rows - 2, cols - 2, 1, 1);
    keypad(win, TRUE);

    int highlight = 0;
    draw_form(win, &cur, highlight);

    int ch;
    while (1) {
        ch = wgetch(win);
        if (ch == KEY_UP) {
            if (highlight > 0) highlight--;
        } else if (ch == KEY_DOWN) {
            if (highlight < FI_COUNT - 1) highlight++;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            int row = 5 + highlight*2;
            int col = 20;
            if (highlight == FI_OUTPUT) {
                edit_string(win, row, col, cur.output, sizeof(cur.output));
            } else if (highlight == FI_GAMMA_MAX) {
                edit_double(win, row, col, &cur.gamma_max);
            } else if (highlight == FI_BRIGHT_MAX) {
                edit_double(win, row, col, &cur.bright_max);
            }
        } else if (ch == 's' || ch == 'S') {
            if (save_config(&cur, cfgpath)) {
                show_message(win, "Saved", "Configuration saved successfully.");
            } else {
                show_message(win, "Error", "Failed to save configuration.");
            }
        } else if (ch == 'q' || ch == 'Q') {
            break;
        }
        getmaxyx(stdscr, rows, cols);
        wresize(win, rows - 2, cols - 2);
        draw_form(win, &cur, highlight);
    }

    delwin(win);
    endwin();
    return 0;
}
