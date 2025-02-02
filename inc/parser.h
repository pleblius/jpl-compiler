#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "vector.h"
#include "token.h"

int parse();

TokenType peek_token(uint64_t);
int expect_token(uint64_t, TokenType, char**);

void parse_print_output();

void free_command_list();

#endif // PARSER_H
