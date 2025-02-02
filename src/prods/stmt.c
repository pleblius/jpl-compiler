#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include "prod.h"
#include "parser.h"
#include "vector.h"
#include "error.h"
#include "stringops.h"

const char *stmt_strings[] = { "(LetStmt", "(AssertStmt", "(ReturnStmt" };

extern Vector *token_list;

int parse_stmt(uint64_t* p_index, Stmt *node) {
    uint64_t index = *p_index;
    TokenType type = peek_token(*p_index);
    int status;
    ++index;
    switch (type) {
        case LET:
            status = parse_letstmt(&index, node);
            break;
        case ASSERT:
            status = parse_assertstmt(&index, node);
            break;
        case RETURN:
            status = parse_returnstmt(&index, node);
            break;
        default:
            ++index;
            parse_error(INVALID_STMT, *p_index, index, index+1, NULL);
            status = EXIT_FAILURE;
    }

    *p_index = index;
    return status;
}

int parse_letstmt(uint64_t *p_index, Stmt *node) {
    uint64_t index = *p_index;

    LValue *lvalue_field = (LValue *) malloc(sizeof(LValue));
    if (!lvalue_field) MALLOC_FAILURE;

    if (parse_lvalue(&index, lvalue_field) == EXIT_FAILURE) {
        free(lvalue_field);
        lvalue_field = NULL;
    }
    
    if (expect_token(index, EQUALS, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, index, index, index+1, "=");
    else ++index;

    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field) == EXIT_FAILURE) {
        free(expr_field);
        expr_field = NULL;
    }

    node->type = LET_STMT;
    node->field1.lvalue = lvalue_field;
    node->field2.expr = expr_field;

    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_assertstmt(uint64_t *p_index, Stmt *node) {
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
    
    node->type = ASSERT_STMT;
    node->field1.expr = expr_field;
    node->field2.string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_returnstmt(uint64_t *p_index, Stmt *node) {
    uint64_t index = *p_index;

    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field) == EXIT_FAILURE) {
        free(expr_field);
        expr_field = NULL;
    }

    node->type = RETURN_STMT;
    node->field1.expr = expr_field;
    node->field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

char *stmt_string(Stmt* node) {
    const char *stmt_type = stmt_strings[node->type];
    char *string1, *string2, *output = NULL;

    switch (node->type) {
        case LET_STMT:
            string1 = lvalue_string(node->field1.lvalue);
            string2 = expr_string(node->field2.expr);
            output = string_combine(6, stmt_type, " ", string1, " ", string2, ")");
            free(string1); free(string2);
            break;
        case ASSERT_STMT:
            string1 = expr_string(node->field1.expr);
            string2 = node->field2.string;
            output = string_combine(6, stmt_type, " ", string1, " ", string2, ")");
            free(string1);
            break;
        case RETURN_STMT:
            string1 = expr_string(node->field1.expr);
            output = string_combine(4, stmt_type, " ", string1, ")");
            free(string1);
            break;
    }

    return output;
}

void free_stmt(Stmt* node) {
    if (!node) return;
    switch (node->type) {
        case LET_STMT:
            free_lvalue(node->field1.lvalue);
            free_expr(node->field2.expr);
            free(node->field1.lvalue);
            free(node->field2.expr);
            break;
        case ASSERT_STMT:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            free(node->field2.string);
            break;
        case RETURN_STMT:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            break;
    }
}
