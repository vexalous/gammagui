#define _POSIX_C_SOURCE 200809L
#include "gammatui.h"
#include <string.h>

enum {
    COLOR_PAIR_DEFAULT = 1,
    COLOR_PAIR_TITLE,
    COLOR_PAIR_HEADER,
    COLOR_PAIR_LABEL,
    COLOR_PAIR_VALUE,
    COLOR_PAIR_VALUE_SELECTED,
    COLOR_PAIR_BAR_FILL,
    COLOR_PAIR_BAR_EMPTY,
    COLOR_PAIR_HELP
};

void init_colors_safe() {
    if (!has_colors()) return;
    start_color();
    use_default_colors();
    
    init_pair(COLOR_PAIR_DEFAULT,      COLOR_WHITE,   -1);
    init_pair(COLOR_PAIR_TITLE,        COLOR_CYAN,    -1);
    init_pair(COLOR_PAIR_HEADER,       COLOR_YELLOW,  -1);
    init_pair(COLOR_PAIR_LABEL,        COLOR_WHITE,   -1);
    init_pair(COLOR_PAIR_VALUE,        COLOR_CYAN,    -1);
    init_pair(COLOR_PAIR_VALUE_SELECTED, COLOR_MAGENTA, -1);
    init_pair(COLOR_PAIR_BAR_FILL,     COLOR_GREEN,   -1);
    init_pair(COLOR_PAIR_BAR_EMPTY,    COLOR_WHITE,   -1);
    init_pair(COLOR_PAIR_HELP,         COLOR_BLUE,    -1);
}

void draw_bar(WINDOW *w, int y, int x, int width, double value, double lo, double hi) {
    int inner_width = width - 2;
    mvwaddch(w, y, x, '[');
    mvwaddch(w, y, x + width - 1, ']');

    double frac = (value - lo) / (hi - lo);
    frac = (frac < 0) ? 0 : (frac > 1) ? 1 : frac;
    int fill_width = (int)(frac * inner_width);

    wattron(w, COLOR_PAIR(COLOR_PAIR_BAR_FILL));
    for (int i = 0; i < fill_width; ++i) {
        mvwaddch(w, y, x + 1 + i, '#');
    }
    wattroff(w, COLOR_PAIR(COLOR_PAIR_BAR_FILL));

    wattron(w, COLOR_PAIR(COLOR_PAIR_BAR_EMPTY));
    for (int i = fill_width; i < inner_width; ++i) {
        mvwaddch(w, y, x + 1 + i, '-');
    }
    wattroff(w, COLOR_PAIR(COLOR_PAIR_BAR_EMPTY));
}

void draw_rounded_border(WINDOW *w) {
    wborder(w, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
}

void draw_ui(WINDOW *win, double gamma, double bright, int selected, int rows, int cols) {
    werase(win);
    draw_rounded_border(win);

    const char *title = "Gamma & Brightness Control";
    int title_x = (cols - strlen(title)) / 2;
    wattron(win, COLOR_PAIR(COLOR_PAIR_TITLE) | A_BOLD);
    mvwprintw(win, 1, title_x, "%s", title);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_TITLE) | A_BOLD);

    wattron(win, COLOR_PAIR(COLOR_PAIR_HEADER));
    char output_buf[128];
    snprintf(output_buf, sizeof(output_buf), "Display Output: %s", display_output[0] ? display_output : "(not detected)");
    mvwprintw(win, 2, (cols - strlen(output_buf)) / 2, "%s", output_buf);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_HEADER));
    
    mvwaddch(win, 3, 1, ACS_LTEE);
    mvwhline(win, 3, 2, ACS_HLINE, cols - 4);
    mvwaddch(win, 3, cols - 2, ACS_RTEE);


    int left_margin = 4;
    int bar_width = cols - (left_margin * 2);
    int control_y_start = 5;

    int gamma_y = control_y_start;
    wattron(win, COLOR_PAIR(selected == 0 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_LABEL) | A_BOLD);
    mvwprintw(win, gamma_y, left_margin, "Gamma");
    wattroff(win, COLOR_PAIR(selected == 0 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_LABEL) | A_BOLD);

    char gamma_val_str[10];
    snprintf(gamma_val_str, sizeof(gamma_val_str), "%.2f", gamma);
    wattron(win, COLOR_PAIR(selected == 0 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_VALUE) | A_BOLD);
    mvwprintw(win, gamma_y, cols - left_margin - strlen(gamma_val_str), "%s", gamma_val_str);
    wattroff(win, COLOR_PAIR(selected == 0 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_VALUE) | A_BOLD);
    
    draw_bar(win, gamma_y + 1, left_margin, bar_width, gamma, 0.5, 3.0);

    int bright_y = gamma_y + 3;
    wattron(win, COLOR_PAIR(selected == 1 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_LABEL) | A_BOLD);
    mvwprintw(win, bright_y, left_margin, "Brightness");
    wattroff(win, COLOR_PAIR(selected == 1 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_LABEL) | A_BOLD);

    char bright_val_str[10];
    snprintf(bright_val_str, sizeof(bright_val_str), "%.2f", bright);
    wattron(win, COLOR_PAIR(selected == 1 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_VALUE) | A_BOLD);
    mvwprintw(win, bright_y, cols - left_margin - strlen(bright_val_str), "%s", bright_val_str);
    wattroff(win, COLOR_PAIR(selected == 1 ? COLOR_PAIR_VALUE_SELECTED : COLOR_PAIR_VALUE) | A_BOLD);

    draw_bar(win, bright_y + 1, left_margin, bar_width, bright, 0.1, 2.0);


    mvwaddch(win, rows - 3, 1, ACS_LTEE);
    mvwhline(win, rows - 3, 2, ACS_HLINE, cols - 4);
    mvwaddch(win, rows - 3, cols - 2, ACS_RTEE);
    
    const char *help_text = "[Up/Down] Select   [Left/Right] Adjust   [R] Reset   [Q] Quit";
    int help_x = (cols - strlen(help_text)) / 2;
    wattron(win, COLOR_PAIR(COLOR_PAIR_HELP));
    mvwprintw(win, rows - 2, help_x, "%s", help_text);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_HELP));

    wnoutrefresh(win);
    doupdate();
}
