#ifndef DIAG_H_
#define DIAG_H_

#include <stddef.h>

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

void diag_report(DiagLevel level, Loc loc, const char *fmt, ...);

#endif  // DIAG_H_
