#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

static const char *token_kind_name(TokenKind kind)
{
    assert(TK_COUNT == 50 && "TokenKind count has changed");
    const char *kind_names[] = {
        [TK_INVALID] = "TK_INVALID",
        [TK_EOF] = "TK_EOF",
        [TK_IDENT] = "TK_IDENT",
        [TK_NUM] = "TK_NUM",
        [TK_STR] = "TK_STR",
        [TK_OPAREN] = "TK_OPAREN",
        [TK_CPAREN] = "TK_CPAREN",
        [TK_OBRACE] = "TK_OBRACE",
        [TK_CBRACE] = "TK_CBRACE",
        [TK_OBRACK] = "TK_OBRACK",
        [TK_CBRACK] = "TK_CBRACK",
        [TK_SEMI] = "TK_SEMI",
        [TK_COLON] = "TK_COLON",
        [TK_DOT] = "TK_DOT",
        [TK_COMMA] = "TK_COMMA",
        [TK_PLUS] = "TK_PLUS",
        [TK_PLUS_PLUS] = "TK_PLUS_PLUS",
        [TK_MINUS] = "TK_MINUS",
        [TK_MINUS_MINUS] = "TK_MINUS_MINUS",
        [TK_STAR] = "TK_STAR",
        [TK_SLASH] = "TK_SLASH",
        [TK_PERCENT] = "TK_PERCENT",
        [TK_EQ] = "TK_EQ",
        [TK_PLUS_EQ] = "TK_PLUS_EQ",
        [TK_MINUS_EQ] = "TK_MINUS_EQ",
        [TK_STAR_EQ] = "TK_STAR_EQ",
        [TK_SLASH_EQ] = "TK_SLASH_EQ",
        [TK_PERCENT_EQ] = "TK_PERCENT_EQ",
        [TK_AMP_EQ] = "TK_AMP_EQ",
        [TK_PIPE_EQ] = "TK_PIPE_EQ",
        [TK_CARET_EQ] = "TK_CARET_EQ",
        [TK_LT_LT_EQ] = "TK_LT_LT_EQ",
        [TK_GT_GT_EQ] = "TK_GT_GT_EQ",
        [TK_TILDE] = "TK_TILDE",
        [TK_AMP] = "TK_AMP",
        [TK_PIPE] = "TK_PIPE",
        [TK_CARET] = "TK_CARET",
        [TK_LT_LT] = "TK_LT_LT",
        [TK_GT_GT] = "TK_GT_GT",
        [TK_BANG] = "TK_BANG",
        [TK_AMP_AMP] = "TK_AMP_AMP",
        [TK_PIPE_PIPE] = "TK_PIPE_PIPE",
        [TK_EQ_EQ] = "TK_EQ_EQ",
        [TK_BANG_EQ] = "TK_BANG_EQ",
        [TK_LT] = "TK_LT",
        [TK_LT_EQ] = "TK_LT_EQ",
        [TK_GT] = "TK_GT",
        [TK_GT_EQ] = "TK_GT_EQ",
        [TK_MINUS_GT] = "TK_MINUS_GT",
        [TK_QUESTION] = "TK_QUESTION",
        [TK_COUNT] = "TK_COUNT",
    };
    return kind_names[kind];
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
        return 1;
    }

    Parser p = parser_init(argv[1]);
    for (size_t i = 0; i < p.token_count; ++i) {
        Token t = p.tokens[i];
        printf("%s:%zu:%zu: %s: %.*s\n", t.loc.file_path, t.loc.line, t.loc.col,
               token_kind_name(t.kind), (int) t.len, t.start);
    }

    return 0;
}
