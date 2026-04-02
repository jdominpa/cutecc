#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "lexer.h"

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
    fread(content, 1, size, f);
    fclose(f);
    content[size] = '\0';

    Lexer l = lexer_init(file_path, content, strlen(content));
    Token t = lexer_next_token(&l);
    while (t.kind != TK_EOF) {
        static_assert(TK_COUNT == 9, "TokenKind count has changed");
        switch (t.kind) {
        case TK_INVALID:
            fprintf(stderr, "TK_INVALID: ");
            break;
        case TK_OPAREN:
            fprintf(stderr, "TK_OPAREN: ");
            break;
        case TK_CPAREN:
            fprintf(stderr, "TK_CPAREN: ");
            break;
        case TK_OBRACE:
            fprintf(stderr, "TK_OBRACE: ");
            break;
        case TK_CBRACE:
            fprintf(stderr, "TK_CBRACE: ");
            break;
        case TK_SEMICOLON:
            fprintf(stderr, "TK_SEMICOLON: ");
            break;
        case TK_IDENT:
            fprintf(stderr, "TK_IDENT: ");
            break;
        case TK_NUM:
            fprintf(stderr, "TK_NUM: ");
            break;
        case TK_EOF:
            UNREACHABLE("main");
            break;
        default:
            UNREACHABLE("main");
            break;
        }
        fprintf(stderr, "%.*s\n", (int) t.len, t.pos);
        t = lexer_next_token(&l);
    }

    free(content);
    return 0;
}
