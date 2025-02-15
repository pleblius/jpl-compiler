#include <stdio.h>
#include <stdlib.h>

#include "printer.h"
#include "compiler.h"

extern CVec *print_buffer;

const char* token_strings[] = { "ARRAY", "ASSERT", "BOOL", "COLON", "COMMA", "DOT", "ELSE", "END_OF_FILE",
    "EQUALS", "FALSE", "FLOAT", "FLOATVAL", "FN", "IF", "IMAGE", "INT", "INTVAL", "LCURLY", "LET",
    "LPAREN", "LSQUARE", "NEWLINE", "OP", "PRINT", "RCURLY", "READ", "RETURN", "RPAREN", "RSQUARE",
    "SHOW", "STRING", "STRUCT", "SUM", "THEN", "TIME", "TO", "TRUE", "VARIABLE", "VOID", "WRITE" };

void print_lexed_tokens(TokenVec *vector) {
    if (!vector) return;
    cvec_clear(print_buffer);

    for (size_t i = 0; i < tokenvec_size(vector); ++i) {
        buffer_token(tokenvec_get(vector, i));
    }

    cvec_print(print_buffer);
}


void buffer_token(Token token) {
    cvec_append_array(print_buffer, token_strings[token.type], strlen(token_strings[token.type]));

    if (token.type != NEWLINE && token.type != END_OF_FILE) {
        cvec_append_array(print_buffer, " '", 2);
        cvec_append_ref(print_buffer, token.strref);
        cvec_append(print_buffer, '\'');
    }

    cvec_append(print_buffer, '\n');
}

void print_parsed_nodes(Vector *vector) {
    if (!vector) return;
    cvec_clear(print_buffer);

    for (size_t i = 0; i < vector_size(vector); ++i) {
        print_cmd_node(vector_get(vector, i));
        cvec_append(print_buffer, '\n');
    }

    cvec_print(print_buffer);    
}

void print_cmd_node(Cmd *node) {
    switch (node->type) {
        case READ_CMD:
            cvec_append_array(print_buffer, "(ReadCmd ", 9);
            cvec_append_ref(print_buffer, node->field1.string);
            cvec_append(print_buffer, ' ');
            print_lvalue_node(node->field2.lvalue);
            cvec_append(print_buffer, ')');
            break;
        case WRITE_CMD:
            cvec_append_array(print_buffer, "(WriteCmd ", 10);
            print_expr_node(node->field1.expr);
            cvec_append(print_buffer, ' ');
            cvec_append_ref(print_buffer, node->field2.string);
            cvec_append(print_buffer, ')');
            break;
        case LET_CMD:
            cvec_append_array(print_buffer, "(LetCmd ", 8);
            print_lvalue_node(node->field1.lvalue);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field2.expr);
            cvec_append(print_buffer, ')');
            break;
        case ASSERT_CMD:
            cvec_append_array(print_buffer, "(AssertCmd ", 11);
            print_expr_node(node->field1.expr);
            cvec_append(print_buffer, ' ');
            cvec_append_ref(print_buffer, node->field2.string);
            cvec_append(print_buffer, ')');
            break;
        case PRINT_CMD:
            cvec_append_array(print_buffer, "(PrintCmd ", 10);
            cvec_append_ref(print_buffer, node->field1.string);
            cvec_append(print_buffer, ')');
            break;
        case SHOW_CMD:
            cvec_append_array(print_buffer, "(ShowCmd ", 9);
            print_expr_node(node->field1.expr);
            cvec_append(print_buffer, ')');
            break;
        case TIME_CMD:
            cvec_append_array(print_buffer, "(TimeCmd ", 9);
            print_cmd_node(node->field1.cmd);
            cvec_append(print_buffer, ')');
            break;
        case FN_CMD:
            cvec_append_array(print_buffer, "(FnCmd ", 7);
            cvec_append_ref(print_buffer, node->field1.string);
            cvec_append_array(print_buffer, " (", 2);
            print_fn_binds(node->field2.binds);
            cvec_append_array(print_buffer, ") ", 2);
            print_type_node(node->field3.type);
            print_fn_stmts(node->field4.stmts);
            cvec_append(print_buffer, ')');
            break;
        case STRUCT_CMD:
            cvec_append_array(print_buffer, "(StructCmd ", 11);
            cvec_append_ref(print_buffer, node->field1.string);
            print_struct_binds(node);
            cvec_append(print_buffer, ')');
            break;
        default:
            return;
    }
}
void print_fn_binds(Vector *vector) {
    cvec_append(print_buffer, '(');
    size_t size = vector->size;
    if (size != 0) print_bind_node(vector_get(vector,0));

    for (size_t i = 1; i < size; ++i) {
        cvec_append(print_buffer, ' ');
        print_bind_node(vector_get(vector, i));
    }
    cvec_append(print_buffer, ')');
}
void print_fn_stmts(Vector *vector) {
    if (vector_is_empty(vector)) return;
    size_t size = vector->size;
    for (size_t i = 0; i < size; ++i) {
        cvec_append(print_buffer, ' ');
        print_stmt_node(vector_get(vector, i));
    }
}
void print_struct_binds(Cmd *node) {
    Vector *vars = node->field2.vars;
    Vector *types = node->field3.types;
    char *variable = NULL;
    if (vector_is_empty(vars)) return;

    size_t size = vars->size;
    for (size_t i = 0; i < size; ++i) {
        cvec_append(print_buffer, ' ');
        variable = vector_get(vars, i);
        cvec_append_array(print_buffer, variable, strlen(variable));
        cvec_append(print_buffer, ' ');
        print_type_node(vector_get(types, i));
    }
}

