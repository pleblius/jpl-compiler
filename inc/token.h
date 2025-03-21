#ifndef TOKEN_H
#define TOKEN_H

#include "stringops.h"

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
    // Meta tokens
    INVALID,
    ILLEGAL,
    UNCLOSED,
    COMMENT,
} TokenType;

typedef struct {
    TokenType type;
    size_t loc;
    StringRef strref;
} Token;

Token create_token(TokenType, size_t, size_t, char*);
#endif // TOKEN_H
