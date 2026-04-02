#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *file_path;
    size_t row;
    size_t col;
} Loc;

typedef enum {
    TK_INVALID,
    TK_EOF,

    // Punctuators
    TK_OPAREN,
    TK_CPAREN,
    TK_OBRACE,
    TK_CBRACE,
    TK_SEMICOLON,

    // Identifier
    TK_IDENT,

    // Literals
    TK_NUM,

    // Number of token kinds
    TK_COUNT,
} TokenKind;

typedef struct {
    TokenKind kind;
    Loc loc;
    const char *pos;
    size_t len;
} Token;

typedef struct {
    const char *file_path;
    const char *content;
    size_t size;
    size_t cur;  // cursor position in `content`
    size_t row;  // current row
    size_t bol;  // beginning of line of current row
} Lexer;

typedef enum {
    INFO,
    WARNING,
    ERROR,
} ReportLevel;

void lexer_report_at(ReportLevel level, Loc loc, const char *fmt, ...);
Lexer lexer_init(const char *file_path, const char *content, size_t size);
Token lexer_next_token(Lexer *l);
