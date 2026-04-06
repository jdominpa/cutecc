#ifndef DIAG_H_
#define DIAG_H_

#include <stddef.h>

typedef struct {
    const char *file_path;
    size_t line;
    size_t col;
} Loc;

typedef enum {
    INFO,
    WARNING,
    ERROR,
} ReportLevel;

void diag_report_at(ReportLevel level, Loc loc, const char *fmt, ...);

#endif  // DIAG_H_
