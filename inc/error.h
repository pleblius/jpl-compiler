#ifndef ERROR_H
#define ERROR_H

#include "token.h"
#include "vector.h"
#include "vecs.h"

typedef enum { LEX_ERROR, PARSE_ERROR, TYPE_ERROR } ErrorType;

typedef enum {
    INVALID_ARG,
    INVALID_BIND,
    INVALID_CMD,
    INVALID_EXPR,
    INVALID_LVALUE,
    INVALID_STMT,
    INVALID_TYPE,
    UNEXPECTED_TOKEN,
    MISSING_TOKEN,
    INT_RANGE,
    FLOAT_RANGE,
    MISSING_BRACE,
    MISSING_PAREN,
    MISSING_BRACKET,
    MISSING_NEWLINE,
    MISSING_COLON,
    MISSING_STRING,
    MISSING_COMMA
} ParseErrorType;

typedef enum {
    INVALID_STRING,
    INVALID_ESCAPE,
    ILLEGAL_CHARACTER,
    INVALID_OPERATOR,
    INVALID_COMMENT
} LexErrorType;

typedef enum {
    SHADOWED_VAR,
    UNDECLARED_VAR,
    INVALID_ASSIGNMENT,
    MISMATCHED_OP,
} TypeErrorType;

typedef struct {
    size_t start;
    size_t error;
    ErrorType type;

    union {
        ParseErrorType parse_error;
        LexErrorType lex_error;
        TypeErrorType type_error;
    } subtype;

    char *misc_string;

} ErrorStruct;

void program_error(char *);

void parse_error(ParseErrorType, size_t, size_t, char*);
void lex_error(LexErrorType, size_t, size_t, char*);
void type_error(TypeErrorType, size_t, size_t);

void print_error_desc(char*, ErrorStruct*, TokenVec*, size_t, size_t);
void print_lex_error_desc(ErrorStruct*, TokenVec*);
void print_parse_error_desc(ErrorStruct*, TokenVec*);
void print_type_error_desc(ErrorStruct*, TokenVec*);

void print_error_line(char*, size_t, size_t, size_t, ErrorStruct*);
void get_loc(size_t, char*, size_t*, size_t*);

#endif // ERROR_H
