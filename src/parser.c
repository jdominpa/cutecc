#include "parser.h"

#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "lexer.h"

Parser parser_init(const char *file_path)
{
    Parser p = { 0 };
    p.a = arena_init();

    /* Count amount of tokens */
    Lexer l = lexer_init_from_file_path(file_path);
    Token t;
    do {
        t = lexer_next_token(&l);
        p.token_count++;
    } while (t.kind != TK_EOF);

    /* Store tokens in `parser.tokens` */
    l.pos = l.bol = l.line = 0;
    p.tokens = (Token *) arena_alloc_many(&p.a, Token, p.token_count);
    for (size_t i = 0; i < p.token_count; ++i) {
        p.tokens[i] = lexer_next_token(&l);
        if (p.tokens[i].kind == TK_INVALID) {
            p.panic_mode = true;
            p.err_count++;
        }
    }

    return p;
}
