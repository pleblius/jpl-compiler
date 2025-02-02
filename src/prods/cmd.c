#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "prod.h"
#include "vector.h"
#include "parser.h"
#include "error.h"
#include "stringops.h"

const char *cmd_strings[] = {
    "(ReadCmd", "(WriteCmd", "(LetCmd", "(AssertCmd", "(PrintCmd", "(ShowCmd", "(TimeCmd", "(FnCmd", "(StructCmd" };

extern Vector *token_list;

int parse_cmd(uint64_t* p_index, Cmd* node) {
    uint64_t index = *p_index;
    TokenType type = peek_token(*p_index);
    int status;
    ++index;
    switch (type) {
        case READ:
            status = parse_readcmd(&index, node);
            break;
        case WRITE:
            status = parse_writecmd(&index, node);
            break;
        case LET:
            status = parse_letcmd(&index, node);
            break;
        case ASSERT:
            status = parse_assertcmd(&index, node);
            break;
        case PRINT:
            status = parse_printcmd(&index, node);
            break;
        case SHOW:
            status = parse_showcmd(&index, node);
            break;
        case TIME:
            status = parse_timecmd(&index, node);
            break;
        case FN:
            status = parse_fncmd(&index, node);
            break;
        case STRUCT:
            status = parse_structcmd(&index, node);
            break;
        default:
            parse_error(INVALID_CMD, *p_index, index, index + 1, NULL);
            status = EXIT_FAILURE;
            break;
    }

    *p_index = index;
    return status;
}

