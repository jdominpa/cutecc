#include "io.h"

#include <stdio.h>

bool read_entire_file(Arena *a, const char *path, char *out, size_t *out_len)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }
    size_t n = ftell(f);
    if (n < 0) {
        fclose(f);
        return false;
    }
    rewind(f);

    out = arena_alloc_many(a, char, n + 1);
    if (!out) {
        fclose(f);
        return false;
    }
    size_t size_read = fread(out, 1, n, f);
    int err = ferror(f);
    fclose(f);
    if (err)
        return false;

    out[size_read] = '\0';
    if (out_len) *out_len = size_read;
    return true;
}
