#include "diag.h"

#include <stdarg.h>
#include <stdio.h>

#include "common.h"

void diag_report_at(ReportLevel level, Loc loc, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s:%zu:%zu: ", loc.file_path, loc.line, loc.col);
    switch (level) {
    case INFO:
        fprintf(stderr, "info: ");
        break;
    case WARNING:
        fprintf(stderr, "warning: ");
        break;
    case ERROR:
        fprintf(stderr, "error: ");
        break;
    default:
        UNREACHABLE("diag_report_at");
    }
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    // TODO: don't panic when we encounter an error
    if (level == ERROR)
        exit(1);
}
