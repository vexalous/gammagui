#define _POSIX_C_SOURCE 200809L
#include "gammagui.h"
#include <string.h>

void init_colors_safe() {
    if (!has_colors()) return;
    start_color();
    use_default_colors();
    init_pair(1, COLOR_YELLOW, -1);
    init_pair(2, COLOR_CYAN, -1);
    init_pair(3, COLOR_WHITE, -1);
    init_pair(4, COLOR_GREEN, -1);
    init_pair(5, COLOR_MAGENTA, -1);
    init_pair(6, COLOR_RED, -1);
}

void draw_bar(WINDOW *w, int y, int x, int width, double value, double lo, double hi, int color_fill, int color_bg) {
    int inner = width - 2;
    mvwaddch(w, y, x, ACS_LTEE);
    for (int i = 0; i < inner; ++i) mvwaddch(w, y, x+1+i, ' ');
    mvwaddch(w, y, x+width-1, ACS_RTEE);

    double frac = (value - lo) / (hi - lo);
    if (frac < 0) { frac = 0; }
    if (frac > 1) { frac = 1; }
    int fill = (int)(frac * inner + 0.5);

    for (int i = 0; i < fill; ++i) {
        wattron(w, COLOR_PAIR(color_fill));
        mvwaddch(w, y, x+1+i, ' ' | A_REVERSE);
        wattroff(w, COLOR_PAIR(color_fill));
    }
    for (int i = fill; i < inner; ++i) {
        if (color_bg) wattron(w, COLOR_PAIR(color_bg));
        mvwaddch(w, y, x+1+i, ' ');
        if (color_bg) wattroff(w, COLOR_PAIR(color_bg));
    }
}

void draw_rounded_border(WINDOW *w) {
    int h, wdt; getmaxyx(w, h, wdt);
    mvwaddch(w, 0, 0, ACS_ULCORNER);
    mvwhline(w, 0, 1, ACS_HLINE, wdt-2);
    mvwaddch(w, 0, wdt-1, ACS_URCORNER);
    mvwaddch(w, h-1, 0, ACS_LLCORNER);
    mvwhline(w, h-1, 1, ACS_HLINE, wdt-2);
    mvwaddch(w, h-1, wdt-1, ACS_LRCORNER);
    mvwvline(w, 1, 0, ACS_VLINE, h-2);
    mvwvline(w, 1, wdt-1, ACS_VLINE, h-2);
}

void draw_ui(WINDOW *win, double gamma, double bright, int selected, int rows, int cols) {
    werase(win);
    draw_rounded_border(win);

    int left = 3, top = 1;
    wattron(win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(win, top, left, "x11gammacurses");
    wattroff(win, COLOR_PAIR(1) | A_BOLD);
    wattron(win, COLOR_PAIR(3));
    mvwprintw(win, top, left+16, "- output: %s", display_output[0]?display_output:"(not detected)");
    wattroff(win, COLOR_PAIR(3));

    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, top+2, left, "Controls:");
    wattroff(win, COLOR_PAIR(2));
    mvwprintw(win, top+2, left+11, "Up/Down select   Left/Right adjust   R=Revert   Q=Quit");

    int bar_w = cols - 12;
    int gy = top+6;
    if (selected == 0) wattron(win, COLOR_PAIR(5) | A_BOLD);
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, gy-1, left, "Gamma  (0.5 - 3.0): %.3f", gamma);
    wattroff(win, COLOR_PAIR(2));
    if (selected == 0) wattroff(win, COLOR_PAIR(5) | A_BOLD);

    draw_bar(win, gy, left, bar_w, gamma, 0.5, 3.0, 4, 3);

    int by = gy + 4;
    if (selected == 1) wattron(win, COLOR_PAIR(5) | A_BOLD);
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, by-1, left, "Brightness (0.1 - 2.0): %.3f", bright);
    wattroff(win, COLOR_PAIR(2));
    if (selected == 1) wattroff(win, COLOR_PAIR(5) | A_BOLD);

    draw_bar(win, by, left, bar_w, bright, 0.1, 2.0, 1, 3);

    wattron(win, COLOR_PAIR(3));
    mvwprintw(win, by+3, left, "Numeric: Gamma=%.3f   Brightness=%.3f", gamma, bright);
    wattroff(win, COLOR_PAIR(3));

    mvwprintw(win, rows-3, cols-45, "[Up/Down] select  [Left/Right] adjust  [R] Revert  [Q] Quit");

    wnoutrefresh(win);
    doupdate();
}
