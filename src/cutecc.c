#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "lexer.h"

static const char *token_kind_name(TokenKind kind)
{
    assert(TK_COUNT == 18 && "TokenKind count has changed");
    const char *kind_names[] = {
        [TK_INVALID] = "TK_INVALID",
        [TK_EOF] = "TK_EOF",
        [TK_LPAREN] = "TK_OPAREN",
        [TK_CPAREN] = "TK_CPAREN",
        [TK_OBRACE] = "TK_OBRACE",
        [TK_CBRACE] = "TK_CBRACE",
        [TK_SEMICOLON] = "TK_SEMICOLON",
        [TK_DOT] = "TK_DOT",
        [TK_PLUS] = "TK_PLUS",
        [TK_PLUS_EQ] = "TK_PLUS_EQ",
        [TK_MINUS] = "TK_MINUS",
        [TK_MINUS_EQ] = "TK_MINUS_EQ",
        [TK_AST] = "TK_AST",
        [TK_AST_EQ] = "TK_AST_EQ",
        [TK_SLASH] = "TK_SLASH",
        [TK_SLASH_EQ] = "TK_SLASH_EQ",
        [TK_IDENT] = "TK_IDENT",
        [TK_NUM] = "TK_NUM",
    };
    return kind_names[kind];
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    const char *file_path = argv[1];
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: error: could not open file '%s'", argv[0],
                file_path);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = malloc(size + 1);
    if (content == NULL) {
        fprintf(stderr, "%s: error: out of memory\n", argv[0]);
        return 1;
    }
    UNUSED(fread(content, 1, size, f));
    fclose(f);
    content[size] = '\0';

    Lexer l = lexer_init(file_path, content, strlen(content));
    Token t = lexer_next_token(&l);
    while (t.kind != TK_EOF) {
        fprintf(stderr, "%s: %.*s\n", token_kind_name(t.kind), (int) t.len, t.pos);
        t = lexer_next_token(&l);
    }

    free(content);
    return 0;
}
