#define _POSIX_C_SOURCE 200809L
#include "gamma_control.h"
#include <stdio.h>

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
