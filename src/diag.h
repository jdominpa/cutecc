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
    DIAG_FATAL,
} DiagLevel;

void diag_report_at(DiagLevel level, Loc loc, const char *fmt, ...);
noreturn void diag_fatal(const char *fmt, ...);
noreturn void diag_fatal_at(Loc loc, const char *fmt, ...);

#define diag_report_at_token(level, t, fmt, ...) \
    diag_report_at((level), (t).loc, (fmt) __VA_OPT__(, ) __VA_ARGS__)

#endif  // DIAG_H_
