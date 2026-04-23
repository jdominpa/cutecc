#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "ast.h"
#include "common.h"
#include "diag.h"
#include "lexer.h"

// TODO: maybe this should be moved somewhere else
const char *token_kind_to_str[] = {
    [TK_INVALID] = "invalid",
    [TK_EOF] = "EOF",
    [TK_IDENT] = "ident",
    [TK_KW] = "keyword",
    [TK_CHAR] = "char",
    [TK_STR] = "str",
    [TK_NUM] = "number",
    [TK_OPAREN] = "(",
    [TK_CPAREN] = ")",
    [TK_OBRACE] = "{",
    [TK_CBRACE] = "}",
    [TK_OBRACK] = "[",
    [TK_CBRACK] = "]",
    [TK_SEMI] = ";",
    [TK_COLON] = ":",
    [TK_DOT] = ".",
    [TK_COMMA] = ",",
    [TK_PLUS] = "+",
    [TK_PLUS_PLUS] = "++",
    [TK_MINUS] = "-",
    [TK_MINUS_MINUS] = "--",
    [TK_STAR] = "*",
    [TK_SLASH] = "/",
    [TK_PERCENT] = "%",
    [TK_EQ] = "=",
    [TK_PLUS_EQ] = "+=",
    [TK_MINUS_EQ] = "-=",
    [TK_STAR_EQ] = "*=",
    [TK_SLASH_EQ] = "/=",
    [TK_PERCENT_EQ] = "%=",
    [TK_AMP_EQ] = "&=",
    [TK_PIPE_EQ] = "|=",
    [TK_CARET_EQ] = "^=",
    [TK_LT_LT_EQ] = "<<=",
    [TK_GT_GT_EQ] = ">>=",
    [TK_TILDE] = "~",
    [TK_AMP] = "&",
    [TK_PIPE] = "|",
    [TK_CARET] = "^",
    [TK_LT_LT] = "<<",
    [TK_GT_GT] = ">>",
    [TK_BANG] = "!",
    [TK_AMP_AMP] = "&&",
    [TK_PIPE_PIPE] = "||",
    [TK_EQ_EQ] = "==",
    [TK_BANG_EQ] = "!=",
    [TK_LT] = "<",
    [TK_LT_EQ] = "<=",
    [TK_GT] = ">",
    [TK_GT_EQ] = ">=",
    [TK_MINUS_GT] = "->",
    [TK_QUESTION] = "?",
};

// Checks if the current token is of TokenKind `kind`, and returns `true` if so.
static inline bool parser_check(Parser *p, TokenKind kind)
{
    assert(p->pos < p->token_count);
    return p->tokens[p->pos].kind == kind;
}

// Returns whether the parser is at EOF or not.
static inline bool parser_at_eof(Parser *p)
{
    return parser_check(p, TK_EOF);
}

// Advance the parser by one token.
static inline void parser_bump(Parser *p)
{
    if (!parser_at_eof(p))
        p->pos++;
}

// Consume token of TokenKind `kind` if present.
// Return whether the given token was present.
static bool parser_eat(Parser *p, TokenKind kind)
{
    if (parser_check(p, kind)) {
        parser_bump(p);
        return true;
    }
    return false;
}

// Expects and consumes a token of TokenKind `kind`.
// Raises an error if the current token is not of type `kind`.
static bool parser_expect(Parser *p, TokenKind kind)
{
    if (parser_check(p, kind)) {
        parser_bump(p);
        return true;
    } else {
        Token t = p->tokens[p->pos];
        // TODO: try to recover from unexpected tokens instead of crashing
        diag_fatal_at(t.loc, "unexpected token, expected `%s` but found `%s`",
                      token_kind_to_str[kind], token_kind_to_str[t.kind]);
    }
}

