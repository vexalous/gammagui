#define _POSIX_C_SOURCE 200809L
#include "proc.h"
#include <errno.h>
#include <ncurses.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/prctl.h>
#endif

volatile sig_atomic_t resized = 0;
volatile pid_t active_child = 0;

void sigwinch_handler(int sig){ (void)sig; resized = 1; }

void terminate_child_group(pid_t childpid){
    if (childpid <= 0) return;
    kill(-childpid, SIGTERM);
    
    const struct timespec ts = {0, 200000000L};
    nanosleep(&ts, NULL);
    
    kill(-childpid, SIGKILL);
    
    int st; 
    while (waitpid(-1, &st, WNOHANG) > 0);
}

void shutdown_handler(int sig){
    pid_t c = active_child;
    if (c > 0) terminate_child_group(c);
    
    endwin();
    _exit(128 + (sig & 0x7f));
}

int spawn_and_wait(const char *binpath){
    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0){
#if defined(__linux__)
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (getppid() == 1) _exit(127);
#endif
        if (setsid() < 0) _exit(127); 
        
        int fd = open("/dev/tty", O_RDWR);
        if (fd >= 0){
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            if (fd > STDERR_FILENO) close(fd);
        }
        
        signal(SIGINT, SIG_DFL); 
        signal(SIGTERM, SIG_DFL);
        
        execl(binpath, binpath, (char *)NULL);
        _exit(127);
    }
    
    active_child = pid;
    int status = 0;
    pid_t w;
    
    while ((w = waitpid(pid, &status, 0)) == -1 && errno == EINTR);
    
    active_child = 0;
    
    while ((w = waitpid(-1, &status, WNOHANG)) > 0);
    
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    
    return -1;
}
