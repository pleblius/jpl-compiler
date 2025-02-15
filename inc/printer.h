#include "token.h"
#include "vecs.h"
#include "vector.h"

void print_lexed_tokens(TokenVec *);
void buffer_token(Token);
void print_parsed_nodes(Vector *);

void print_cmd_node(Cmd*);
void print_fn_binds(Vector*);
void print_fn_stmts(Vector*);
void print_struct_binds(Cmd*);

void print_expr_node(Expr*);
void print_expr_list(Vector*);
void print_loop_binds(Vector*, Vector*);

void print_lvalue_node(LValue*);
void print_lvalue_vars(Vector*);

void print_type_node(Type*);

void print_stmt_node(Stmt*);

void print_bind_node(Bind*);
