#ifndef TYPECHECK_H
#define TYPECHECK_H

#include <stdint.h>

#include "astnode.h"
#include "vector.h"
#include "vecs.h"
#include "dict.h"
#include "error.h"

void generate_predefs(NodeVec*);
int type_check(TokenVec*, NodeVec*, Vector*);

int type_check_cmd(uint32_t);
int type_check_read_cmd(uint32_t);
int type_check_write_cmd(uint32_t);
int type_check_let_cmd(uint32_t);
int type_check_fn_cmd(uint32_t);
int type_check_struct_cmd(uint32_t);

int type_check_statement(uint32_t);
int type_check_lvalue(uint32_t);

int type_check_expr(uint32_t);
int type_check_var_expr(uint32_t);
int type_check_arraylit_expr(uint32_t);
int type_check_structlit_expr(uint32_t);
int type_check_dot_expr(uint32_t);
int type_check_arrayindex_expr(uint32_t);
int type_check_call_expr(uint32_t);
int type_check_unop_expr(uint32_t);
int type_check_binop_expr(uint32_t);
int type_check_if_expr(uint32_t);
int type_check_loop_expr(uint32_t);

int compare_expr_types(uint32_t, uint32_t);
int expect_expr_type(TypeType, uint32_t);
int compare_types(uint32_t, uint32_t);
int expect_type(TypeType, uint32_t);
void type_error(TypeErrorType, uint32_t, uint32_t);

void remove_lvalue(uint32_t);

#endif // TYPECHECK_H
