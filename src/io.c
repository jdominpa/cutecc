#include "io.h"

#include <stdio.h>

// Reads the whole file at `path` into a NUL-terminated buffer allocated from
// `a`, stored in `*out`. `out_len` receives the length without the terminator
// and may be NULL. Returns false on failure, leaving `*out` untouched.
bool read_entire_file(Arena *a, const char *path, char **out, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }
    long n = ftell(f);
    if (n < 0) {
        fclose(f);
        return false;
    }
    rewind(f);

    char *buf = arena_alloc_many(a, char, (size_t) n + 1);
    if (!buf) {
        fclose(f);
        return false;
    }
    size_t size_read = fread(buf, 1, (size_t) n, f);
    int err = ferror(f);
    fclose(f);
    if (err)
        return false;

    buf[size_read] = '\0';
    *out = buf;
    if (out_len) *out_len = size_read;
    return true;
}
