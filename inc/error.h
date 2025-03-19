#ifndef ERROR_H
#define ERROR_H

#include "token.h"
#include "vecs.h"

typedef enum { LEX_ERROR, PARSE_ERROR, TYPE_ERROR } ErrorType;
typedef enum { INVALID_LEX, ILLEGAL_LEX, UNCLOSED_STRING } LexErrorType;
typedef enum { UNEXPECTED_TOKEN, BAD_CMD, BAD_EXPR, BAD_LVALUE, BAD_STMT, BAD_BIND, BAD_TYPE, BAD_UNARY, BAD_BINARY,
                UNCLOSED_PAREN, INT_RANGE, FLOAT_RANGE } ParseErrorType;
typedef enum { UNEXPECTED_TYPE, UNRESOLVED_TYPE, MISMATCHED_BINOP, MISMATCHED_IF, BAD_BINOP, BAD_UNOP, BAD_INDEX, BAD_DERFERENCE, BAD_MEMBER, 
                UNDECLARED_VARIABLE, SHADOWED_VARIABLE, NO_RETURN, BAD_RETURN, STRUCT_ASSIGN, VOID_ASSIGN, MISMATCHED_MEMBERS,
                EMPTY_ARRAY, MISMATCHED_ARRAY, NO_INDEX, BAD_RANK, BAD_FN, BAD_DIMENSION, BAD_SUM } TypeErrorType;

typedef struct {
    ErrorType error_type;
    
    union {
        LexErrorType lex_error;
        ParseErrorType parse_error;
        TypeErrorType type_error;
    } error_subtype;

    void *first_token;
    void *second_token;
} ErrorToken;

void error_setup(char *, char*);
void token_list_setup(TokenVec*);
void clear_errors();

void add_lex_error(LexErrorType, Token*);
void add_parse_error(ParseErrorType, Token*, Token*);
void add_type_error(TypeErrorType, AstNode*, AstNode*);

void print_errors();
void print_lex_error(ErrorToken*);
void print_parse_error(ErrorToken*);
void print_type_error(ErrorToken*);


void print_one_token_line(Token*);
void print_two_token_line(Token*, Token*);
void get_error_loc(Token*, uint32_t*, uint32_t*);

#endif // ERROR_H
