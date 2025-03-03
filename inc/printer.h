#ifndef PRINTER_H
#define PRINTER_H

#include "vecs.h"
#include "vector.h"

void print_tokens(TokenVec*);
void print_nodes(NodeVec*, Vector*, TokenVec*);
void set_type_check();

void print_command(AstNode *);
void print_expression(AstNode *);
void print_lvalue(AstNode *);
void print_binding(AstNode *);
void print_type(AstNode *);
void print_statement(AstNode *);

#endif // PRINTER_H