int parse_readcmd(uint64_t* p_index, Cmd* node) {
    uint64_t index = *p_index;
    char *string;

    if (expect_token(index, IMAGE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "image");
    else ++index;

    if (expect_token(index, STRING, &string) == EXIT_FAILURE)
        parse_error(MISSING_STRING, *p_index, index, index+1, NULL);
    else ++index;
    
    if (expect_token(index, TO, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "to");
    else ++index;

    LValue *lvalue_field = (LValue *) malloc(sizeof(LValue));
    if (!lvalue_field) MALLOC_FAILURE;
    if (parse_lvalue(&index, lvalue_field) == EXIT_FAILURE) {
        free(lvalue_field);
        lvalue_field = NULL;
    }

    node -> type = READ_CMD;
    node -> field1.string = string;
    node -> field2.lvalue = lvalue_field;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_writecmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index;
    char *string;

    if (expect_token(index, IMAGE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "image");
    else ++index;
    
    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field) == EXIT_FAILURE) {
        free(expr_field);
        expr_field = NULL;
    }

    if (expect_token(index, TO, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index + 1, "to");
    else ++index;
    
    if (expect_token(index, STRING, &string) == EXIT_FAILURE)
        parse_error(MISSING_STRING, *p_index, index, index+1, NULL);
    else ++index;

    node -> type = WRITE_CMD;
    node -> field1.expr = expr_field;
    node -> field2.string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_letcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index;

    LValue *lvalue_field = (LValue *) malloc(sizeof(LValue));
    if (!lvalue_field) MALLOC_FAILURE;

    if (parse_lvalue(&index, lvalue_field) == EXIT_FAILURE) {
        free(lvalue_field);
        lvalue_field = NULL;
    }

    if (expect_token(index, EQUALS, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "=");
    else ++index;

    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if(!expr_field) MALLOC_FAILURE;

    if (parse_expr(&index, expr_field) == EXIT_FAILURE) {
        free(expr_field);
        expr_field = NULL;
    }

    node -> type = LET_CMD;
    node -> field1.lvalue = lvalue_field;
    node -> field2.expr = expr_field;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_assertcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index;
    char *string;

    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field) == EXIT_FAILURE) {
        free(expr_field);
        expr_field = NULL;
    }

    if (expect_token(index, COMMA, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, ",");
    else ++index;

    if (expect_token(index, STRING, &string) == EXIT_FAILURE)
        parse_error(MISSING_STRING, *p_index, index, index+1, NULL);
    else ++index;

    node -> type = ASSERT_CMD;
    node -> field1.expr = expr_field;
    node -> field2.string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_printcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index;
    char *string;

    if (expect_token(index, STRING, &string) == EXIT_FAILURE)
        parse_error(MISSING_STRING, *p_index, index, index+1, NULL);
    else ++index;

    node -> type = PRINT_CMD;
    node->field1.string = string;
    node->field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_showcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index;
    
    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) MALLOC_FAILURE;

    if (parse_expr(&index, expr_field) == EXIT_FAILURE) {
        free(expr_field);
        return EXIT_FAILURE;
    }

    node -> type = SHOW_CMD;
    node -> field1.expr = expr_field;
    node -> field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_timecmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index;


    Cmd *cmd_field = (Cmd *) malloc(sizeof(Cmd));
    if (!cmd_field) MALLOC_FAILURE;
    if (parse_cmd(&index, cmd_field) == EXIT_FAILURE) {
        free(cmd_field);
        cmd_field = NULL;
    }

    node -> type = TIME_CMD;
    node -> field1.cmd = cmd_field;
    node -> field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_fncmd(uint64_t *p_index, Cmd *node) {
    uint64_t index = *p_index;
    uint64_t lparen_index = index;
    char *string;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[variable]");
    else ++index;
    if (expect_token(index, LPAREN, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "(");
    lparen_index = index;
    ++index;
    
    Vector *bindings = vector_create();
    if (!bindings) MALLOC_FAILURE;
    if (parse_fnbindings(&index, bindings) == EXIT_FAILURE) {
        vector_destroy(bindings);
        bindings = NULL;
    }

    if (expect_token(index, RPAREN, NULL) == EXIT_FAILURE)
        parse_error(MISSING_PAREN, lparen_index, index, index+1, NULL);
    else ++index;
    if (expect_token(index, COLON, NULL) == EXIT_FAILURE)
        parse_error(MISSING_COLON, index-1, index, index+1, NULL);
    else ++index;

    Type *type_field = (Type *) malloc(sizeof(Type));
    if (!type_field) MALLOC_FAILURE;

    if (parse_type(&index, type_field) == EXIT_FAILURE) {
        free(type_field);
        type_field = NULL;
    }

    if (expect_token(index, LCURLY, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "{");
    lparen_index = index;
    ++index;

    Vector *statements = vector_create();
    if (!statements) MALLOC_FAILURE;

    if (parse_fnstmts(&index, statements) == EXIT_FAILURE) {
        vector_destroy(statements);
        statements = NULL;
    }

    if (expect_token(index, RCURLY, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACE, lparen_index, index, index+1, NULL);
    ++index;

    node -> type = FN_CMD;
    node -> field1.string = string;
    node -> field2.binds = bindings;
    node -> field3.type = type_field;
    node -> field4.stmts = statements;

    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_fnbindings(uint64_t *p_index, Vector *list) {
    uint64_t index = *p_index;
    TokenType type;

    if (peek_token(index) == RPAREN) return EXIT_SUCCESS;

    while (1) {
        type = peek_token(index);

        if (type == NEWLINE || type == END_OF_FILE) 
            return EXIT_FAILURE;

        Bind *bind = malloc(sizeof(Bind));
        if (!bind) MALLOC_FAILURE;
        
        if (parse_bind(&index, bind) == EXIT_FAILURE) {
            free(bind);
            bind = NULL;
        }

        vector_append(list, bind);

        type = peek_token(index);

        if (type == RPAREN) break;
        else if (expect_token(index, COMMA, NULL) == EXIT_FAILURE)
            parse_error(MISSING_COMMA, index-1, index, index+1, NULL);
        else ++index;
    }

    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_fnstmts(uint64_t *p_index, Vector *list)  {
    uint64_t index = *p_index;

    if (peek_token(index) == RCURLY) return EXIT_SUCCESS;

    if (peek_token(index) == NEWLINE) ++index;

    while (1) {
        Stmt *stmt = (Stmt *) malloc(sizeof(Stmt));
        if (!stmt) MALLOC_FAILURE;
        uint64_t stmt_index = index;
        if (parse_stmt(&index, stmt) == EXIT_FAILURE) {
            free(stmt);
            stmt = NULL;
        }

        vector_append(list, stmt);

        if (expect_token(index, NEWLINE, NULL) == EXIT_FAILURE)
            parse_error(MISSING_NEWLINE, stmt_index, index, index+1, NULL);
        else ++index;

        if (peek_token(index) == RCURLY) break;
    }

    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_structcmd(uint64_t *p_index, Cmd *node) {
    uint64_t index = *p_index;
    char *string;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[variable]");
    else ++index;
    
    if (expect_token(index, LCURLY, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "{");
    else ++index;

    if (expect_token(index, NEWLINE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_NEWLINE, *p_index, index, index+1, NULL);
    else ++index;

    Vector *variables = vector_create();
    Vector *types = vector_create();
    if (!variables || !types) MALLOC_FAILURE;

    if (parse_structmembers(&index, variables, types) == EXIT_FAILURE) {
        vector_destroy(variables);
        vector_destroy(types);
        types = NULL;
        variables = NULL;
    }

    if (expect_token(index, RCURLY, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACE, index-1, index, index+1, NULL);
    else ++index;

    node->type = STRUCT_CMD;
    node->field1.string = string;
    node->field2.vars = variables;
    node->field3.types = types;
    node->field4.stmts = NULL;

    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_structmembers(uint64_t *p_index, Vector *variables, Vector *types) {
    uint64_t index = *p_index;
    char *string;

    if (peek_token(index) == RCURLY) return EXIT_SUCCESS;
    
    while (1) {
        uint64_t line_index = index;
        if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE)
            parse_error(MISSING_TOKEN, line_index, index, index+1, "[variable]");
        else ++index;

        if (expect_token(index, COLON, NULL) == EXIT_FAILURE)
            parse_error(MISSING_COLON, index-1, index, index+1, NULL);
        else ++index;

        Type* type_field = (Type *) malloc(sizeof(Type));
        if (!type_field) MALLOC_FAILURE;

        if(parse_type(&index, type_field) == EXIT_FAILURE) {
            free(type_field);
            type_field = NULL;
        }
        vector_append(variables, string);
        vector_append(types, type_field);

        if (expect_token(index, NEWLINE, NULL) == EXIT_FAILURE)
            parse_error(MISSING_NEWLINE, line_index, index, index+1, NULL);
        else ++index;

        if (peek_token(index) == RCURLY) break;
    }

    *p_index = index;
    return EXIT_SUCCESS;
}

char *cmd_string(Cmd* node) {
    const char *cmd_type = cmd_strings[node->type];
    char *string1, *string2, *string3, *string4, *output = NULL;

    switch (node->type) {
        case READ_CMD:
            string1 = node->field1.string;
            string2 = lvalue_string(node->field2.lvalue); if (!string1 || !string2) return NULL;
            output = string_combine(6, cmd_type, " ", string1, " ", string2, ")");
            free(string2);
            break;
        case WRITE_CMD:
            string1 = expr_string(node->field1.expr);
            string2 = node->field2.string; if (!string1 || !string2) return NULL;
            output = string_combine(6, cmd_type, " ", string1, " ", string2, ")");
            free(string1);
            break;
        case LET_CMD:
            string1 = lvalue_string(node->field1.lvalue);
            string2 = expr_string(node->field2.expr); if (!string1 || !string2) return NULL;
            output = string_combine(6, cmd_type, " ", string1, " ", string2, ")");
            free(string1); free(string2);
            break;
        case ASSERT_CMD:
            string1 = expr_string(node->field1.expr);
            string2 = node->field2.string; if (!string1 || !string2) return NULL;
            output = string_combine(6, cmd_type, " ", string1, " ", string2, ")");
            free(string1);
            break;
        case PRINT_CMD:
            string1 = node->field1.string; if (!string1) return NULL;
            output = string_combine(4, cmd_type, " ", string1, ")");
            break;
        case SHOW_CMD:
            string1 = expr_string(node->field1.expr); if (!string1) return NULL;
            output = string_combine(4, cmd_type, " ", string1, ")");
            free(string1);
            break;
        case TIME_CMD:
            string1 = cmd_string(node->field1.cmd); if (!string1) return NULL;
            output = string_combine(4, cmd_type, " ", string1, ")");
            free(string1);
            break;
        case FN_CMD:
            string1 = node->field1.string;
            string2 = cmd_bind_string(node);
            string3 = type_string(node->field3.type);
            string4 = cmd_stmt_string(node);
            output = string_combine(9, cmd_type, " ", string1, " (", string2, ") ", string3, string4, ")");
            free(string2); free(string3); free(string4);
            break;
        case STRUCT_CMD:
            string1 = node->field1.string;
            string2 = cmd_struct_string(node);
            if (strlen(string2) == 0) output = string_combine(4, cmd_type, " ", string1, ")");
            else output = string_combine(6, cmd_type, " ", string1, " ", string2, ")");
            free(string2);
            break;
    }

    return output;
}
char *cmd_bind_string(Cmd *node) {
    char *temp1, *temp2;
    Vector *list = node -> field2.binds;
    if (list->size == 0) {
        return string_combine(1, "()");
    }
    char *output = bind_string(vector_get(list, 0));

    for (int i = 1; i < list->size; ++i) {
        temp1 = bind_string(vector_get(list, i));
        temp2 = output;
        output = string_combine(3, temp2, " ", temp1);
        free(temp1);
        free(temp2);
    }

    temp1 = output;
    output = string_combine(3, "(", temp1, ")");
    free(temp1);

    return output;
}
char *cmd_stmt_string(Cmd *node) {
    char *temp1, *temp2;
    Vector *list = node -> field4.stmts;
    if (list->size == 0) return calloc(1,1);
    char *output = stmt_string(vector_get(list, 0));

    for (int i = 1; i < list->size; ++i) {
        temp1 = stmt_string(vector_get(list, i));
        temp2 = output;
        output = string_combine(3, temp2, " ", temp1);
        free(temp1);
        free(temp2);
    }

    temp1 = output;
    output = string_combine(2, " ", temp1);
    free(temp1);
    return output;
}
char *cmd_struct_string(Cmd *node) {
    char *temp1, *temp2;
    Vector *var_list = node->field2.vars;
    Vector *type_list = node->field3.types;
    if (var_list->size == 0) return calloc(1,1);
    temp1 = vector_get(var_list, 0);
    temp2 = type_string(vector_get(type_list, 0));
    char *output = string_combine(3, temp1, " ", temp2);
    free(temp2);

    for (int i = 1; i < var_list->size; ++i) {
        temp1 = type_string(vector_get(type_list, i));
        temp2 = output;
        output = string_combine(5, temp2, " ", vector_get(var_list,i), " ", temp1);
        free(temp1); free(temp2);
    }

    return output;
}
void free_cmd(Cmd* node) {
    if (!node) return;

    switch (node->type) {
        case READ_CMD:
            free(node->field1.string);
            free_lvalue(node->field2.lvalue);
            free(node->field2.lvalue);
            break;
        case WRITE_CMD:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            free(node->field2.string);
            break;
        case LET_CMD:
            free_lvalue(node->field1.lvalue);
            free(node->field1.lvalue);
            free_expr(node->field2.expr);
            free(node->field2.expr);
            break;
        case ASSERT_CMD:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            free(node->field2.string);
            break;
        case PRINT_CMD:
            free(node->field1.string);
            break;
        case SHOW_CMD:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            break;
        case TIME_CMD:
            free_cmd(node->field1.cmd);
            free(node->field1.cmd);
            break;
        case FN_CMD:
            free(node->field1.string);
            free_bindings(node->field2.binds);
            vector_destroy(node->field2.binds);
            free_type(node->field3.type);
            free_statements(node->field4.stmts);
            vector_destroy(node->field4.stmts);
            break;
        case STRUCT_CMD:
            free(node->field1.string);
            free(node->field2.string);
            free_type(node->field3.type);
            break;
    }
}
void free_bindings(Vector *list) {
    if (!list) return;
    for (int i = 0; i < list->size; ++i) {
        free_bind(vector_get(list, i));
    }
}
void free_statements(Vector *list) {
    if (!list) return;
    for (int i = 0; i < list->size; ++i) {
        free_stmt(vector_get(list, i));
    }   
}
void free_types(Vector *list) {
    if (!list) return;
    for (int i = 0; i < list->size; ++i) {
        free_type(vector_get(list, i));
    }
}
