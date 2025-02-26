#ifndef ERROR_H
#define ERROR_H

#include "token.h"

typedef enum { LEX_ERROR, PARSE_ERROR, TYPE_ERROR } ErrorType;
typedef enum { INVALID_LEX, ILLEGAL_LEX } LexErrorType;
typedef enum { UNEXPECTED_TOKEN, BAD_CMD, BAD_EXPR, BAD_LVALUE, BAD_STMT, BAD_ARG, BAD_TYPE, UNCLOSED_PAREN } ParseErrorType;
typedef enum { MISMATCHED_BINOP, BAD_INDEX, BAD_DERFERENCE, UNDECLARED_VARIABLE, SHADOWED_VARIABLE, NO_RETURN, BAD_RETURN, BAD_ASSIGN } TypeErrorType;

typedef struct {
    ErrorType error_type;
    
    union {
        LexErrorType lex_error;
        ParseErrorType parse_error;
        TypeErrorType type_error;
    } error_subtype;

    Token *first_token;
    Token *second_token;
} ErrorToken;

void error_setup(char *, char*);
void clear_errors();

void add_lex_error(LexErrorType, Token*);
void add_parse_error(ParseErrorType, Token*, Token*);
void add_type_error(TypeErrorType, Token*, Token*);

void print_errors(TokenVec*);
void print_lex_error(ErrorToken*, TokenVec*);
void print_parse_error(ErrorToken*, TokenVec*);
void print_type_error(ErrorToken*, TokenVec*);


void print_one_token_line(Token*);
void print_two_token_line(Token*, Token*);
void get_error_loc(Token*, uint32_t*, uint32_t*);

#endif // ERROR_H
