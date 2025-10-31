#define _POSIX_C_SOURCE 200809L
#include "gammatui.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char display_output[128] = {0};

static pthread_mutex_t debounce_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct timespec last_apply = {0,0};
static const long DEBOUNCE_MS = 80;

int detect_output(char *outbuf, size_t outlen) {
    FILE *fp = popen("xrandr --query 2>/dev/null", "r");
    if (!fp) return 0;
    char line[512];
    char candidate[128] = {0};
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, " connected")) {
            if (strstr(line, " primary ")) {
                sscanf(line, "%127s", outbuf);
                pclose(fp);
                return 1;
            }
            if (candidate[0] == '\0') sscanf(line, "%127s", candidate);
        }
    }
    if (candidate[0]) {
        strncpy(outbuf, candidate, outlen-1);
        outbuf[outlen-1] = '\0';
        pclose(fp);
        return 1;
    }
    pclose(fp);
    return 0;
}

struct xr_args { char output[128]; char opt[64]; char val[64]; };

static void *xr_worker(void *arg) {
    struct xr_args *a = arg;
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "xrandr --output %s %s %s 2>/dev/null", a->output, a->opt, a->val);
    system(cmd);
    free(a);
    return NULL;
}

void xr_call_async(const char *output, const char *opt, const char *val) {
    struct xr_args *a = malloc(sizeof(*a));
    if (!a) return;
    strncpy(a->output, output, sizeof(a->output)-1);
    a->output[sizeof(a->output)-1] = '\0';
    strncpy(a->opt, opt, sizeof(a->opt)-1);
    a->opt[sizeof(a->opt)-1] = '\0';
    strncpy(a->val, val, sizeof(a->val)-1);
    a->val[sizeof(a->val)-1] = '\0';
    pthread_t t;
    if (pthread_create(&t, NULL, xr_worker, a) == 0) pthread_detach(t);
}

int debounce_allow() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    pthread_mutex_lock(&debounce_mutex);
    long diff_ms = (now.tv_sec - last_apply.tv_sec) * 1000 + (now.tv_nsec - last_apply.tv_nsec) / 1000000;
    if (diff_ms >= DEBOUNCE_MS) {
        last_apply = now;
        pthread_mutex_unlock(&debounce_mutex);
        return 1;
    }
    pthread_mutex_unlock(&debounce_mutex);
    return 0;
}

double clamp_double(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}