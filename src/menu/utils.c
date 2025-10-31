#define _POSIX_C_SOURCE 200809L
#include "utils.h"
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>

bool is_executable_file(const char *p){
    struct stat s;
    return p && stat(p,&s)==0 && S_ISREG(s.st_mode) && access(p,X_OK)==0;
}

bool resolve_exe_dir(char *out, size_t outlen, const char *argv0){
#if defined(__linux__)
    char b[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", b, sizeof b - 1);
    if (n > 0){
        b[n] = '\0';
        char *d = dirname(b);
        if (d) return snprintf(out,outlen,"%s",d) < (int)outlen;
    }
#endif
    if (!argv0 || !*argv0) return false;
    if (!strchr(argv0, '/')) return false;
    char tmp[PATH_MAX];
    if (argv0[0] == '/') {
        if (snprintf(tmp, sizeof tmp, "%s", argv0) >= (int)sizeof tmp) return false;
    } else {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof cwd)) return false;
        if (snprintf(tmp, sizeof tmp, "%s/%s", cwd, argv0) >= (int)sizeof tmp) return false;
    }
    char *d = dirname(tmp);
    if (!d) return false;
    return snprintf(out,outlen,"%s",d) < (int)outlen;
}

bool build_gammatui_path(char *out, size_t outlen, const char *argv0){
    char d[PATH_MAX];
    return resolve_exe_dir(d,sizeof d,argv0) && snprintf(out,outlen,"%s/../gammatui/gammatui.elf",d) < (int)outlen;
}
bool build_settings_path(char *out, size_t outlen, const char *argv0){
    char d[PATH_MAX];
    return resolve_exe_dir(d,sizeof d,argv0) && snprintf(out,outlen,"%s/../settings/brightnesstui.elf",d) < (int)outlen;
}