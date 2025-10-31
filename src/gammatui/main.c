#define _POSIX_C_SOURCE 200809L
#include "gammatui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

void apply_values(double gamma, double bright) {
    if (!display_output[0]) return;
    char val[128];
    snprintf(val, sizeof(val), "%.3f:%.3f:%.3f", gamma, gamma, gamma);
    xr_call_async(display_output, "--gamma", val);
    snprintf(val, sizeof(val), "%.3f", bright);
    xr_call_async(display_output, "--brightness", val);
}

void revert_values() {
    if (!display_output[0]) return;
    xr_call_async(display_output, "--gamma", "1:1:1");
    xr_call_async(display_output, "--brightness", "1");
}

int main(int argc, char **argv) {
    const char *forced_output = NULL;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i],"--help")==0) { 
            printf("Usage: %s [--output NAME]\n", argv[0]); 
            return 0; 
        }
        if (strcmp(argv[i],"--output")==0 && i+1<argc) { 
            forced_output = argv[++i]; 
            continue; 
        }
        fprintf(stderr, "Unknown arg: %s\n", argv[i]);
        return 1;
    }

    if (system("which xrandr >/dev/null 2>&1") != 0) {
        fprintf(stderr, "WARNING: xrandr not found. The UI will still run but changes won't apply.\n");
    }

    if (forced_output) {
        strncpy(display_output, forced_output, sizeof(display_output)-1);
        display_output[sizeof(display_output)-1] = '\0';
    }
    else if (!detect_output(display_output, sizeof(display_output))) {
        display_output[0] = '\0';
    }

    double gamma = 1.0, bright = 1.0;
    int selected = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) init_colors_safe();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    WINDOW *win = newwin(rows - 2, cols - 2, 1, 1);
    nodelay(stdscr, FALSE);

    draw_ui(win, gamma, bright, selected, rows - 2, cols - 2);

    int ch;
    int needs_redraw = 0;

    while (1) {
        ch = getch();
        
        if (ch == 'q' || ch == 'Q') break;

        if (ch == KEY_UP || ch == KEY_DOWN) {
            selected = 1 - selected; 
            needs_redraw = 1;
        } else if (ch == KEY_LEFT) {
            if (selected == 0) gamma = clamp_double(gamma - 0.02, 0.5, 3.0);
            else bright = clamp_double(bright - 0.01, 0.1, 2.0);
            
            if (debounce_allow()) {
                apply_values(gamma, bright);
                needs_redraw = 1;
            }
        } else if (ch == KEY_RIGHT) {
            if (selected == 0) gamma = clamp_double(gamma + 0.02, 0.5, 3.0);
            else bright = clamp_double(bright + 0.01, 0.1, 2.0);
            
            if (debounce_allow()) {
                apply_values(gamma, bright);
                needs_redraw = 1;
            }
        } else if (ch == 'r' || ch == 'R') { 
            revert_values(); 
            gamma = 1.0; 
            bright = 1.0; 
            needs_redraw = 1; 
        }

        int new_rows, new_cols;
        getmaxyx(stdscr, new_rows, new_cols);
        
        if (new_rows != rows || new_cols != cols) {
            rows = new_rows;
            cols = new_cols;
            wresize(win, rows - 2, cols - 2);
            needs_redraw = 1;
        }

        if (needs_redraw) {
            draw_ui(win, gamma, bright, selected, rows - 2, cols - 2);
            needs_redraw = 0;
        }
    }

    delwin(win);
    endwin();
    return 0;
}