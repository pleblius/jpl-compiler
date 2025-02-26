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
    COMMENT,
    // Meta tokens
    INVALID,
    ILLEGAL,
    CMD,
    EXPR,
    LVALUE,
    ARG,
    STMT,
    TYPE,
    BIND,
    STMTS,
    BINDS,
    STRUCTBINDS,
    EXPRS,
    LOOPBINDS,
    VARIABLES,
    COMMAS,
    END
} TokenType;

typedef struct {
    TokenType type;
    size_t loc;
    StringRef strref;
} Token;

Token create_token(TokenType, size_t, size_t, char*);
#endif // TOKEN_H
