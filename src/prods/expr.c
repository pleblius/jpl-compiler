#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "prod.h"
#include "parser.h"
#include "stringops.h"
#include "vector.h"
#include "error.h"

const char *expr_strings[] = { "(IntExpr", "(FloatExpr", "(TrueExpr", "(FalseExpr", "(VarExpr", "(ArrayLiteralExpr", 
                    "(VoidExpr", "(StructLiteralExpr", "(DotExpr", "(ArrayIndexExpr", "(CallExpr", "(UnopExpr", 
                    "(BinopExpr", "(IfExpr", "(ArrayLoopExpr", "(SumLoopExpr" };

const char *binops[] = { "+", "-", "*", "/", "%", "<", ">", "==", "!=", "<=", ">=", "&&", "||" };
#define NUM_BINOPS 13

#define MAX_PRECEDENCE 7
#define MIN_PRECEDENCE 0

const char *pr5[] = {"*", "/", "%"};
#define PR5_COUNT 3
const char *pr4[] = {"+", "-"};
#define PR4_COUNT 2
const char *pr3[] = {"<", ">", "<=", ">=", "==", "!="};
#define PR3_COUNT 6
const char *pr2[] = {"&&", "||"};
#define PR2_COUNT 2 
const char *pr1[] = {"array", "sum", "if"};
#define PR1_COUNT 3

const char *unops[] = { "-", "!" };
#define NUM_UNOPS 2

extern Vector *token_list;

int parse_expr(uint64_t *p_index, Expr* node) {
    if (parse_expr_literal(p_index, node) == EXIT_FAILURE) return EXIT_FAILURE;

    if (peek_token(*p_index) == OP)
        if (parse_binopexpr(p_index, node, MIN_PRECEDENCE) == EXIT_FAILURE) {
            free_expr(node);
            return EXIT_FAILURE;
        }

    return EXIT_SUCCESS;
}

