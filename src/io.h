#ifndef IO_H_
#define IO_H_

#include <stdbool.h>

#include "arena.h"

bool read_entire_file(Arena *a, const char *path, char **out, size_t *out_len);

#endif  // IO_H_
