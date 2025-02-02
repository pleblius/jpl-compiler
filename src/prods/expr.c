#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "prod.h"
#include "parser.h"
#include "stringops.h"
#include "vector.h"
#include "error.h"

#define MAXIMUM_BUFFER 1024

const char *expr_strings[] = { "(IntExpr", "(FloatExpr", "(TrueExpr", "(FalseExpr", "(VarExpr", "(ArrayLiteralExpr", 
                    "(VoidExpr", "(StructLiteralExpr", "(DotExpr", "(ArrayIndexExpr", "(CallExpr" };

extern Vector *token_list;

int parse_expr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    TokenType type = peek_token(index);

    int status = EXIT_SUCCESS;

    switch (type) {
        case LPAREN:
            ++index;
            if (parse_expr(&index, node) == EXIT_FAILURE)
                status = EXIT_FAILURE;

            if (expect_token(index, RPAREN, NULL) == EXIT_FAILURE)
                parse_error(MISSING_PAREN, *p_index, index, index+1, NULL);
            else ++index;
            break;
        case INTVAL:
            if (parse_intexpr(&index, node) == EXIT_FAILURE) status = EXIT_FAILURE;
            break;
        case FLOATVAL:
            if (parse_floatexpr(&index, node) == EXIT_FAILURE) status = EXIT_FAILURE;
            break;
        case TRUE:
            if (parse_trueexpr(&index, node) == EXIT_FAILURE) status = EXIT_FAILURE;
            break;
        case FALSE:
            if  (parse_falseexpr(&index, node) == EXIT_FAILURE) status = EXIT_FAILURE;
            break;
        case LSQUARE:
            if (parse_arrayliteralexpr(&index, node) == EXIT_FAILURE) status = EXIT_FAILURE;
            break;
        case VARIABLE:
            if (parse_varexpr(&index, node) == EXIT_FAILURE) status = EXIT_FAILURE;
            break;
        case VOID:
            if (parse_voidexpr(&index, node) == EXIT_FAILURE) status = EXIT_FAILURE;
            break;
        default:
            parse_error(INVALID_EXPR, *p_index, index, index+1, NULL);
            status = EXIT_FAILURE;
    }

    while (1) {
        type = peek_token(index);
        if (type == DOT) {
            if (parse_dotexpr(&index, node) == EXIT_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
        else if (type == LSQUARE) {
            if (parse_arrayindexexpr(&index, node) == EXIT_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
        else
            break;
    }

    if (status == EXIT_FAILURE) free_expr(node);

    *p_index = index;
    return status;
}
int parse_intexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    char *string, *end;

    if (expect_token(index, INTVAL, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[intval]");
    else ++index;
    
    uint64_t intval = strtol(string, &end, 10);
    free(string);

    if (errno == ERANGE)
        parse_error(INT_RANGE, *p_index, index, index+1, NULL);

    node -> type = INT_EXPR;
    node -> field1.int_value = intval;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_floatexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    char *string, *end;
    
    if (expect_token(index, FLOATVAL, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[floatval]");
    else ++index;

    double floatval = strtod(string, &end);
    free(string);
    if (errno == ERANGE)
        parse_error(FLOAT_RANGE, *p_index, index, index+1, NULL);

    node -> type = FLOAT_EXPR;
    node -> field1.float_value = floatval;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_trueexpr(uint64_t *p_index, Expr* node) {
    node -> type = TRUE_EXPR;
    node -> field1.int_value = 1;
    *p_index += 1;
    return EXIT_SUCCESS;
}
int parse_falseexpr(uint64_t *p_index, Expr* node) {
    node -> type = FALSE_EXPR;
    node -> field1.int_value = 0;
    *p_index += 1;
    return EXIT_SUCCESS;
}
int parse_voidexpr(uint64_t *p_index, Expr *node) {
    node->type = VOID_EXPR;
    node->field1.expr = NULL;
    *p_index += 1;
    return EXIT_SUCCESS;
}

int parse_arrayliteralexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;

    ++index;

    Vector *expr_list = vector_create();
    if (!expr_list) MALLOC_FAILURE;

    if (parse_exprlist(&index, expr_list, LSQUARE) == EXIT_FAILURE) {
        vector_destroy(expr_list);
        expr_list = NULL;
    }

    if (expect_token(index, RSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACKET, *p_index, index, index+1, NULL);
    else ++index;

    node->type = ARRAY_EXPR;
    node->field1.expr_list = expr_list;
    node->field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_varexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    char *string;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[variable]");
    else ++index;

    node->type = VAR_EXPR;
    node->field1.string = string;
    node->field2.string = NULL;

    TokenType type = peek_token(index);

    if (type == LCURLY) {
        parse_structliteralexpr(&index, node);
    }
    else if (type == LPAREN)
        parse_callexpr(&index, node);

    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_structliteralexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    ++index;

    Vector *expr_list = vector_create();
    if (!expr_list) MALLOC_FAILURE;

    if (parse_exprlist(&index, expr_list, LCURLY) == EXIT_FAILURE) {
        vector_destroy(expr_list);
        expr_list = NULL;
    }

    if (expect_token(index, RCURLY, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACE, *p_index, index, index+1, NULL);
    else ++index;

    node->type = STRUCTLITERAL_EXPR;
    node->field2.expr_list = expr_list;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_callexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    ++index;

    Vector *expr_list = vector_create();
    if (!expr_list) MALLOC_FAILURE;
    if (parse_exprlist(&index, expr_list, LPAREN) == EXIT_FAILURE) {
        vector_destroy(expr_list);
        expr_list = NULL;
    }

    if (expect_token(index, RPAREN, NULL) == EXIT_FAILURE)
        parse_error(MISSING_PAREN, *p_index, index, index+1, NULL);
    else ++index;

    node->type = CALL_EXPR;
    node->field2.expr_list = expr_list;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_arrayindexexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;

    if (expect_token(index, LSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[");
    else ++index;

    Vector *expr_list = vector_create();
    if (!expr_list) MALLOC_FAILURE;
    if (parse_exprlist(&index, expr_list, LSQUARE) == EXIT_FAILURE) {
        vector_destroy(expr_list);
        expr_list = NULL;
    }

    if (expect_token(index, RSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACKET, *p_index, index, index+1, NULL);
    else ++index;

    Expr *new_child = malloc(sizeof(Expr));
    if (!new_child) MALLOC_FAILURE;

    memcpy(new_child, node, sizeof(Expr));

    node->type = ARRAYINDEX_EXPR;
    node->field1.expr = new_child;
    node->field2.expr_list = expr_list;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_dotexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    char *string;

    if (expect_token(index, DOT, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, ".");
    else ++index;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[variable]");
    else ++index;

    Expr *new_child = malloc(sizeof(Expr));
    if (!new_child) MALLOC_FAILURE;

    memcpy(new_child, node, sizeof(Expr));

    node->type = DOT_EXPR;
    node->field1.expr = new_child;
    node->field2.string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_exprlist(uint64_t *p_index, Vector *list, TokenType encps) {
    uint64_t index = *p_index;
    ParseErrorType error;

    TokenType end;
    switch (encps) {
        case LSQUARE:
            end = RSQUARE;
            error = MISSING_BRACKET;
            break;
        case LCURLY:
            end = RCURLY;
            error = MISSING_BRACE;
            break;
        case LPAREN:
            end = RPAREN;
            error = MISSING_PAREN;
            break;
        default:
            return EXIT_FAILURE;
    }

    TokenType type;
    type = peek_token(index);
    if (type == end) return EXIT_SUCCESS;

    uint64_t expr_index = index;

    while (1) {
        type = peek_token(index);

        if (type == NEWLINE || type == END_OF_FILE) {
            parse_error(error, expr_index, index, index+1, NULL);
            break;
        }

        Expr *expr = malloc(sizeof(Expr));
        if (!expr) MALLOC_FAILURE;

        if (parse_expr(&index, expr) == EXIT_FAILURE) {
            free(expr);
            expr = NULL;
        }

        vector_append(list, expr);

        type = peek_token(index);

        if (type == COMMA) {
            ++index;
        }
        else if (type == end) {
            break;
        }
        else {
            parse_error(error, expr_index, index, index+1, NULL);
            break;
        }
    }

    *p_index = index;

    return EXIT_SUCCESS;
}

char *expr_string(Expr* node) {
    const char *expr_type = expr_strings[node->type];
    char *string1, *string2, *output = NULL;

    int rem;
    
    switch (node->type) {
        case INT_EXPR:
            string1 = malloc(MAXIMUM_BUFFER); if (!string1) return NULL;
            rem = snprintf(string1, MAXIMUM_BUFFER, "%ld", node->field1.int_value); if (rem < 0) fprintf(stderr, "Buffer error.\n");
            output = string_combine(4, expr_type, " ", string1, ")");
            free(string1);
            break;
        case FLOAT_EXPR:
            string1 = malloc(MAXIMUM_BUFFER); if (!string1) return NULL;
            rem = snprintf(string1, MAXIMUM_BUFFER, "%ld", (uint64_t)node->field1.float_value); if (rem < 0) fprintf(stderr, "Buffer error.\n");
            output = string_combine(4, expr_type, " ", string1, ")");
            free(string1);
            break;
        case FALSE_EXPR:
        case VOID_EXPR:
        case TRUE_EXPR:
            output = string_combine(2, expr_type, ")");
            break;
        case VAR_EXPR:
            output = string_combine(4, expr_type, " ", node->field1.string, ")");
            break;
        case ARRAY_EXPR:
            string1 = exprlist_string(node->field1.expr_list);
            if (strlen(string1) == 0) output = string_combine(2, expr_type, ")");
            else output = string_combine(4, expr_type, " ", string1, ")");
            free(string1);
            break;
        case DOT_EXPR:
            string1 = expr_string(node->field1.expr);
            string2 = node->field2.string;
            output = string_combine(6, expr_type, " ", string1, " ", string2, ")");
            free(string1);
            break;
        case ARRAYINDEX_EXPR:
            string1 = expr_string(node->field1.expr);
            string2 = exprlist_string(node->field2.expr_list);
            if (strlen(string2) == 0) output = string_combine(4, expr_type, " ", string1, ")");
            else output = string_combine(6, expr_type, " ", string1, " ", string2, ")");
            free(string1); free(string2);
            break;
        case STRUCTLITERAL_EXPR:
            string1 = node->field1.string;
            string2 = exprlist_string(node->field2.expr_list);
            if (strlen(string2) == 0) output = string_combine(4, expr_type, " ", string1, ")");
            else output = string_combine(6, expr_type, " ", string1, " ", string2, ")");
            free(string2);
            break;
        case CALL_EXPR:
            string1 = node->field1.string;
            string2 = exprlist_string(node->field2.expr_list);
            if (strlen(string2) == 0) output = string_combine(4, expr_type, " ", string1, ")");
            else output = string_combine(6, expr_type, " ", string1, " ", string2, ")");
            free(string2);
            break;
    }
    return output;
}
char *exprlist_string(Vector *list) {
    char *temp1, *temp2;
    if (list->size == 0) return calloc(1,1);
    char *output = expr_string(vector_get(list, 0));

    for (int i = 1; i < list->size; ++i) {
        temp1 = expr_string(vector_get(list, i));
        temp2 = output;
        output = string_combine(3, temp2, " ", temp1);
        free(temp1);
        free(temp2);
    }

    return output;
}

void free_expr(Expr* node) {
    if (!node) return;
    switch (node->type) {
        case VAR_EXPR:
            free(node->field1.string);
            break;
        case ARRAY_EXPR:
            free_exprlist(node->field1.expr_list);
            vector_destroy(node->field1.expr_list);
            break;
        case STRUCTLITERAL_EXPR:
            free(node->field1.string);
            free_exprlist(node->field2.expr_list);
            vector_destroy(node->field2.expr_list);
            break;
        case DOT_EXPR:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            free(node->field2.string);
            break;
        case ARRAYINDEX_EXPR:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            free_exprlist(node->field2.expr_list);
            vector_destroy(node->field2.expr_list);
            break;
        case CALL_EXPR:
            free(node->field1.string);
            free_exprlist(node->field2.expr_list);
            vector_destroy(node->field2.expr_list);
            break;
        default: return;
    }
}
void free_exprlist(Vector *list) {
    if (!list) return;
    for (int i = 0; i < list->size; ++i) {
        Expr *ptr = vector_get(list, i);
        free_expr(ptr);
    }
}
