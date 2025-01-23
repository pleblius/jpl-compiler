#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "ast.h"
#include "vector.h"

uint64_t parse_expr(uint64_t, ASTNode*);
uint64_t parse_cmd(uint64_t, ASTNode*);
#endif // PARSER_H
