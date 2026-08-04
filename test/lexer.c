#include "../src/lexer.h"
#include <string.h>

#include "test.h"

// Takes the next token of the lexer `l` and checks whether its TokenKind is
// `kind` and its content is `token_text`.
static void check_next_token(Lexer *l, TokenKind kind, const char *token_text)
{
    Token t = lexer_next_token(l);
    ASSERT(t.kind == kind,
           "expected TokenKind `%s` but got `%s` (%.*s at <%zu:%zu>)",
           token_kind_to_str[kind], token_kind_to_str[t.kind], (int) t.len,
           t.start, t.loc.line, t.loc.col);
    if (token_text != NULL)
        EXPECT(t.len == strlen(token_text) && strncmp(t.start, token_text, t.len) == 0,
               "expected token content `%s` but got `%.*s`", token_text,
               (int) t.len, t.start);
}

//
// Punctuators
//

DEFINE_TEST(test_whitespace)
{
    Lexer l = lexer_init_from_src("   \n\n   ");
    check_next_token(&l, TK_EOF, NULL);
}

DEFINE_TEST(test_comments)
{
    Lexer l = lexer_init_from_src("/* Test\nblock\n*comment */\n\n// Test inline comment\n// Test inline /* nested comment */");
    check_next_token(&l, TK_EOF, NULL);
}

DEFINE_TEST(test_identifiers_and_keywords)
{
    Lexer l = lexer_init_from_src("int x y _if enum extern");
    check_next_token(&l, TK_KW, "int");
    check_next_token(&l, TK_IDENT, "x");
    check_next_token(&l, TK_IDENT, "y");
    check_next_token(&l, TK_IDENT, "_if");
    check_next_token(&l, TK_KW, "enum");
    check_next_token(&l, TK_KW, "extern");
}

DEFINE_TEST(test_single_char_punctuators)
{
    Lexer l = lexer_init_from_src("(){}[];:,.");
    check_next_token(&l, TK_OPAREN, "(");
    check_next_token(&l, TK_CPAREN, ")");
    check_next_token(&l, TK_OBRACE, "{");
    check_next_token(&l, TK_CBRACE, "}");
    check_next_token(&l, TK_OBRACK, "[");
    check_next_token(&l, TK_CBRACK, "]");
    check_next_token(&l, TK_SEMI, ";");
    check_next_token(&l, TK_COLON, ":");
    check_next_token(&l, TK_COMMA, ",");
    check_next_token(&l, TK_DOT, ".");
    check_next_token(&l, TK_EOF, NULL);
}

int main(void)
{
    RUN_TEST(test_whitespace);
    RUN_TEST(test_comments);
    RUN_TEST(test_identifiers_and_keywords);
    RUN_TEST(test_single_char_punctuators);
    TEST_SUMMARY();
    return 0;
}
