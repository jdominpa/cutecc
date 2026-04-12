#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Lexer lexer_init_from_source(const char *file_path, const char *source)
{
    return (Lexer) {
        .file_path = file_path,
        .source = source,
        .size = strlen(source),
    };
}

Lexer lexer_init_from_file_path(const char *file_path)
{
    /* Read file */
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: ERROR: could not open file '%s' for parsing\n", file_path, file_path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *source = malloc(size + 1);
    if (source == NULL) {
        fprintf(stderr, "%s: ERROR: could not allocate memory to read file '%s'\n", file_path, file_path);
        exit(1);
    }
    size_t size_read = fread(source, 1, size, f);
    if (size_read != size) {
        fprintf(stderr, "%s: ERROR: expected %zu bytes when reading '%s' but got %zu\n", file_path, size, file_path, size_read);
        exit(1);
    }
    fclose(f);
    source[size] = '\0';

    return lexer_init_from_source(file_path, source);
}

static Loc lexer_get_loc(Lexer *l)
{
    return (Loc) {
        .file_path = l->file_path,
        .line = l->line + 1,
        .col = l->pos - l->bol + 1,
    };
}

static bool lexer_bump(Lexer *l)
{
    if (l->pos >= l->size)
        return false;
    if (l->source[l->pos++] == '\n') {
        l->bol = l->pos;
        l->line++;
    }
    return true;
}

static bool lexer_bump_bytes(Lexer *l, size_t n)
{
    while (n--)
        if (!lexer_bump(l))
            return false;
    return true;
}

static char lexer_peek_first(Lexer *l)
{
    if (l->pos + 1 >= l->size)
        return '\0';
    return l->source[l->pos + 1];
}

static char lexer_peek_second(Lexer *l)
{
    if (l->pos + 2 >= l->size)
        return '\0';
    return l->source[l->pos + 2];
}

