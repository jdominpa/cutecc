#include "diag.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const char *diag_level_as_str[] = {
    [DIAG_INFO] = "info",
    [DIAG_WARNING] = "warning",
    [DIAG_ERROR] = "error",
};

void diag_report(DiagLevel level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "cutecc: %s: ", diag_level_as_str[level]);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    // TODO: handle errors more gracefully
    if (level == DIAG_ERROR)
        exit(1);
}

void diag_report_at(DiagLevel level, Loc loc, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s:%zu:%zu: %s: ", loc.file_path, loc.line, loc.col, diag_level_as_str[level]);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    // TODO: handle errors more gracefully
    if (level == DIAG_ERROR)
        exit(1);
}