void print_expr_node(Expr *node) {
    char *string;
    switch (node->type) {
        case INT_EXPR:
            cvec_append_array(print_buffer, "(IntExpr ", 9);
            string = malloc(MAXIMUM_BUFFER); if (!string) return;
            if (snprintf(string, MAXIMUM_BUFFER, "%ld", node->field1.int_value) < 0) return;
            cvec_append_array(print_buffer, string, strlen(string));
            free(string);
            break;
        case FLOAT_EXPR:
            cvec_append_array(print_buffer, "(FloatExpr ", 11);
            string = malloc(MAXIMUM_BUFFER); if (!string) return;
            if (snprintf(string, MAXIMUM_BUFFER, "%ld", (uint64_t) node->field1.float_value) < 0) return;
            cvec_append_array(print_buffer, string, strlen(string));
            free(string);
            break;
        case TRUE_EXPR:
            cvec_append_array(print_buffer, "(TrueExpr", 9);
            break;
        case FALSE_EXPR:
            cvec_append_array(print_buffer, "(FalseExpr", 10);
            break;
        case VOID_EXPR:
            cvec_append_array(print_buffer, "(VoidExpr", 9);
            break;
        case VAR_EXPR:
            cvec_append_array(print_buffer, "(VarExpr ", 9);
            cvec_append_ref(print_buffer, node->field1.string);
            break;
        case ARRAY_EXPR:
            cvec_append_array(print_buffer, "(ArrayLiteralExpr", 17);
            print_expr_list(node->field1.expr_list);
            break;
        case STRUCTLITERAL_EXPR:
            cvec_append_array(print_buffer, "(StructLiteralExpr ", 19);
            cvec_append_ref(print_buffer, node->field1.string);
            print_expr_list(node->field2.expr_list);
            break;
        case DOT_EXPR:
            cvec_append_array(print_buffer, "(DotExpr ", 9);
            print_expr_node(node->field1.expr);
            cvec_append(print_buffer, ' ');
            cvec_append_ref(print_buffer, node->field2.string);
            break;
        case ARRAYINDEX_EXPR:
            cvec_append_array(print_buffer, "(ArrayIndexExpr ", 16);
            print_expr_node(node->field1.expr);
            print_expr_list(node->field2.expr_list);
            break;
        case CALL_EXPR:
            cvec_append_array(print_buffer, "(CallExpr ", 10);
            cvec_append_ref(print_buffer, node->field1.string);
            print_expr_list(node->field2.expr_list);
            break;
        case UNOP_EXPR:
            cvec_append_array(print_buffer, "(UnopExpr ", 10);
            cvec_append_ref(print_buffer, node->field1.string);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field2.expr);
            break;
        case BINOP_EXPR:
            cvec_append_array(print_buffer, "(BinopExpr ", 11);
            print_expr_node(node->field2.expr);
            cvec_append(print_buffer, ' ');
            cvec_append_ref(print_buffer, node->field1.string);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field3.expr);
            break;
        case IF_EXPR:
            cvec_append_array(print_buffer, "(IfExpr ", 8);
            print_expr_node(node->field1.expr);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field2.expr);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field3.expr);
            break;
        case ARRAY_LOOP_EXPR:
            cvec_append_array(print_buffer, "(ArrayLoopExpr", 14);
            print_loop_binds(node->field1.var_list, node->field2.expr_list);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field3.expr);
            break;
        case SUM_LOOP_EXPR:
            cvec_append_array(print_buffer, "(SumLoopExpr", 12);
            print_loop_binds(node->field1.var_list, node->field2.expr_list);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field3.expr);
            break;
        default:
            return;
    }

    cvec_append(print_buffer, ')');
}
void print_expr_list(Vector *vector) {
    if (vector_is_empty(vector)) return;
    size_t size = vector->size;

    for (size_t i = 0; i < size; ++i) {
        cvec_append(print_buffer, ' ');
        print_expr_node(vector_get(vector, i));
    }
}
void print_loop_binds(Vector *vars, Vector *exprs) {
    if (vector_is_empty(vars)) return;

    size_t size = vars->size;
    char *string;
    for (size_t i = 0; i < size; ++i) {
        string = vector_get(vars, i);
        cvec_append(print_buffer, ' ');
        cvec_append_array(print_buffer, string, strlen(string));
        cvec_append(print_buffer, ' ');
        print_expr_node(vector_get(exprs, i));
    }
}