int parse_expr_literal(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    int status = EXIT_SUCCESS;

    if (is_prefix(index)) {
        if (parse_prefixexpr(&index, node) == EXIT_FAILURE) return EXIT_FAILURE;
    }
    else {
        switch (peek_token(index)) {
            case LPAREN:
                ++index;
                status = parse_expr(&index, node);
                if (expect_token(index, RPAREN, NULL) == EXIT_FAILURE) 
                    parse_error(MISSING_PAREN, *p_index, index, NULL);
                else ++index;
                break;
            case INTVAL:
                status = parse_intexpr(&index, node);
                break;
            case FLOATVAL:
                status = parse_floatexpr(&index, node);
                break;
            case TRUE:
                status = parse_trueexpr(&index, node);
                break;
            case FALSE:
                status = parse_falseexpr(&index, node);
                break;
            case LSQUARE:
                status = parse_arrayliteralexpr(&index, node);
                break;
            case VARIABLE:
                status = parse_varexpr(&index, node);
                break;
            case VOID:
                status = parse_voidexpr(&index, node);
                break;
            case ARRAY:
                status = parse_arrayloopexpr(&index, node);
                break;
            case SUM:
                status = parse_sumloopexpr(&index, node);
                break;
            case IF:
                status = parse_ifthenexpr(&index, node);
                break;
            default:
                parse_error(INVALID_EXPR, *p_index, index, NULL);
                status = EXIT_FAILURE;
        }
    }
    if (status == EXIT_FAILURE) return EXIT_FAILURE;

    while (is_postfix(index)) {
        if (parse_postfixexpr(&index, node) == EXIT_FAILURE) goto exprliteral_exit1;
    }

    *p_index = index;
    return EXIT_SUCCESS;

exprliteral_exit1:
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_intexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    char *string = NULL;
    char *end;
    uint64_t intval = 0;

    if (expect_token(index, INTVAL, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, "[intval]");
    else {
        intval = strtol(string, &end, 10);
        free(string);
        ++index;
        if (errno == ERANGE)
            parse_error(INT_RANGE, *p_index, index, NULL);
    }
    
    node -> type = INT_EXPR;
    node -> field1.int_value = intval;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_floatexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    char *string = NULL;
    char *end;
    double floatval = 0.0;

    if (expect_token(index, FLOATVAL, &string) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, "[floatval]");
    else {
        floatval = strtod(string, &end);
        free(string);
        if (errno == ERANGE)
            parse_error(FLOAT_RANGE, *p_index, index, NULL);
        ++index;
    }

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
    if (parse_exprlist(&index, expr_list, LSQUARE) == EXIT_FAILURE)
        goto arraylit_exit1;

    if (expect_token(index, RSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACKET, *p_index, index, NULL);
    else ++index;

    *p_index = index;
    node->type = ARRAY_EXPR;
    node->field1.expr_list = expr_list;
    return EXIT_SUCCESS;

arraylit_exit1:
    vector_destroy(expr_list);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_varexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    char* string;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE) {
        parse_error(MISSING_TOKEN, *p_index, index, "[variable]");
        return EXIT_FAILURE;
    }
    
    ++index;

    node->type = VAR_EXPR;
    node->field1.string = string;

    TokenType type = peek_token(index);

    if (type == LCURLY) {
        if (parse_structliteralexpr(&index, node) == EXIT_FAILURE)
            goto varexpr_exit1;
    }
    else if (type == LPAREN)
        if (parse_callexpr(&index, node) == EXIT_FAILURE)
            goto varexpr_exit1;

    *p_index = index;
    return EXIT_SUCCESS;

varexpr_exit1:
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_structliteralexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    ++index;

    Vector *expr_list = vector_create();
    if (!expr_list) MALLOC_FAILURE;
    if (parse_exprlist(&index, expr_list, LCURLY) == EXIT_FAILURE)
        goto structlit_exit1;

    if (expect_token(index, RCURLY, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACE, *p_index, index, NULL);
    else ++index;

    *p_index = index;
    node->type = STRUCTLITERAL_EXPR;
    node->field2.expr_list = expr_list;
    return EXIT_SUCCESS;

structlit_exit1:
    vector_destroy(expr_list);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_callexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;

    ++index;

    Vector *expr_list = vector_create();
    if (!expr_list) MALLOC_FAILURE;
    if (parse_exprlist(&index, expr_list, LPAREN) == EXIT_FAILURE)
        goto callexpr_exit1;

    if (expect_token(index, RPAREN, NULL) == EXIT_FAILURE)
        parse_error(MISSING_PAREN, *p_index, index, NULL);
    else ++index;

    *p_index = index;
    node->type = CALL_EXPR;
    node->field2.expr_list = expr_list;
    return EXIT_SUCCESS;

callexpr_exit1:
    vector_destroy(expr_list);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_arrayindexexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;

    ++index;

    Vector *expr_list = vector_create();
    if (!expr_list) MALLOC_FAILURE;
    if (parse_exprlist(&index, expr_list, LSQUARE) == EXIT_FAILURE)
        goto indexexpr_exit1;

    if (expect_token(index, RSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACKET, *p_index, index, NULL);
    else ++index;

    *p_index = index;

    Expr *new_child = malloc(sizeof(Expr));
    if (!new_child) MALLOC_FAILURE;

    memcpy(new_child, node, sizeof(Expr));

    node->type = ARRAYINDEX_EXPR;
    node->field1.expr = new_child;
    node->field2.expr_list = expr_list;
    return EXIT_SUCCESS;

indexexpr_exit1:
    free_expr(node);
    vector_destroy(expr_list);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_dotexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    char* string;

    ++index;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE) {
        parse_error(MISSING_TOKEN, *p_index, index, "[variable]");
        goto dotexpr_exit1;
    }
    
    ++index;

    *p_index = index;

    Expr *new_child = malloc(sizeof(Expr));
    if (!new_child) MALLOC_FAILURE;

    memcpy(new_child, node, sizeof(Expr));

    node->type = DOT_EXPR;
    node->field1.expr = new_child;
    node->field2.string = string;
    return EXIT_SUCCESS;

dotexpr_exit1:
    free_expr(node);
    *p_index = index;
    return EXIT_FAILURE;
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

    TokenType type = peek_token(index);
    if (type == end) return EXIT_SUCCESS;

    while (1) {
        type = peek_token(index);

        if (type == NEWLINE || type == END_OF_FILE) {
            parse_error(error, index-1, index, NULL);
            goto exprlist_exit1;
        }

        Expr *expr = malloc(sizeof(Expr));
        if (!expr) MALLOC_FAILURE;
        if (parse_expr(&index, expr) == EXIT_FAILURE) {
            free(expr);
            goto exprloop1;
        }

        vector_append(list, expr);

        type = peek_token(index);

        if (type == COMMA) {
            ++index;
            continue;
        }
        else if (type == end)
            break;
        else parse_error(MISSING_BRACKET, index-1, index, NULL);

    exprloop1:
        while (peek_token(index) != end) {
            if (peek_token(index) == NEWLINE || peek_token(index) == END_OF_FILE) break;
            ++index;
        }
        goto exprlist_exit1;
    }
    
    *p_index = index;
    return EXIT_SUCCESS;

exprlist_exit1:
    free_exprlist(list);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_prefixexpr(uint64_t *p_index, Expr *node) {
    return parse_unopexpr(p_index, node);
}

int parse_arrayloopexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    Vector *var_list = NULL;
    Vector *expr_list = NULL;
    Expr *expr_field = NULL;

    ++index;
    if (expect_token(index, LSQUARE, NULL) == EXIT_FAILURE) 
        parse_error(MISSING_TOKEN, *p_index, index, "[");
    else ++index;
    
    var_list = vector_create(); expr_list = vector_create();
    if (!var_list || !expr_list) MALLOC_FAILURE;
    if (parse_loopbinds(&index, var_list, expr_list) == EXIT_FAILURE) goto arrayloop_exit1;

    if (expect_token(index, RSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACKET, index-1 , index, NULL);
    else ++index;
    
    expr_field = malloc(sizeof(Expr)); if (!expr_field) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field) == EXIT_FAILURE) goto arrayloop_exit2;

    node->type = ARRAY_LOOP_EXPR;
    node->field1.var_list = var_list;
    node->field2.expr_list = expr_list;
    node->field3.expr = expr_field;
    *p_index = index;
    return EXIT_SUCCESS;

arrayloop_exit2:
    free(expr_field);
    free_exprlist(expr_list);
arrayloop_exit1:
    vector_destroy(expr_list);
    vector_destroy(var_list);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_sumloopexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    Vector *var_list = NULL;
    Vector *expr_list = NULL;
    Expr *expr_field = NULL;

    ++index;
    if (expect_token(index, LSQUARE, NULL) == EXIT_FAILURE) parse_error(MISSING_TOKEN, *p_index, index, "[");
    else ++index;

    var_list = vector_create(); expr_list = vector_create();
    if (!var_list || !expr_list) MALLOC_FAILURE;
    if (parse_loopbinds(&index, var_list, expr_list) == EXIT_FAILURE) goto sumloop_exit1;

    if (expect_token(index, RSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACKET, index-1, index, NULL);
    else ++index;

    expr_field = malloc(sizeof(Expr));
    if (!expr_field) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field) == EXIT_FAILURE) goto sumloop_exit2;

    node->type = SUM_LOOP_EXPR;
    node->field1.var_list = var_list;
    node->field2.expr_list = expr_list;
    node->field3.expr = expr_field;
    *p_index = index;
    return EXIT_SUCCESS;

sumloop_exit2:
    free(expr_field);
    free_exprlist(expr_list);
sumloop_exit1:
    vector_destroy(expr_list);
    vector_destroy(var_list);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_ifthenexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    Expr *expr_field1 = NULL;
    Expr *expr_field2 = NULL;
    Expr *expr_field3 = NULL;

    if (expect_token(index, IF, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, "if");
    else ++index;

    expr_field1 = malloc(sizeof(Expr)); if (!expr_field1) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field1) == EXIT_FAILURE) goto ifthen_exit1;

    if (expect_token(index, THEN, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, "then");
    else ++index;

    expr_field2 = malloc(sizeof(Expr)); if (!expr_field2) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field2) == EXIT_FAILURE) goto ifthen_exit2;

    if (expect_token(index, ELSE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_TOKEN, *p_index, index, "else");
    else ++index;

    expr_field3 = malloc(sizeof(Expr)); if (!expr_field3) MALLOC_FAILURE;
    if (parse_expr(&index, expr_field3) == EXIT_FAILURE) goto ifthen_exit3;

    node->type = IF_EXPR;
    node->field1.expr = expr_field1;
    node->field2.expr = expr_field2;
    node->field3.expr = expr_field3;
    *p_index = index;
    return EXIT_SUCCESS;

ifthen_exit3:
    free(expr_field3);
    free_expr(expr_field2);
ifthen_exit2:
    free(expr_field2);
    free_expr(expr_field1);
ifthen_exit1:
    free(expr_field1);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_unopexpr(uint64_t *p_index, Expr *node) {
    uint64_t index = *p_index;
    char* string;
    Expr *expr_field = NULL;

    expect_token(index, OP, &string);
    ++index;

    expr_field = malloc(sizeof(Expr)); if (!expr_field) MALLOC_FAILURE;
    if (parse_expr_literal(&index, expr_field) == EXIT_FAILURE) goto unopexpr_exit1;

    node->type = UNOP_EXPR;
    node->field1.string = string;
    node->field2.expr = expr_field;
    *p_index = index;
    return EXIT_SUCCESS;

unopexpr_exit1:
    free(expr_field);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_binopexpr(uint64_t *p_index, Expr *node, uint32_t min_precedence) {
    uint64_t index = *p_index;
    uint32_t precedence = 0;
    char *string;
    Expr *right_node = NULL;
    Expr *new_left = NULL;

    while (expect_token(index, OP, &string) == EXIT_SUCCESS) {
        precedence = get_precedence(string);
        if (precedence < min_precedence) {
            free(string);
            break;
        }
        ++index;

        right_node = malloc(sizeof(Expr)); if (!right_node) MALLOC_FAILURE;
        if (parse_expr_literal(&index, right_node) == EXIT_FAILURE) goto binopexpr_exit1;

        if (peek_token(index) == OP)
            if (parse_binopexpr(&index, right_node, precedence + 1) == EXIT_FAILURE) goto binopexpr_exit2;

        new_left = malloc(sizeof(Expr)); if (!new_left) MALLOC_FAILURE;
        
        memcpy(new_left, node, sizeof(Expr));
        node->type = BINOP_EXPR;
        node->field1.string = string;
        node->field2.expr = new_left;
        node->field3.expr = right_node;
    }

    *p_index = index;
    return EXIT_SUCCESS;

binopexpr_exit2:
    free_expr(right_node);
binopexpr_exit1:
    free(right_node);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_postfixexpr(uint64_t *p_index, Expr *node) {
    TokenType type = peek_token(*p_index);

    switch (type) {
        case LSQUARE:
            return parse_arrayindexexpr(p_index, node);
        case DOT:
            return parse_dotexpr(p_index, node);
        default: return EXIT_FAILURE;
    }
}

int parse_loopbinds(uint64_t *p_index, Vector *vars, Vector *exprs) {
    uint64_t index = *p_index;
    char* string;
    int status = EXIT_SUCCESS;
    
    while (1) {
        uint64_t line_index = index;

        if (peek_token(index) == RSQUARE) break;

        if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE) {
            parse_error(MISSING_TOKEN, line_index, index, "[variable]");
            goto structloop1;
        }
        
        ++index;

        if (expect_token(index, COLON, NULL) == EXIT_FAILURE)
            parse_error(MISSING_COLON, index-1, index, NULL);
        else ++index;

        Expr* expr_field = (Expr *) malloc(sizeof(Expr));
        if (!expr_field) MALLOC_FAILURE;
        if(parse_expr(&index, expr_field) == EXIT_FAILURE)
            goto structloop2;

        vector_append(vars, string);
        vector_append(exprs, expr_field);

        if (peek_token(index) == RSQUARE) break;
        else if (expect_token(index, COMMA, NULL) == EXIT_FAILURE) {
            parse_error(MISSING_COMMA, line_index, index, NULL);
            goto structloop1;
        }
        else {
            ++index;
            continue;
        }

    structloop2:
        free(expr_field);
    structloop1:
        free_exprlist(exprs);
        vector_clear(exprs);
        vector_clear(vars);
        status = EXIT_FAILURE;
        while (peek_token(index) != NEWLINE && peek_token(index) != END_OF_FILE) {
            if (peek_token(index) == RSQUARE) break;
            ++index;   
        }
        if (peek_token(index) == NEWLINE || peek_token(index) == RSQUARE || peek_token(index) == END_OF_FILE) break;
    }

    *p_index = index;    
    return status;
}

int is_prefix(uint64_t index) {
    Token *token = vector_get(token_list, index);
    return token->strref.length == 1 && (token->strref.string[0] == '-' || token->strref.string[0] == '!');
}

int is_postfix(uint64_t index) {
    Token *token = vector_get(token_list, index);

    return token->type == LSQUARE || token->type == DOT;
}

int is_binop(uint64_t index, char* *output) {
    Token *token = vector_get(token_list, index);

    char *string = array_from_ref(token->strref);

    for (int i = 0; i < NUM_BINOPS; ++i)
        if (!strcmp(string, binops[i])) {
            *output = array_from_ref(token->strref);
            free(string);
            return 1;
        }

    free(string);
    return 0;
}

uint32_t get_precedence(char *string) {
    if (!string) return 7;

    for (int i = 0; i < PR5_COUNT; ++i)
        if (!strcmp(string, pr5[i])) return 5;
    for (int i = 0; i < PR4_COUNT; ++i)
        if (!strcmp(string, pr4[i])) return 4;
    for (int i = 0; i < PR3_COUNT; ++i)
        if (!strcmp(string, pr3[i])) return 3;
    for (int i = 0; i < PR2_COUNT; ++i)
        if (!strcmp(string, pr2[i])) return 2;
    for (int i = 0; i < PR1_COUNT; ++i)
        if (!strcmp(string, pr1[i])) return 1;

    return 7;
}

void free_expr(Expr* node) {
    if (!node) return;
    switch (node->type) {
        case VAR_EXPR:
            break;
        case ARRAY_EXPR:
            free_exprlist(node->field1.expr_list);
            vector_destroy(node->field1.expr_list);
            break;
        case STRUCTLITERAL_EXPR:
            free_exprlist(node->field2.expr_list);
            vector_destroy(node->field2.expr_list);
            break;
        case DOT_EXPR:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            break;
        case ARRAYINDEX_EXPR:
            free_expr(node->field1.expr);
            free(node->field1.expr);
            free_exprlist(node->field2.expr_list);
            vector_destroy(node->field2.expr_list);
            break;
        case CALL_EXPR:
            free_exprlist(node->field2.expr_list);
            vector_destroy(node->field2.expr_list);
            break;
        case BINOP_EXPR:
            free_expr(node->field3.expr);
            free(node->field3.expr);
            // fallthrough
        case UNOP_EXPR:
            free_expr(node->field2.expr);
            free(node->field2.expr);
            break;
        case IF_EXPR:
            free_expr(node->field1.expr);
            free_expr(node->field2.expr);
            free_expr(node->field3.expr);
            free(node->field1.expr);
            free(node->field2.expr);
            free(node->field3.expr);
            break;
        case ARRAY_LOOP_EXPR:
        case SUM_LOOP_EXPR:
            vector_destroy(node->field1.var_list);
            free_exprlist(node->field2.expr_list);
            vector_destroy(node->field2.expr_list);
            free_expr(node->field3.expr);
            free(node->field3.expr);
            break;
        default: return;
    }
}
void free_exprlist(Vector *list) {
    if (!list) return;
    for (uint64_t i = 0; i < list->size; ++i) {
        Expr *ptr = vector_get(list, i);
        free_expr(ptr);
    }
}

char *expr_string(Expr* node) {
    const char *expr_type = expr_strings[node->type];
    char *string1, *string2, *string3, *output = NULL;

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
        case BINOP_EXPR:
            string1 = node->field1.string;
            string2 = expr_string(node->field2.expr);
            string3 = expr_string(node->field3.expr);
            output = string_combine(8, expr_type, " ", string2, " ", string1, " ", string3, ")");
            free(string2); free(string3);
            break;
        case UNOP_EXPR:
            string1 = node->field1.string;
            string2 = expr_string(node->field2.expr);
            output = string_combine(6, expr_type, " ", string1, " ", string2, ")");
            free(string2);
            break;
        case IF_EXPR:
            string1 = expr_string(node->field1.expr);
            string2 = expr_string(node->field2.expr);
            string3 = expr_string(node->field3.expr);
            output = string_combine(8, expr_type, " ", string1, " ", string2, " ", string3, ")");
            free(string1); free(string2); free(string3);
            break;
        case ARRAY_LOOP_EXPR:
        case SUM_LOOP_EXPR:
            string1 = exprloop_string(node);
            string2 = expr_string(node->field3.expr);
            if (strlen(string1) == 0) output = string_combine(4, expr_type, " ", string2, ")");
            else output = string_combine(6, expr_type, " ", string1, " ", string2, ")");
            break;
        default:
            return "NULL";
    }
    return output;
}
char *exprlist_string(Vector *list) {
    char *temp1, *temp2;
    if (list->size == 0) return calloc(1,1);
    char *output = expr_string(vector_get(list, 0));

    for (size_t i = 1; i < list->size; ++i) {
        temp1 = expr_string(vector_get(list, i));
        temp2 = output;
        output = string_combine(3, temp2, " ", temp1);
        free(temp1);
        free(temp2);
    }

    return output;
}
char *exprloop_string(Expr *node) {
    char *temp1, *temp2;
    Vector *vars = node->field1.var_list;
    Vector *exprs = node->field2.expr_list;
    if (vars->size == 0) return calloc(1,1);
    char *output = string_combine(3, vector_get(vars, 0), " ", expr_string(vector_get(exprs, 0)));
    for (size_t i = 1; i < vars->size; ++i) {
        temp1 = expr_string(vector_get(exprs, i));
        temp2 = output;
        output = string_combine(4, " ", temp2, " ", temp1);
        free(temp1); free(temp2);
    }
    return output;
}
