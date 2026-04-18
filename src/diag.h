#ifndef DIAG_H_
#define DIAG_H_

#include <stddef.h>
#include <stdnoreturn.h>

typedef struct {
    const char *file_path;
    size_t line;
    size_t col;
} Loc;

typedef enum {
    DIAG_INFO,
    DIAG_WARNING,
    DIAG_ERROR,
} DiagLevel;

void diag_report(DiagLevel level, const char *fmt, ...);
void diag_report_at(DiagLevel level, Loc loc, const char *fmt, ...);

#define diag_report_at_token(level, t, fmt) diag_report_at((level), (t).loc, (fmt))

#endif  // DIAG_H_