void print_lvalue_node(LValue *node) {
    switch (node->type) {
        case VAR_LVALUE:
            cvec_append_array(print_buffer, "(VarLValue ", 11);
            cvec_append_ref(print_buffer, node->field1.string);
            break;
        case ARRAY_LVALUE:
            cvec_append_array(print_buffer, "(ArrayLValue ", 13);
            cvec_append_ref(print_buffer, node->field1.string);
            print_lvalue_vars(node->field2.var_list);
            break;
        default:
            return;
    }
    cvec_append(print_buffer, ')');
}
void print_lvalue_vars(Vector *vector) {
    if (vector_is_empty(vector)) return;
    size_t size = vector->size;
    char *string;

    for (size_t i = 0; i < size; ++i) {
        cvec_append(print_buffer, ' ');
        string = vector_get(vector, i);
        cvec_append_array(print_buffer, string, strlen(string));
    }
}

void print_type_node(Type *node) {
    switch (node->type) {
        case INT_TYPE:
            cvec_append_array(print_buffer, "(IntType", 8);
            break;
        case FLOAT_TYPE:
            cvec_append_array(print_buffer, "(FloatType", 10);
            break;
        case BOOL_TYPE:
            cvec_append_array(print_buffer, "(BoolType", 9);
            break;
        case ARRAY_TYPE:
            cvec_append_array(print_buffer, "(ArrayType ", 11);
            print_type_node(node->field1.type);
            cvec_append(print_buffer, ' ');
            char *string = malloc(MAXIMUM_BUFFER); if (!string) return;
            if (snprintf(string, MAXIMUM_BUFFER, "%d", node->comma_count) < 0) return;
            cvec_append_array(print_buffer, string, strlen(string));
            free(string);
            break;
        case VOID_TYPE:
            cvec_append_array(print_buffer, "(VoidType", 9);
            break;
        case STRUCT_TYPE:
            cvec_append_array(print_buffer, "(StructType ", 12);
            cvec_append_ref(print_buffer, node->field1.string);
            break;
        default:
            return;
    }

    cvec_append(print_buffer, ')');
}

void print_stmt_node(Stmt *node) {
    switch (node->type) {
        case LET_STMT:
            cvec_append_array(print_buffer, "(LetStmt ", 9);
            print_lvalue_node(node->field1.lvalue);
            cvec_append(print_buffer, ' ');
            print_expr_node(node->field2.expr);
            break;
        case ASSERT_STMT:
            cvec_append_array(print_buffer, "(AssertStmt ", 12);
            print_expr_node(node->field1.expr);
            cvec_append(print_buffer, ' ');
            cvec_append_ref(print_buffer, node->field2.string);
            break;
        case RETURN_STMT:
            cvec_append_array(print_buffer, "(ReturnStmt ", 12);
            print_expr_node(node->field1.expr);
            break;
        default:
            return;
    }
    cvec_append(print_buffer, ')');
}

void print_bind_node(Bind *node) {
    print_lvalue_node(node->field1);
    cvec_append(print_buffer, ' ');
    print_type_node(node->field2);
}
