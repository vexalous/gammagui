#define _POSIX_C_SOURCE 200809L
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

static bool exe_dir(char *out, size_t outlen, const char *argv0) {
#if defined(__linux__)
    char buf[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        char *d = dirname(buf);
        if (!d) return false;
        strncpy(out, d, outlen - 1);
        out[outlen - 1] = '\0';
        return true;
    }
#endif
    if (!argv0 || argv0[0] == '\0') return false;
    if (strchr(argv0, '/')) {
        char tmp[PATH_MAX];
        if (argv0[0] == '/') {
            strncpy(tmp, argv0, sizeof(tmp)-1);
            tmp[sizeof(tmp)-1] = '\0';
        } else {
            char cwd[PATH_MAX];
            if (!getcwd(cwd, sizeof(cwd))) return false;
            if (snprintf(tmp, sizeof(tmp), "%s/%s", cwd, argv0) >= (int)sizeof(tmp)) return false;
        }
        char *d = dirname(tmp);
        if (!d) return false;
        strncpy(out, d, outlen - 1);
        out[outlen - 1] = '\0';
        return true;
    }
    return false;
}

bool config_path_for_exe(char *out, size_t outlen, const char *argv0) {
    char d[PATH_MAX];
    if (!exe_dir(d, sizeof(d), argv0)) return false;
    if (snprintf(out, outlen, "%s/config.json", d) >= (int)outlen) return false;
    return true;
}

bool load_config(struct cfg *c, const char *path) {
    if (!c) return false;

    c->output[0] = '\0';
    c->gamma_max = 3.0;
    c->bright_max = 2.0;

    FILE *f = fopen(path, "r");
    if (!f) return false;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
    long sz = ftell(f);
    if (sz <= 0 || sz > 200000) { fclose(f); return false; }
    rewind(f);

    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return false; }
    size_t got = fread(buf, 1, (size_t)sz, f);
    buf[got] = '\0';
    fclose(f);

    char *p;
    p = strstr(buf, "\"output\"");
    if (p) {
        p = strchr(p, ':');
        if (p) {
            p++;
            while (*p && (*p == ' ' || *p == '\t')) p++;
            if (*p == '\"') {
                p++;
                char *q = strchr(p, '\"');
                if (q) {
                    size_t n = (size_t)(q - p);
                    if (n >= OUTPUT_LEN) n = OUTPUT_LEN - 1;
                    memcpy(c->output, p, n);
                    c->output[n] = '\0';
                }
            }
        }
    }

    p = strstr(buf, "\"gamma_max\"");
    if (p) {
        double v = 0;
        if (sscanf(p, "\"gamma_max\"%*[^0-9.-]%lf", &v) == 1) c->gamma_max = v;
    }

    p = strstr(buf, "\"brightness_max\"");
    if (!p) p = strstr(buf, "\"bright_max\"");
    if (p) {
        double v = 0;
        if (sscanf(p, "%*[^0-9.-]%lf", &v) == 1) c->bright_max = v;
    }

    free(buf);
    return true;
}

bool save_config(const struct cfg *c, const char *path) {
    if (!c || !path) return false;
    char tmp[PATH_MAX];
    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp)) return false;
    FILE *f = fopen(tmp, "w");
    if (!f) return false;
    fprintf(f, "{\n");
    fprintf(f, "  \"output\": \"%s\",\n", c->output[0] ? c->output : "");
    fprintf(f, "  \"gamma_max\": %.3f,\n", c->gamma_max);
    fprintf(f, "  \"brightness_max\": %.3f\n", c->bright_max);
    fprintf(f, "}\n");
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    if (rename(tmp, path) != 0) return false;
    return true;
}
