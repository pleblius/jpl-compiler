#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "parser.h"
#include "error.h"
#include "stringops.h"

#define MAXIMUM_BUFFER 1024

extern Vector *token_list;
Vector *command_list;

const char *cmd_strings[] = {
    "(ReadCmd", 
    "(WriteCmd", 
    "(LetCmd", 
    "(AssertCmd", 
    "(PrintCmd", 
    "(ShowCmd", 
    "(TimeCmd" };
const char *expr_strings[] = {
    "(IntExpr", 
    "(FloatExpr", 
    "(TrueExpr", 
    "(FalseExpr", 
    "(VarExpr", 
    "(ArrayLiteralExpr" };
const char *arg_strings[] = {"(VarArg"};
const char *lvalue_strings[] = {"(VarLValue"};
const char *type_strings[] = {"VarType"};

int parse() {
    uint64_t index = 0;
    size_t size = token_list -> size;
    TokenType type;

    command_list = vector_create();
    if (!command_list) {
        fprintf(stderr, "Failed to initialize list.\n");
        return EXIT_FAILURE;
    }

    while (index < size) {
        type = peek_token(index);

        while (type == NEWLINE) {
            ++index;
            type = peek_token(index);
        }

        if (type == END_OF_FILE)
            return EXIT_SUCCESS;

        Cmd *node = (Cmd *) malloc(sizeof(Cmd));
        if (!node) {
            fprintf(stderr, "Malloc failure.\n");
            exit(EXIT_FAILURE);
        }

        if (parse_cmd(&index, node) == EXIT_FAILURE) {
            free(node);
            return EXIT_FAILURE;
        }

        vector_append(command_list, node);
        if (expect_token(index++, NEWLINE, NULL) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int parse_cmd(uint64_t* p_index, Cmd* node) {
    TokenType type = peek_token(*p_index);

    if (!IS_COMMAND(type)) {
        parse_error(INVALID_COMMAND, type, vector_get(token_list, *p_index));
        return EXIT_FAILURE;
    }

    switch (type) {
        case (READ):
            return parse_readcmd(p_index, node);
        case (WRITE):
            return parse_writecmd(p_index, node);
        case (LET):
            return parse_letcmd(p_index, node);
        case (ASSERT):
            return parse_assertcmd(p_index, node);
        case (PRINT):
            return parse_printcmd(p_index, node);
        case (SHOW):
            return parse_showcmd(p_index, node);
        case (TIME):
            return parse_timecmd(p_index, node);
        default:
            parse_error(INVALID_COMMAND, type, vector_get(token_list, *p_index));
            return EXIT_FAILURE;
    }
}

int parse_readcmd(uint64_t* p_index, Cmd* node) {
    uint64_t index = *p_index + 1;
    char *string;
    node -> type = READ_CMD;

    if (expect_token(index++, IMAGE, NULL) == EXIT_FAILURE 
            || expect_token(index++, STRING, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if (expect_token(index++, TO, NULL) == EXIT_FAILURE)
        goto readcmd_exit1;

    LValue *lvalue_field = (LValue *) malloc(sizeof(LValue));
    if (!lvalue_field) {
        fprintf(stderr, "Malloc failure.\n");
        exit(EXIT_FAILURE);
    }

    if (parse_lvalue(&index, lvalue_field) == EXIT_FAILURE)
        goto readcmd_exit2;

    node -> field1.string = string;
    node -> field2.lvalue = lvalue_field;
    *p_index = index;
    return EXIT_SUCCESS;

readcmd_exit2:
    free(lvalue_field);
readcmd_exit1:
    free(string);
    return EXIT_FAILURE;
}

int parse_writecmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index + 1;
    char *string;
    node -> type = WRITE_CMD;

    if (expect_token(index++, IMAGE, NULL) == EXIT_FAILURE)
        return EXIT_FAILURE;
    
    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) {
        fprintf(stderr, "Malloc failure.\n");
        exit(EXIT_FAILURE);
    }

    if (parse_expr(&index, expr_field) == EXIT_FAILURE)
        goto writecmd_exit1;
    if (expect_token(index++, TO, NULL) == EXIT_FAILURE 
            ||expect_token(index++, STRING, &string) == EXIT_FAILURE)
        goto writecmd_exit2;

    node -> field1.expr = expr_field;
    node -> field2.string = string;
    *p_index = index;
    return EXIT_SUCCESS;

writecmd_exit2:
    free_expr(expr_field);
writecmd_exit1:
    free(expr_field);
    return EXIT_FAILURE;
}

int parse_letcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index + 1;
    node -> type = LET_CMD;

    LValue *lvalue_field = (LValue *) malloc(sizeof(LValue));
    if (!lvalue_field) {
        fprintf(stderr, "Malloc failure\n.");
        exit(EXIT_FAILURE);
    }

    if (parse_lvalue(&index, lvalue_field) == EXIT_FAILURE)
        goto letcmd_exit1;
    if (expect_token(index++, EQUALS, NULL) == EXIT_FAILURE)
        goto letcmd_exit2;

    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if(!expr_field) {
        fprintf(stderr, "Malloc failure.\n");
        exit(EXIT_FAILURE);
    }
    if (parse_expr(&index, expr_field) == EXIT_FAILURE)
        goto letcmd_exit3;

    node -> field1.lvalue = lvalue_field;
    node -> field2.expr = expr_field;
    *p_index = index;
    return EXIT_SUCCESS;

letcmd_exit3:
    free(expr_field);
letcmd_exit2:
    free_lvalue(lvalue_field);
letcmd_exit1:
    free(lvalue_field);
    return EXIT_FAILURE;
}

int parse_assertcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index + 1;
    char *string;
    node -> type = ASSERT_CMD;

    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) {
        fprintf(stderr, "Malloc failure\n.");
        exit(EXIT_FAILURE);
    }
    if (parse_expr(&index, expr_field) == EXIT_FAILURE)
        goto assertcmd_exit1;
    if (expect_token(index++, COMMA, NULL) == EXIT_FAILURE
            || expect_token(index++, STRING, &string) == EXIT_FAILURE)
        goto assertcmd_exit2;

    node -> field1.expr = expr_field;
    node -> field2.string = string;
    *p_index = index;
    return EXIT_SUCCESS;

assertcmd_exit2:
    free_expr(expr_field);
assertcmd_exit1:
    free(expr_field);
    return EXIT_FAILURE;
}

