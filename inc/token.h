#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    ARRAY,
    ASSERT,
    BOOL,
    COLON,
    COMMA,
    DOT,
    ELSE,
    END_OF_FILE,
    EQUALS,
    FALSE,
    FLOAT,
    FLOATVAL,
    FN,
    IF,
    IMAGE,
    INT,
    INTVAL,
    LCURLY,
    LET,
    LPAREN,
    LSQUARE,
    NEWLINE,
    OP,
    PRINT,
    RCURLY,
    READ,
    RETURN,
    RPAREN,
    RSQUARE,
    SHOW,
    STRING,
    STRUCT,
    SUM,
    THEN,
    TIME,
    TO,
    TRUE,
    VARIABLE,
    VOID,
    WRITE,
    COMMENT,
    INVALID,
    ILLEGAL
} TokenType;

typedef struct {
    TokenType type;
    unsigned long byte;
    char *string;
} Token;

Token *create_token(TokenType, unsigned long, char*);
void print_token(Token*);
void free_token_string(Token*);

#endif // TOKEN_H
