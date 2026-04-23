#ifndef PARSER_H_
#define PARSER_H_

#include <stdbool.h>
#include <stddef.h>

#include "arena.h"
#include "ast.h"
#include "lexer.h"

typedef struct {
    Arena *a;
    Token *tokens;
    size_t token_count;
    size_t pos;
    bool panic_mode;
    size_t err_count;
} Parser;

typedef struct {
    uint8_t left;
    uint8_t right;
} BindPower;

Parser parser_init_from_file_path(Arena *a, const char *file_path);
void parse_transl_unit(Parser *p);

#endif  // PARSER_H_
