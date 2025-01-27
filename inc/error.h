#ifndef ERROR_H
#define ERROR_H

#include "token.h"

typedef enum { INVALID_COMMAND, INVALID_TOKEN, INVALID_ARG, INVALID_EXPRESSION, MISSING_NUMBER, MISSING_STRING, OUT_OF_RANGE } ParseError;

void parse_error(ParseError, TokenType, Token*);
void parse_missing_number(TokenType, Token*);
void parse_missing_string(TokenType, Token*);
void parse_token_error(TokenType, Token*);
void parse_range_error(Token*);
void parse_arg_error();
void parse_cmd_error();
void print_error_line(unsigned long, unsigned long, Token*);

unsigned long get_col(unsigned long);
unsigned long get_line(unsigned long);

#endif // ERROR_H
