#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

typedef struct {
    TokenKind kind;
    const char *text;
} Punct;

Punct puncts[] = {
    { .kind = TK_OPAREN, .text = "(" },    { .kind = TK_CPAREN, .text = ")" },
    { .kind = TK_OBRACE, .text = "{" },    { .kind = TK_CBRACE, .text = "}" },
    { .kind = TK_SEMICOLON, .text = ";" },
};
#define PUNCT_COUNT sizeof(puncts) / sizeof(puncts[0])

void lexer_report_at(ReportLevel level, Loc loc, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s:%zu:%zu: ", loc.file_path, loc.row, loc.col);
    switch (level) {
    case INFO:
        fprintf(stderr, "info: ");
        break;
    case WARNING:
        fprintf(stderr, "warning: ");
        break;
    case ERROR:
        fprintf(stderr, "error: ");
        break;
    default:
        UNREACHABLE("lexer_report_at");
    }
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    if (level == ERROR)
        exit(1);
}

Lexer lexer_init(const char *file_path, const char *content, size_t size)
{
    return (Lexer) {
        .file_path = file_path,
        .content = content,
        .size = size,
    };
}

static Loc lexer_loc(Lexer *l)
{
    return (Loc) {
        .file_path = l->file_path,
        .row = l->row + 1,
        .col = l->cur - l->bol + 1,
    };
}

static bool lexer_advance_char(Lexer *l)
{
    if (l->cur >= l->size)
        return false;
    if (l->content[l->cur++] == '\n') {
        l->bol = l->cur;
        l->row++;
    }
    return true;
}

static bool lexer_advance_chars(Lexer *l, size_t n)
{
    while (n--)
        if (!lexer_advance_char(l))
            return false;
    return true;
}

static bool lexer_starts_with(Lexer *l, const char *prefix)
{
    assert(l->cur < l->size && "lexer cursor out of bounds");
    size_t len = strlen(prefix);
    if (l->cur + len <= l->size) {
        for (size_t i = 0; i < len; ++i)
            if (l->content[l->cur + i] != prefix[i])
                return false;
        return true;
    }
    return false;
}

static bool lexer_is_ident_start(char c)
{
    return isalpha(c) || c == '_';
}

static bool lexer_is_ident_cont(char c)
{
    return isalnum(c) || c == '_';
}

Token lexer_next_token(Lexer *l)
{
    Token t = { 0 };

    // Skip whitespace and comments
    for (;;) {
        while (l->cur < l->size && isspace(l->content[l->cur]))
            lexer_advance_char(l);
        if (l->cur < l->size && lexer_starts_with(l, "//")) {
            while (l->cur < l->size && l->content[l->cur] != '\r' &&
                   l->content[l->cur] != '\n')
                lexer_advance_char(l);
            continue;
        }
        if (l->cur < l->size && lexer_starts_with(l, "/*")) {
            Loc comment_beg = lexer_loc(l);
            lexer_advance_chars(l, 2);
            while (l->cur < l->size && !lexer_starts_with(l, "*/"))
                lexer_advance_char(l);
            if (l->cur >= l->size)
                lexer_report_at(ERROR, comment_beg, "unclosed comment block");
            lexer_advance_chars(l, 2);
            continue;
        }
        break;
    }

    t.loc = lexer_loc(l);

    // EOF
    if (l->cur >= l->size) {
        t.kind = TK_EOF;
        t.pos = l->content + l->size;
        return t;
    }

    // Punctuators
    for (size_t i = 0; i < PUNCT_COUNT; ++i) {
        if (lexer_starts_with(l, puncts[i].text)) {
            t.kind = puncts[i].kind;
            t.pos = l->content + l->cur;
            t.len = strlen(puncts[i].text);
            lexer_advance_chars(l, t.len);
            return t;
        }
    }

    // Identifier
    if (lexer_is_ident_start(l->content[l->cur])) {
        t.kind = TK_IDENT;
        t.pos = l->content + l->cur;
        while (l->cur < l->size && lexer_is_ident_cont(l->content[l->cur])) {
            t.len++;
            lexer_advance_char(l);
        }
        return t;
    }

    // Number
    if (isdigit(l->content[l->cur])) {
        t.kind = TK_NUM;
        t.pos = l->content + l->cur;
        // TODO: handle non-integer numbers
        while (l->cur < l->size && isdigit(l->content[l->cur])) {
            t.len++;
            lexer_advance_char(l);
        }
        return t;
    }

    t.pos = l->content + l->cur;
    t.len = 1;
    lexer_advance_char(l);
    return t;
}