static bool lexer_starts_with(Lexer *l, const char *prefix)
{
    size_t len = strlen(prefix);
    if (l->pos + len <= l->size) {
        for (size_t i = 0; i < len; ++i)
            if (l->source[l->pos + i] != prefix[i])
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
        while (l->pos < l->size && isspace(l->source[l->pos]))
            lexer_bump(l);
        if (l->pos < l->size && l->source[l->pos] == '/') {
            switch (lexer_peek_first(l)) {
            case '/':
                lexer_bump_bytes(l, 2);
                while (l->pos < l->size && l->source[l->pos] != '\r' &&
                       l->source[l->pos] != '\n')
                    lexer_bump(l);
                continue;
            case '*':
                Loc comment_beg = lexer_get_loc(l);
                lexer_bump_bytes(l, 2);
                while (l->pos < l->size && !lexer_starts_with(l, "*/"))
                    lexer_bump(l);
                if (l->pos >= l->size)
                    diag_report_at(ERROR, comment_beg,
                                   "unclosed comment block");
                lexer_bump_bytes(l, 2);
                continue;
            default:
                break;
            }
        }
        break;
    }

    t.loc = lexer_get_loc(l);

    // EOF
    if (l->pos >= l->size) {
        t.kind = TK_EOF;
        t.start = l->source + l->size;
        return t;
    }

    // Identifier
    if (lexer_is_ident_start(l->source[l->pos])) {
        t.kind = TK_IDENT;
        t.start = l->source + l->pos;
        while (l->pos < l->size && lexer_is_ident_cont(l->source[l->pos])) {
            t.len++;
            lexer_bump(l);
        }
        return t;
    }

    // Number
    if (isdigit(l->source[l->pos])) {
        t.kind = TK_NUM;
        t.start = l->source + l->pos;
        // TODO: handle non-integer numbers
        while (l->pos < l->size && isdigit(l->source[l->pos])) {
            t.len++;
            lexer_bump(l);
        }
        return t;
    }

    // String
    if (l->source[l->pos] == '"') {
        t.kind = TK_STR;
        t.start = l->source + l->pos;
        lexer_bump(l);
        while (l->pos < l->size && l->source[l->pos] != '"') {
            if (l->source[l->pos] == '\n' || l->source[l->pos] == '\0')
                diag_report_at(ERROR, t.loc, "unclosed string literal");
            if (l->source[l->pos] == '\\')
                lexer_bump(l);
            lexer_bump(l);
        }
        if (l->pos >= l->size)
            diag_report_at(ERROR, t.loc, "unclosed string literal");
        lexer_bump(l);
        t.len = l->pos - (t.start - l->source);
        return t;
    }

#define MAKE_TOKEN(k, length)          \
    do {                               \
        t.kind = (k);                  \
        t.start = l->source + l->pos;  \
        t.len = (length);              \
        lexer_bump_bytes(l, (length)); \
        return t;                      \
    } while (0)

    char c = l->source[l->pos];
    char first = lexer_peek_first(l);
    char second = lexer_peek_second(l);

    switch (c) {
    case '(':
        MAKE_TOKEN(TK_OPAREN, 1);
        break;
    case ')':
        MAKE_TOKEN(TK_CPAREN, 1);
        break;
    case '{':
        MAKE_TOKEN(TK_OBRACE, 1);
        break;
    case '}':
        MAKE_TOKEN(TK_CBRACE, 1);
        break;
    case '[':
        MAKE_TOKEN(TK_OBRACK, 1);
        break;
    case ']':
        MAKE_TOKEN(TK_CBRACK, 1);
        break;
    case ';':
        MAKE_TOKEN(TK_SEMI, 1);
        break;
    case ':':
        MAKE_TOKEN(TK_COLON, 1);
        break;
    case '.':
        MAKE_TOKEN(TK_DOT, 1);
        break;
    case ',':
        MAKE_TOKEN(TK_COMMA, 1);
        break;
    case '?':
        MAKE_TOKEN(TK_QUESTION, 1);
        break;
    case '+':
        if (first == '=')
            MAKE_TOKEN(TK_PLUS_EQ, 2);
        else if (first == '+')
            MAKE_TOKEN(TK_PLUS_PLUS, 2);
        else
            MAKE_TOKEN(TK_PLUS, 1);
        break;
    case '-':
        if (first == '=')
            MAKE_TOKEN(TK_MINUS_EQ, 2);
        else if (first == '-')
            MAKE_TOKEN(TK_MINUS_MINUS, 2);
        else if (first == '>')
            MAKE_TOKEN(TK_MINUS_GT, 2);
        else
            MAKE_TOKEN(TK_MINUS, 1);
        break;
    case '*':
        if (first == '=')
            MAKE_TOKEN(TK_STAR_EQ, 2);
        else
            MAKE_TOKEN(TK_STAR, 1);
        break;
    case '/':
        if (first == '=')
            MAKE_TOKEN(TK_SLASH_EQ, 2);
        else
            MAKE_TOKEN(TK_SLASH, 1);
        break;
    case '%':
        if (first == '=')
            MAKE_TOKEN(TK_PERCENT_EQ, 2);
        else
            MAKE_TOKEN(TK_PERCENT, 1);
        break;
    case '~':
        MAKE_TOKEN(TK_TILDE, 1);
        break;
    case '&':
        if (first == '=')
            MAKE_TOKEN(TK_AMP_EQ, 2);
        else if (first == '&')
            MAKE_TOKEN(TK_AMP_AMP, 2);
        else
            MAKE_TOKEN(TK_AMP, 1);
        break;
    case '|':
        if (first == '=')
            MAKE_TOKEN(TK_PIPE_EQ, 2);
        else if (first == '|')
            MAKE_TOKEN(TK_PIPE_PIPE, 2);
        else
            MAKE_TOKEN(TK_PIPE, 1);
        break;
    case '^':
        if (first == '=')
            MAKE_TOKEN(TK_CARET_EQ, 2);
        else
            MAKE_TOKEN(TK_CARET, 1);
        break;
    case '!':
        if (first == '=')
            MAKE_TOKEN(TK_BANG_EQ, 2);
        else
            MAKE_TOKEN(TK_BANG, 1);
        break;
    case '=':
        if (first == '=')
            MAKE_TOKEN(TK_EQ_EQ, 2);
        else
            MAKE_TOKEN(TK_EQ, 1);
        break;
    case '<':
        if (first == '<')
            if (second == '=')
                MAKE_TOKEN(TK_LT_LT_EQ, 3);
            else
                MAKE_TOKEN(TK_LT_LT, 2);
        else if (first == '=')
            MAKE_TOKEN(TK_LT_EQ, 2);
        else
            MAKE_TOKEN(TK_LT, 1);
        break;
    case '>':
        if (first == '>')
            if (second == '=')
                MAKE_TOKEN(TK_GT_GT_EQ, 3);
            else
                MAKE_TOKEN(TK_GT_GT, 2);
        else if (first == '=')
            MAKE_TOKEN(TK_GT_EQ, 2);
        else
            MAKE_TOKEN(TK_GT, 1);
        break;
    }

#undef MAKE_TOKEN

    t.start = l->source + l->pos;
    t.len = 1;
    lexer_bump(l);
    return t;
}
