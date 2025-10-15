#include <limits.h>
#define _POSIX_C_SOURCE 200809L
#include "proc.h"
#include "ui.h"
#include "utils.h"
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv){
    struct sigaction sa = {0};
    sa.sa_handler = sigwinch_handler;
    sigemptyset(&sa.sa_mask); sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, NULL);

    struct sigaction sh = {0};
    sh.sa_handler = shutdown_handler;
    sigemptyset(&sh.sa_mask); sh.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sh, NULL); sigaction(SIGTERM, &sh, NULL);

    int rows = 24, cols = 80;
    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    getmaxyx(stdscr, rows, cols);

    WINDOW *win = newwin(rows - 2, cols - 2, 1, 1);
    keypad(win, TRUE); wtimeout(win, -1);

    int highlight = 0;

    char gammagui_path[PATH_MAX] = {0};
    bool gammagui_built = build_gammagui_path(gammagui_path, sizeof gammagui_path, (argc>0)?argv[0]:NULL);
    bool gammagui_ok = gammagui_built && is_executable_file(gammagui_path);

    char settings_path[PATH_MAX] = {0};
    bool settings_built = build_settings_path(settings_path, sizeof settings_path, (argc>0)?argv[0]:NULL);
    bool settings_ok = settings_built && is_executable_file(settings_path);

    draw_menu(win, highlight);

    for(;;){
        if (resized){ resized = 0; endwin(); refresh(); clear(); if (win){ delwin(win); win=NULL; } win = recreate_window(&rows,&cols); draw_menu(win, highlight); }

        int ch = wgetch(win);
        if (ch == KEY_RESIZE){ if (win){ delwin(win); win=NULL; } win = recreate_window(&rows,&cols); draw_menu(win, highlight); continue; }
        if (ch == ERR) continue;

        if (ch == KEY_UP) highlight = (highlight + 3 - 1) % 3;
        else if (ch == KEY_DOWN) highlight = (highlight + 1) % 3;
        else if (ch == '\n' || ch == KEY_ENTER){
            if (highlight == MI_Adjustment){
                if (!gammagui_ok){
                    show_message(win, "Not found",
                                 "Expected ../gammagui/gammagui.elf relative to menu.elf\nPlace gammagui.elf at ../gammagui/gammagui.elf and make it executable.");
                } else {
                    flushinp(); endwin();
                    int rc = spawn_and_wait(gammagui_path);
                    struct timespec ts = {0, 100000000L}; nanosleep(&ts,NULL);
                    if (win){ delwin(win); win = NULL; } win = recreate_window(&rows,&cols);
                    if (rc == 0) gammagui_ok = is_executable_file(gammagui_path);
                    else if (rc == -1) show_message(win, "Error", "Failed to run adjustment (fork/exec error).");
                    else if (rc == 127) show_message(win, "Error", "Adjustment failed to exec (exit 127).");
                    else if (rc >= 128){ char buf[128]; snprintf(buf,sizeof buf,"Adjustment terminated by signal %d", rc-128); show_message(win,"Crashed",buf); }
                    else { char buf[128]; snprintf(buf,sizeof buf,"Adjustment exited with status %d", rc); show_message(win,"Exited",buf); }
                }
            } else if (highlight == MI_Settings){
                if (!settings_ok){
                    show_message(win, "Not found",
                                 "Expected ../settings/brightnesstui.elf relative to menu.elf\nPlace brightnesstui.elf at ../settings/brightnesstui.elf and make it executable.");
                } else {
                    flushinp(); endwin();
                    int rc = spawn_and_wait(settings_path);
                    struct timespec ts = {0, 100000000L}; nanosleep(&ts,NULL);
                    if (win){ delwin(win); win = NULL; } win = recreate_window(&rows,&cols);
                    if (rc == 0) settings_ok = is_executable_file(settings_path);
                    else if (rc == -1) show_message(win, "Error", "Failed to run settings (fork/exec error).");
                    else if (rc == 127) show_message(win, "Error", "Settings failed to exec (exit 127).");
                    else if (rc >= 128){ char buf[128]; snprintf(buf,sizeof buf,"Settings terminated by signal %d", rc-128); show_message(win,"Crashed",buf); }
                    else { char buf[128]; snprintf(buf,sizeof buf,"Settings exited with status %d", rc); show_message(win,"Exited",buf); }
                }
            } else if (highlight == MI_Quit) break;
        } else if (ch == 'q' || ch == 'Q') break;

        getmaxyx(stdscr, rows, cols);
        if (win) wresize(win, rows - 2, cols - 2);
        draw_menu(win, highlight);
    }

    pid_t c = active_child;
    if (c > 0) terminate_child_group(c);
    if (win) delwin(win);
    endwin();
    return 0;
}