int parse_printcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index + 1;
    char *string;
    node -> type = PRINT_CMD;

    if (expect_token(index++, STRING, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;

    node->field1.string = string;
    node->field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_showcmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index + 1;
    node -> type = SHOW_CMD;
    
    Expr *expr_field = (Expr *) malloc(sizeof(Expr));
    if (!expr_field) {
        fprintf(stderr, "Malloc failure.\n");
        exit(EXIT_FAILURE);
    }

    if (parse_expr(&index, expr_field) == EXIT_FAILURE) {
        free(expr_field);
        return EXIT_FAILURE;
    }

    node -> field1.expr = expr_field;
    node -> field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_timecmd(uint64_t *p_index, Cmd* node) {
    uint64_t index = *p_index + 1;
    node -> type = TIME_CMD;

    Cmd *cmd_field = (Cmd *) malloc(sizeof(Cmd));
    if (!cmd_field) {
        fprintf(stderr, "Malloc failure.\n");
        exit(EXIT_FAILURE);
    }

    if (parse_cmd(&index, cmd_field) == EXIT_FAILURE) {
        free(cmd_field);
        return EXIT_FAILURE;
    }

    node -> field1.cmd = cmd_field;
    node -> field2.string = NULL;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_arg(uint64_t *p_index, Arg* node) {
    uint64_t index = *p_index;
    char *string = NULL;
    node -> type = VAR_ARG;

    if (expect_token(index++, VARIABLE, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;

    node -> string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_lvalue(uint64_t *p_index, LValue *node) {
    uint64_t index = *p_index;
    char *string = NULL;
    node -> type = VAR_LVALUE;

    if (expect_token(index++, VARIABLE, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;

    node -> string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_type(uint64_t *p_index, Type *node) {
    uint64_t index = *p_index + 1;
    char *string;
    node -> type = VAR_TYPE;

    if (expect_token(index++, VARIABLE, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;
    
    node -> string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}

int parse_expr(uint64_t *p_index, Expr* node) {
    TokenType type = peek_token(*p_index);

    if (!IS_EXPR(type)) {
        parse_error(INVALID_EXPRESSION, type, vector_get(token_list, *p_index));
        return EXIT_FAILURE;
    }

    switch (type) {
        case INTVAL:
            return parse_intexpr(p_index, node);
        case FLOATVAL:
            return parse_floatexpr(p_index, node);
        case TRUE:
            return parse_trueexpr(p_index, node);
        case FALSE:
            return parse_falseexpr(p_index, node);
        case LSQUARE:
            return parse_arrayexpr(p_index, node);
        case VARIABLE:
            return parse_varexpr(p_index, node);
        default:
            parse_error(INVALID_EXPRESSION, type, vector_get(token_list, *p_index));
            return EXIT_FAILURE;
    }
}
int parse_intexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    char *string, *end;
    node -> type = INT_EXPR;

    if (expect_token(index++, INTVAL, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;
    
    uint64_t intval = strtol(string, &end, 10);
    free(string);
    if (errno == ERANGE) {
        parse_error(OUT_OF_RANGE, INTVAL, vector_get(token_list, index-1));
        return EXIT_FAILURE;
    }

    node -> field1.int_value = intval;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_floatexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    char *string, *end;
    node -> type = FLOAT_EXPR;
    
    if (expect_token(index++, FLOATVAL, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;

    double floatval = strtod(string, &end);
    free(string);
    if (errno == ERANGE) {
        parse_error(OUT_OF_RANGE, FLOATVAL, vector_get(token_list, index-1));
        return EXIT_FAILURE;
    }

    node -> field1.float_value = floatval;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_trueexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    node -> type = TRUE_EXPR;

    if (expect_token(index++, TRUE, NULL) == EXIT_FAILURE)
        return EXIT_FAILURE;
    
    node -> field1.int_value = 1;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_falseexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    node -> type = FALSE_EXPR;

    if (expect_token(index++, FALSE, NULL) == EXIT_FAILURE)
        return EXIT_FAILURE;
    
    node -> field1.int_value = 0;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_varexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index;
    char *string;
    node->type = VAR_EXPR;

    if (expect_token(index++, VARIABLE, &string) == EXIT_FAILURE)
        return EXIT_FAILURE;
        
    node -> field1.string = string;
    *p_index = index;
    return EXIT_SUCCESS;
}
int parse_arrayexpr(uint64_t *p_index, Expr* node) {
    uint64_t index = *p_index + 1;
    node->type = ARRAY_EXPR;
    
    Vector *expr_list = vector_create();
    node->field1.expr_list = expr_list;

    while (1) {
        if (peek_token(index) == RSQUARE) {
            ++index;
            break;
        }

        Expr *expr_node = (Expr *) malloc(sizeof(Expr));
        if (!expr_node) {
            fprintf(stderr, "Failed to initialize list.\n");
            exit(EXIT_FAILURE);
        }

        if (parse_expr(&index, expr_node) == EXIT_FAILURE) {
            free(expr_node);
            return EXIT_FAILURE;
        }
        vector_append(expr_list, expr_node);
        
        TokenType next = peek_token(index++);
        if (next == RSQUARE) break;
        else if (next == COMMA) continue;
        else {
            parse_error(INVALID_TOKEN, RSQUARE, vector_get(token_list, index-1));
            EXIT_FAILURE;
        }
    }

    *p_index = index;
    return EXIT_SUCCESS;
}

TokenType peek_token(uint64_t index) {
    return ((Token *) vector_get(token_list, index)) -> type;
}

int expect_token(uint64_t index, TokenType type, char **p_string) {
    Token *token = vector_get(token_list, index);

    if (token->type != type) {
        parse_error(INVALID_TOKEN, type, token);
        return EXIT_FAILURE;
    }

    if (p_string != NULL) {
        if (token->string == NULL) {
            parse_error(MISSING_STRING, type, token);
            return EXIT_FAILURE;
        }
        *p_string = string_dup(token->string);
    }
    
    return EXIT_SUCCESS;
}

void parse_print_output() {
    char *print_string;

    for (int i = 0; i < command_list->size; ++i) {
        print_string = cmd_string(vector_get(command_list, i));
        
        printf("%s\n", print_string);

        free(print_string);
    }

    printf("Compilation succeeded.\n");
}

char *cmd_string(Cmd* node) {
    const char *type_string = cmd_strings[node->type];
    char *string1, *string2, *output = NULL;

    switch (node->type) {
        case READ_CMD:
            string1 = node->field1.string;
            string2 = lvalue_string(node->field2.lvalue); if (!string1 || !string2) return NULL;
            output = string_combine(6, type_string, " ", string1, " ", string2, ")");
            free(string2);
            break;
        case WRITE_CMD:
            string1 = expr_string(node->field1.expr);
            string2 = node->field2.string; if (!string1 || !string2) return NULL;
            output = string_combine(6, type_string, " ", string1, " ", string2, ")");
            free(string1);
            break;
        case LET_CMD:
            string1 = lvalue_string(node->field1.lvalue);
            string2 = expr_string(node->field2.expr); if (!string1 || !string2) return NULL;
            output = string_combine(6, type_string, " ", string1, " ", string2, ")");
            free(string1); free(string2);
            break;
        case ASSERT_CMD:
            string1 = expr_string(node->field1.expr);
            string2 = node->field2.string; if (!string1 || !string2) return NULL;
            output = string_combine(6, type_string, " ", string1, " ", string2, ")");
            free(string1);
            break;
        case PRINT_CMD:
            string1 = node->field1.string; if (!string1) return NULL;
            output = string_combine(4, type_string, " ", string1, ")");
            break;
        case SHOW_CMD:
            string1 = expr_string(node->field1.expr); if (!string1) return NULL;
            output = string_combine(4, type_string, " ", string1, ")");
            free(string1);
            break;
        case TIME_CMD:
            string1 = cmd_string(node->field1.cmd); if (!string1) return NULL;
            output = string_combine(4, type_string, " ", string1, ")");
            free(string1);
            break;
    }

    return output;
}

char *expr_string(Expr* node) {
    const char *type_string = expr_strings[node->type];
    char *string1, *output = NULL;

    int rem;
    
    switch (node->type) {
        case INT_EXPR:
            string1 = malloc(MAXIMUM_BUFFER); if (!string1) return NULL;
            rem = snprintf(string1, MAXIMUM_BUFFER, "%ld", node->field1.int_value); if (rem < 0) fprintf(stderr, "Buffer error.\n");
            output = string_combine(4, type_string, " ", string1, ")");
            free(string1);
            break;
        case FLOAT_EXPR:
            string1 = malloc(MAXIMUM_BUFFER); if (!string1) return NULL;
            rem = snprintf(string1, MAXIMUM_BUFFER, "%ld", (uint64_t)node->field1.float_value); if (rem < 0) fprintf(stderr, "Buffer error.\n");
            output = string_combine(4, type_string, " ", string1, ")");
            free(string1);
            break;
        case FALSE_EXPR:
        case TRUE_EXPR:
            output = string_combine(2, type_string, ")");
            break;
        case VAR_EXPR:
            output = string_combine(4, type_string, " ", node->field1.string, ")");
            break;
        case ARRAY_EXPR:
            string1 = array_expr_string(node);
            if (strlen(string1) == 0) output = string_combine(2, type_string, ")");
            else output = string_combine(4, type_string, " ", string1, ")");
            free(string1);
    }
    return output;
}
char *array_expr_string(Expr* node) {
    char *temp1, *temp2;
    Vector *list = node -> field1.expr_list;
    if (list->size == 0) return calloc(1,1);
    char *output = expr_string(vector_get(list, 0));
    size_t len = strlen(output) + 1;

    for (int i = 1; i < list->size; ++i) {
        temp1 = expr_string(vector_get(list, i));
        temp2 = output;
        len += strlen(temp1) + 1;
        output = string_combine(3, temp2, " ", temp1);
        free(temp1);
        free(temp2);
    }

    return output;
}

char *arg_string(Arg* node) {
    const char *type_string = arg_strings[node->type];
    
    return string_combine(4, type_string, " ", node->string, ")");
}

char *lvalue_string(LValue* node) {
    const char *type_string = lvalue_strings[node->type];

    return string_combine(4, type_string, " ", node->string, ")");
}

char *type_string(Type* node) {
    const char *type_string = type_strings[node->type];

    return string_combine(4, type_string, " ", node->string, ")");
}

void free_command_list() {
    if (!command_list) return;
    for (int i = 0; i < command_list -> size; ++i) {
        free_cmd(vector_get(command_list, i));
    }

    vector_destroy(command_list);
}

void free_cmd(Cmd* node) {
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
    }
}
void free_expr(Expr* node) {
    if (!node) return;
    switch (node->type) {
        case VAR_EXPR:
            free(node->field1.string);
            break;
        case ARRAY_EXPR:
            for (int i=0; i < node->field1.expr_list->size; ++i) {
                Expr* ptr = vector_get(node->field1.expr_list, i);
                free_expr(ptr);
            }
            vector_destroy(node->field1.expr_list);
            break;
        default: return;
    }
}
void free_arg(Arg* node) {
    free(node->string);
}
void free_lvalue(LValue* node) {
    free(node->string);
}
void free_type(Type* node){
    free(node->string);
}


