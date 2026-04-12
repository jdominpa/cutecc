#ifndef PARSER_H_
#define PARSER_H_

#include <stdbool.h>
#include <stddef.h>

#include "arena.h"
#include "lexer.h"

typedef struct {
    Arena a;
    Token *tokens;
    size_t token_count;
    size_t pos;
    bool panic_mode;
    size_t err_count;
} Parser;

Parser parser_init(const char *file_path);

#endif  // PARSER_H_
