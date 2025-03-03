#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "error.h"
#include "vecs.h"
#include "token.h"
#include "stringops.h"
#include "vector.h"

int parse_tokens(TokenVec*, NodeVec**, Vector**);
int expect_token(TokenType, uint32_t, StringRef*);
TokenType peek_token_type(uint32_t);
int try_find_next(uint32_t*, TokenType, TokenType);

void parse_error(ParseErrorType, uint32_t);

int parse_command(uint32_t*, uint64_t*);
int parse_lvalue(uint32_t*, uint64_t*);
int parse_expression(uint32_t*, uint64_t*, int);
int parse_expression_literal(uint32_t*, uint64_t*);
int parse_binding(uint32_t*, uint64_t*);
int parse_type(uint32_t*, uint64_t*);
int parse_statement(uint32_t*, uint64_t*);
int is_binary_operator(uint32_t);
int get_operator_precedence(StringRef);
#endif // PARSER_H