static Expr *new_unop_expr(Arena *a, Loc loc, UnopKind kind, Expr *operand)
{
    Expr *e = arena_alloc(a, Expr);
    e->kind = EXPR_UNOP;
    e->loc = loc;
    e->unop.kind = kind;
    e->unop.operand = operand;
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

static Expr *new_assign_expr(Arena *a, Loc loc, AssignKind kind, Expr *var, Expr *value)
{
    Expr *e = arena_alloc(a, Expr);
    e->kind = EXPR_ASSIGN;
    e->loc = loc;
    e->assign.kind = kind;
    e->assign.var = var;
    e->assign.value = value;
    return e;
}

static BinopKind get_binop_kind(Token t)
{
    switch (t.kind) {
    case TK_PIPE_PIPE: return BINOP_OR;
    case TK_AMP_AMP:   return BINOP_AND;
    case TK_PIPE:      return BINOP_BIT_OR;
    case TK_CARET:     return BINOP_BIT_XOR;
    case TK_AMP:       return BINOP_BIT_AND;
    case TK_EQ_EQ:     return BINOP_EQ;
    case TK_BANG_EQ:   return BINOP_NOT_EQ;
    case TK_LT:        return BINOP_LT;
    case TK_LT_EQ:     return BINOP_LT_EQ;
    case TK_GT:        return BINOP_GT;
    case TK_GT_EQ:     return BINOP_GT_EQ;
    case TK_LT_LT:     return BINOP_LSFT;
    case TK_GT_GT:     return BINOP_RSFT;
    case TK_PLUS:      return BINOP_PLUS;
    case TK_MINUS:     return BINOP_MINUS;
    case TK_STAR:      return BINOP_MULT;
    case TK_SLASH:     return BINOP_DIV;
    case TK_PERCENT:   return BINOP_MOD;
    default:
        UNREACHABLE("get_binop_kind called with non-binop token");
    }
}

static AssignKind get_assign_kind(Token t)
{
    switch (t.kind) {
    case TK_AMP_EQ:     return ASSIGN_AND;
    case TK_CARET_EQ:   return ASSIGN_XOR;
    case TK_PIPE_EQ:    return ASSIGN_OR;
    case TK_LT_LT_EQ:   return ASSIGN_LSFT;
    case TK_GT_GT_EQ:   return ASSIGN_RSFT;
    case TK_STAR_EQ:    return ASSIGN_MULT;
    case TK_SLASH_EQ:   return ASSIGN_DIV;
    case TK_PERCENT_EQ: return ASSIGN_MOD;
    case TK_PLUS_EQ:    return ASSIGN_PLUS;
    case TK_MINUS_EQ:   return ASSIGN_MINUS;
    case TK_EQ:         return ASSIGN_EQ;
    default:
        UNREACHABLE("get_assign_kind called with non-assingment token");
    }
}

static bool is_assign_op(TokenKind kind)
{
    switch (kind) {
    case TK_AMP_EQ:   case TK_CARET_EQ: case TK_PIPE_EQ:
    case TK_LT_LT_EQ: case TK_GT_GT_EQ:
    case TK_STAR_EQ:  case TK_SLASH_EQ: case TK_PERCENT_EQ:
    case TK_PLUS_EQ:  case TK_MINUS_EQ:
    case TK_EQ:
        return true;
    default:
        return false;
    }
}

static bool try_get_op_bp(Token t, BindPower *bp)
{
    switch (t.kind) {
    case TK_COMMA:                                          // ","
        bp->left = 1; bp->right = 2;
        return true;
    case TK_AMP_EQ:   case TK_CARET_EQ: case TK_PIPE_EQ:    // "&=", "^=", "|="
    case TK_LT_LT_EQ: case TK_GT_GT_EQ:                     // "<<=", ">>="
    case TK_STAR_EQ:  case TK_SLASH_EQ: case TK_PERCENT_EQ: // "*=", "/=", "%="
    case TK_PLUS_EQ:  case TK_MINUS_EQ:                     // "+=", "-="
    case TK_EQ:                                             // "="
        bp->left = 3; bp->right = 2;
        return true;
    case TK_QUESTION:                                       // "?" ":"
        bp->left = 4; bp->right = 3;
        return true;
    case TK_PIPE_PIPE:                                      // "||"
        bp->left = 4; bp->right = 5;
        return true;
    case TK_AMP_AMP:                                        // "&&"
        bp->left = 5; bp->right = 6;
        return true;
    case TK_PIPE:                                           // "|"
        bp->left = 6; bp->right = 7;
        return true;
    case TK_CARET:                                          // "^"
        bp->left = 7; bp->right = 8;
        return true;
    case TK_AMP:                                            // "&"
        bp->left = 8; bp->right = 9;
        return true;
    case TK_EQ_EQ: case TK_BANG_EQ:                         // "==", "!="
        bp->left = 9; bp->right = 10;
        return true;
    case TK_LT: case TK_LT_EQ:                              // "<", "<="
    case TK_GT: case TK_GT_EQ:                              // ">", ">="
        bp->left = 10; bp->right = 11;
        return true;
    case TK_LT_LT: case TK_GT_GT:                           // "<<", ">>"
        bp->left = 11; bp->right = 12;
        return true;
    case TK_PLUS: case TK_MINUS:                            // "+", "-"
        bp->left = 11; bp->right = 12;
        return true;
    case TK_STAR: case TK_SLASH: case TK_PERCENT:           // "*", "/", "%"
        bp->left = 12; bp->right = 13;
        return true;
    // NOTE: `get_prefix_op_bp` needs to be updated if the highest postfix
    // operator changes
    case TK_DOT: case TK_MINUS_GT:                          // ".", "->"
    case TK_OPAREN: case TK_OBRACK:                         // "(", "["
    case TK_PLUS_PLUS: case TK_MINUS_MINUS:                 // "++", "--"
        bp->left = 13; bp->right = 14;
        return true;
    default:
        return false;
    }
}

static uint8_t get_prefix_op_bp(void)
{
    BindPower bp;
    // The prefix left binding power equals the right binding power of the
    // tightest postfix operator (i.e. *p++ == *(p++)).
    if (!try_get_op_bp((Token) { .kind = TK_MINUS_MINUS }, &bp))
        UNREACHABLE("get_prefix_bp called with non-operation token");
    return bp.left;
}

static Expr *parse_expr(Parser *p);
static Expr *parse_expr_bp(Parser *p, uint8_t min_bp);

/*
  expr_head = "(" expr ")"
            | num
*/
static Expr *parse_expr_head(Parser *p)
{
    Token t = p->tokens[p->pos];
    parser_bump(p);
    switch (t.kind) {
    // TODO: parse character literals
    case TK_STR: {
        Expr *e = arena_alloc(p->a, Expr);
        e->kind = EXPR_STR;
        e->loc = t.loc;
        e->str = strndup(t.start, t.len);
        return e;
    }
    case TK_NUM: {
        Expr *e = arena_alloc(p->a, Expr);
        e->kind = EXPR_NUM;
        e->loc = t.loc;
        e->val = 0;
        for (size_t i = 0; i < t.len; ++i)
            e->val = e->val * 10 + (t.start[i] - '0');
        return e;
    }
    case TK_IDENT: {
        Expr *e = arena_alloc(p->a, Expr);
        e->kind = EXPR_VAR;
        e->loc = t.loc;
        e->var = strndup(t.start, t.len);
        return e;
    }
    case TK_OPAREN: {
        Expr *e = parse_expr(p);
        if (!parser_expect(p, TK_CPAREN))
            UNREACHABLE("parser_expect is nonreturnable");
        return e;
    }
    case TK_PLUS:
        return new_unop_expr(p->a, t.loc, UNOP_POS,
                             parse_expr_bp(p, get_prefix_op_bp()));
    case TK_MINUS:
        return new_unop_expr(p->a, t.loc, UNOP_NEG,
                             parse_expr_bp(p, get_prefix_op_bp()));
    case TK_BANG:
        return new_unop_expr(p->a, t.loc, UNOP_NOT,
                             parse_expr_bp(p, get_prefix_op_bp()));
    case TK_TILDE:
        return new_unop_expr(p->a, t.loc, UNOP_BIT_NOT,
                             parse_expr_bp(p, get_prefix_op_bp()));
    case TK_STAR:
        return new_unop_expr(p->a, t.loc, UNOP_DEREF,
                             parse_expr_bp(p, get_prefix_op_bp()));
    case TK_AMP:
        return new_unop_expr(p->a, t.loc, UNOP_ADDR,
                             parse_expr_bp(p, get_prefix_op_bp()));
    case TK_PLUS_PLUS:
        return new_unop_expr(p->a, t.loc, UNOP_PRE_INC,
                             parse_expr_bp(p, get_prefix_op_bp()));
    case TK_MINUS_MINUS:
        return new_unop_expr(p->a, t.loc, UNOP_PRE_DEC,
                             parse_expr_bp(p, get_prefix_op_bp()));
    default:
        // TODO: return EXPR_ERROR and try to recover from unexpected expression
        diag_fatal_at(t.loc, "unexpected expression `%.*s`", t.len, t.start);
    }
}

// expr      = expr_head expr_tail*
// expr_tail = tail_op expr
static Expr *parse_expr_bp(Parser *p, uint8_t min_bp)
{
    Expr *e = parse_expr_head(p);
    while (!parser_at_eof(p)) {
        Token op = p->tokens[p->pos];

        // Operation precedence check
        BindPower op_bp;
        if (!try_get_op_bp(op, &op_bp))
            break;
        if (op_bp.left < min_bp)
            break;

        // Struct or struct pointer field access
        if (parser_eat(p, TK_DOT) || parser_eat(p, TK_MINUS_GT)) {
            Token field = p->tokens[p->pos];
            if (!parser_expect(p, TK_IDENT))
                UNREACHABLE("parser_expect is nonreturnable");
            Expr *obj = e;
            e = arena_alloc(p->a, Expr);
            e->kind = op.kind == TK_DOT ? EXPR_FIELD : EXPR_ARROW;
            e->field.field = strndup(field.start, field.len);
            e->field.obj = obj;
            continue;
        }

        // Array element access
        if (parser_eat(p, TK_OBRACK)) {
            Expr *array = e;
            e = arena_alloc(p->a, Expr);
            e->kind = EXPR_INDEX;
            e->index.array = array;
            e->index.index = parse_expr(p);
            if (!parser_expect(p, TK_CBRACK))
                UNREACHABLE("parse_expect is nonreturnable");
            continue;
        }

        // Postfix increment/decrement
        if (parser_eat(p, TK_PLUS_PLUS) || parser_eat(p, TK_MINUS_MINUS)) {
            e = new_unop_expr(p->a, op.loc,
                              op.kind == TK_PLUS_PLUS ? UNOP_POST_INC : UNOP_POST_DEC, e);
            continue;
        }

        parser_bump(p);
        if (is_assign_op(op.kind))
            e = new_assign_expr(p->a, op.loc, get_assign_kind(op),
                                e, parse_expr_bp(p, op_bp.right));
        else
            e = new_binop_expr(p->a, op.loc, get_binop_kind(op),
                               e, parse_expr_bp(p, op_bp.right));
    }
    return e;
}

static Expr *parse_expr(Parser *p)
{
    return parse_expr_bp(p, 0);
}

static void print_expr_as_sexp(Expr *e)
{
    switch (e->kind) {
    case EXPR_CHAR:
        printf("%c", e->c);
        break;
    case EXPR_STR:
        printf("%s", e->str);
        break;
    case EXPR_NUM:
        printf("%d", e->val);
        break;
    case EXPR_VAR:
        printf("%s", e->var);
        break;
    case EXPR_UNOP:
        switch (e->unop.kind) {
        case UNOP_POS:
            printf("+");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_NEG:
            printf("-");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_NOT:
            printf("!");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_BIT_NOT:
            printf("~");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_DEREF:
            printf("*");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_ADDR:
            printf("&");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_PRE_INC:
            printf("++");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_PRE_DEC:
            printf("--");
            print_expr_as_sexp(e->unop.operand);
            break;
        case UNOP_POST_INC:
            print_expr_as_sexp(e->unop.operand);
            printf("++");
            break;
        case UNOP_POST_DEC:
            print_expr_as_sexp(e->unop.operand);
            printf("--");
            break;
        default:
            UNREACHABLE("e->unop.kind at print_expr_as_sexp");
        }
        break;
    case EXPR_BINOP:
        switch (e->binop.kind) {
        case BINOP_OR:
            printf("(|| ");
            break;
        case BINOP_AND:
            printf("(&& ");
            break;
        case BINOP_BIT_OR:
            printf("(| ");
            break;
        case BINOP_BIT_XOR:
            printf("(^ ");
            break;
        case BINOP_BIT_AND:
            printf("(& ");
            break;
        case BINOP_EQ:
            printf("(== ");
            break;
        case BINOP_NOT_EQ:
            printf("(!= ");
            break;
        case BINOP_LT:
            printf("(< ");
            break;
        case BINOP_LT_EQ:
            printf("(<= ");
            break;
        case BINOP_GT:
            printf("(> ");
            break;
        case BINOP_GT_EQ:
            printf("(>= ");
            break;
        case BINOP_LSFT:
            printf("(<< ");
            break;
        case BINOP_RSFT:
            printf("(>> ");
            break;
        case BINOP_PLUS:
            printf("(+ ");
            break;
        case BINOP_MINUS:
            printf("(- ");
            break;
        case BINOP_MULT:
            printf("(* ");
            break;
        case BINOP_DIV:
            printf("(/ ");
            break;
        case BINOP_MOD:
            printf("(%% ");
            break;
        default:
            UNREACHABLE("e->binop.kind at print_expr_as_sexp");
        }
        print_expr_as_sexp(e->binop.lhs);
        printf(" ");
        print_expr_as_sexp(e->binop.rhs);
        printf(")");
        break;
    case EXPR_ASSIGN:
        printf("(let ");
        print_expr_as_sexp(e->assign.var);
        printf(" ");
        switch (e->assign.kind) {
        case ASSIGN_AND:
            printf("(& ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_XOR:
            printf("(^ ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_OR:
            printf("(| ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_LSFT:
            printf("(<< ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_RSFT:
            printf("(>> ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_MULT:
            printf("(* ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_DIV:
            printf("(/ ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_MOD:
            printf("(%% ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_PLUS:
            printf("(+ ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_MINUS:
            printf("(- ");
            print_expr_as_sexp(e->assign.var);
            printf(" ");
            print_expr_as_sexp(e->assign.value);
            printf(")");
            break;
        case ASSIGN_EQ:
            print_expr_as_sexp(e->assign.value);
            break;
        default:
            UNREACHABLE("e->assign.kind at print_expr_as_sexp");
        }
        printf(")");
        break;
    case EXPR_INDEX:
        print_expr_as_sexp(e->index.array);
        printf("\[");
        print_expr_as_sexp(e->index.index);
        printf("]");
        break;
    case EXPR_FIELD:
        print_expr_as_sexp(e->field.obj);
        printf(".%s", e->field.field);
        break;
    case EXPR_ARROW:
        print_expr_as_sexp(e->field.obj);
        printf("->%s", e->field.field);
        break;
    default:
        UNREACHABLE("print_expr_as_sexp");
    }
}

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

void parse_transl_unit(Parser *p)
{
    Expr *e = parse_expr(p);
    print_expr_as_sexp(e);
    printf("\n");
}
