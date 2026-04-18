#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "ast.h"
#include "common.h"
#include "diag.h"
#include "lexer.h"

Parser parser_init_from_file_path(Arena *a, const char *file_path)
{
    Parser p = { 0 };
    p.a = a;

    // Count amount of tokens
    Lexer l = lexer_init_from_file_path(file_path);
    Token t = { 0 };
    do {
        t = lexer_next_token(&l);
        p.token_count++;
    } while (t.kind != TK_EOF);

    // Store tokens in token array
    l.pos = l.bol = l.line = 0;
    p.tokens = (Token *) arena_alloc_many(p.a, Token, p.token_count);
    for (size_t i = 0; i < p.token_count; ++i)
        p.tokens[i] = lexer_next_token(&l);

    return p;
}

static inline bool parser_expect(Parser *p, TokenKind kind)
{
    assert(p->pos < p->token_count);
    return p->tokens[p->pos].kind == kind;
}

static inline bool parser_at_eof(Parser *p)
{
    return parser_expect(p, TK_EOF);
}

static Expr *new_num_expr(Arena *a, Token t)
{
    Expr *e = arena_alloc(a, Expr);
    e->kind = EXPR_NUM;
    e->loc = t.loc;
    const char *num_as_str = strndup(t.start, t.len);
    e->val = atoi(num_as_str);
    return e;
}

static Expr *new_binop_expr(Arena *a, Loc loc, BinopKind kind, Expr *lhs, Expr *rhs)
{
    Expr *e = arena_alloc(a, Expr);
    e->kind = EXPR_BINOP;
    e->loc = loc;
    e->binop.kind = kind;
    e->binop.lhs = lhs;
    e->binop.rhs = rhs;
    return e;
}

static Expr *parse_expr(Parser *p);

// primary = "(" expr ")" | num
static Expr *parse_primary(Parser *p)
{
    Token t = p->tokens[p->pos];
    if (t.kind == TK_NUM) {
        p->pos++;
        return new_num_expr(p->a, t);
    }
    if (t.kind == TK_OPAREN) {
        p->pos++;
        if (parser_at_eof(p))
            diag_report_at_token(DIAG_ERROR, p->tokens[p->pos-1], "expected expression after (");
        return parse_expr(p);
    }
    diag_report_at_token(DIAG_ERROR, t, "unexpected expression");
    return NULL;
}

// mul = primary ("*" primary | "/" primary)*
static Expr *parse_mul(Parser *p)
{
    Expr *e = parse_primary(p);
    for (;;) {
        Token t = p->tokens[p->pos];
        if (t.kind == TK_STAR) {
            p->pos++;
            e = new_binop_expr(p->a, t.loc, BINOP_MULT, e, parse_primary(p));
            continue;
        }
        if (t.kind == TK_SLASH) {
            p->pos++;
            e = new_binop_expr(p->a, t.loc, BINOP_DIV, e, parse_primary(p));
            continue;
        }
        return e;
    }
}

// expr = mul ("+" mul | "-" mul)*
static Expr *parse_expr(Parser *p)
{
    Expr *e = parse_mul(p);
    for (;;) {
        Token t = p->tokens[p->pos];
        if (t.kind == TK_PLUS) {
            p->pos++;
            e = new_binop_expr(p->a, t.loc, BINOP_PLUS, e, parse_mul(p));
            continue;
        }
        if (t.kind == TK_MINUS) {
            p->pos++;
            e = new_binop_expr(p->a, t.loc, BINOP_MINUS, e, parse_mul(p));
            continue;
        }
        return e;
    }
}

const char *expr_kind_name[] = {
    [EXPR_NUM] = "num",
    [EXPR_BINOP] = "binop",
};

static void print_expr(Expr *e)
{
    switch (e->kind) {
    case EXPR_NUM:
        printf("{%s: %d}", expr_kind_name[e->kind], e->val);
        break;
    case EXPR_BINOP:
        printf("{%s: ", expr_kind_name[e->kind]);
        print_expr(e->binop.lhs);
        switch (e->binop.kind) {
        case BINOP_PLUS:
            printf(" + ");
            break;
        case BINOP_MINUS:
            printf(" - ");
            break;
        case BINOP_MULT:
            printf(" * ");
            break;
        case BINOP_DIV:
            printf(" / ");
            break;
        }
        print_expr(e->binop.rhs);
        printf("}");
        break;
    default:
        UNREACHABLE("print_expr");
    }
}

void parse_transl_unit(Parser *p)
{
    Expr *e = parse_expr(p);
    print_expr(e);
    printf("\n");
}
