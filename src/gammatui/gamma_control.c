#define _POSIX_C_SOURCE 200809L
#include "gamma_control.h"
#include <stdio.h>
#include <string.h>

void apply_values(double gamma, double bright) {
    if (display_output[0] == '\0') {
        return;
    }

    char value_str[32];
    int len = snprintf(value_str, sizeof(value_str), "%.3f:%.3f:%.3f", gamma, gamma, gamma);
    if (len > 0 && (size_t)len < sizeof(value_str)) {
        xr_call_async(display_output, "--gamma", value_str);
    }

    len = snprintf(value_str, sizeof(value_str), "%.3f", bright);
    if (len > 0 && (size_t)len < sizeof(value_str)) {
        xr_call_async(display_output, "--brightness", value_str);
    }
}

void revert_values() {
    if (display_output[0] == '\0') {
        return;
    }
    static const char* const DEFAULT_GAMMA = "1:1:1";
    static const char* const DEFAULT_BRIGHTNESS = "1";
    xr_call_async(display_output, "--gamma", DEFAULT_GAMMA);
    xr_call_async(display_output, "--brightness", DEFAULT_BRIGHTNESS);
}
