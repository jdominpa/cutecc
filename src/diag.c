#include "diag.h"

#include <stdarg.h>
#include <stdio.h>

#include "common.h"

void diag_report(DiagLevel level, Loc loc, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s:%zu:%zu: ", loc.file_path, loc.line, loc.col);
    switch (level) {
    case DIAG_INFO:
        fprintf(stderr, "INFO: ");
        break;
    case DIAG_WARNING:
        fprintf(stderr, "WARNING: ");
        break;
    case DIAG_ERROR:
        fprintf(stderr, "ERROR: ");
        break;
    default:
        UNREACHABLE("diag_report");
    }
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    // TODO: don't panic when we encounter an error
    if (level == DIAG_ERROR)
        exit(1);
}
