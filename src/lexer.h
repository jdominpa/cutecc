#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *file_path;
    size_t line;
    size_t col;
} Loc;

typedef enum {
    TK_INVALID,                 // invalid token
    TK_EOF,                     // end of file

    // Literals
    TK_IDENT,                   // identifier
    TK_NUM,                     // number

    // Punctuators
    TK_OPAREN,                  // (
    TK_CPAREN,                  // )
    TK_OBRACE,                  // {
    TK_CBRACE,                  // }
    TK_OBRACK,                  // [
    TK_CBRACK,                  // ]
    TK_SEMI,                    // ;
    TK_COLON,                   // :
    TK_DOT,                     // .
    TK_COMMA,                   // ,

    // Arithmetic operators
    TK_PLUS,                    // +
    TK_PLUS_PLUS,               // ++
    TK_MINUS,                   // -
    TK_MINUS_MINUS,             // --
    TK_STAR,                    // *
    TK_SLASH,                   // /
    TK_PERCENT,                 // %

    // Assignment operators
    TK_EQ,                      // =
    TK_PLUS_EQ,                 // +=
    TK_MINUS_EQ,                // -=
    TK_STAR_EQ,                 // *=
    TK_SLASH_EQ,                // /=
    TK_PERCENT_EQ,              // %=
    TK_AMP_EQ,                  // &=
    TK_PIPE_EQ,                 // |=
    TK_CARET_EQ,                // ^=
    TK_LT_LT_EQ,                // <<=
    TK_GT_GT_EQ,                // >>=

    // Bitwise operators
    TK_TILDE,                   // ~
    TK_AMP,                     // &
    TK_PIPE,                    // |
    TK_CARET,                   // ^
    TK_LT_LT,                   // <<
    TK_GT_GT,                   // >>

    // Logical operators
    TK_BANG,                    // !
    TK_AMP_AMP,                 // &&
    TK_PIPE_PIPE,               // ||

    // Relational operators
    TK_EQ_EQ,                   // ==
    TK_BANG_EQ,                 // !=
    TK_LT,                      // <
    TK_LT_EQ,                   // <=
    TK_GT,                      // >
    TK_GT_EQ,                   // >=

    // Structure dereference operator
    TK_MINUS_GT,                // ->

    // Ternary operator
    TK_QUESTION,                // ?

    // Number of token kinds
    TK_COUNT,
} TokenKind;

typedef struct {
    TokenKind kind;
    const char *pos;
    size_t len;
    Loc loc;
} Token;

typedef struct {
    const char *file_path;
    const char *source;
    size_t size;                // size of source
    size_t pos;                 // position
    size_t bol;                 // beginning of current line
    size_t line;                // current line number
} Lexer;

typedef enum {
    INFO,
    WARNING,
    ERROR,
} ReportLevel;

void lexer_report_at(ReportLevel level, Loc loc, const char *fmt, ...);
Lexer lexer_init(const char *file_path, const char *content, size_t size);
Token lexer_next_token(Lexer *l);
