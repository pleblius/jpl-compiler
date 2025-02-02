#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "prod.h"
#include "parser.h"
#include "stringops.h"
#include "vector.h"
#include "error.h"

const char *lvalue_strings[] = { "(VarLValue", "(ArrayLValue" };

extern Vector *token_list;

int parse_lvalue(uint64_t *p_index, LValue *node) {
    uint64_t index = *p_index;
    char *string;
    int status = EXIT_SUCCESS;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE) {
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[variable]");
        return EXIT_FAILURE;
    }
    else ++index;
    node->type = VAR_LVALUE;

    if (peek_token(index) == LSQUARE) {
        ++index;
        if (parse_arraylvalue(&index, node) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }

    node -> field1.string = string;
    *p_index = index;
    return status;
}

int parse_arraylvalue(uint64_t *p_index, LValue *node) {
    uint64_t index = *p_index;
    char *string;

    Vector *var_list = vector_create();
    if (!var_list) MALLOC_FAILURE;

    TokenType type = peek_token(index);
    if (type == RSQUARE) return EXIT_SUCCESS;

    while (1) {
        if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE)
            parse_error(MISSING_TOKEN, index, index, index+1, "[variable]");
        else ++index;
        
        vector_append(var_list, string);
        
        type = peek_token(index);
        if (type == COMMA)
            ++index;
        else if (type == RSQUARE) {
            ++index;
            break;
        }
        else if (type != RSQUARE){
            parse_error(MISSING_BRACKET, *p_index, index, index+1, NULL);
            return EXIT_FAILURE;
        }
    }

    *p_index = index;
    node->type = ARRAY_LVALUE;
    node->field2.var_list = var_list;
    return EXIT_SUCCESS;
}

char *lvalue_string(LValue* node) {
    const char *lvalue_type = lvalue_strings[node->type];
    char *string1, *string2, *output = NULL;

    switch (node->type) {
        case ARRAY_LVALUE:
            string1 = node->field1.string;
            string2 = lvaluelist_string(node->field2.var_list);
            output = string_combine(6, lvalue_type, " ", string1, " ", string2, ")");
            free(string2);
            break;      
        case VAR_LVALUE:
            string1 = node->field1.string;
            output = string_combine(4, lvalue_type, " ", string1, ")");
            break;
    }

    return output;
}

char *lvaluelist_string(Vector *list) {
    char *temp1, *temp2;
    char *output = vector_get(list, 0);

    for (int i = 1; i < list->size; ++i) {
        temp1 = vector_get(list, i);
        temp2 = output;
        output = string_combine(3, temp2, " ", temp1);
        free(temp1);
        free(temp2);
    }

    return output;
}

void free_lvalue(LValue* node) {
    if (!node) return;
    switch (node->type) {
        case ARRAY_LVALUE:
            vector_destroy(node->field2.var_list);
            free(node->field1.string);
            break;
        case VAR_LVALUE:
            free(node->field1.string);
    }
}

void free_lvaluelist(Vector *list) {
    if (!list) return;
    for (int i = 0; i < list->size; ++i) {
        free( ((LValue *) vector_get(list, i)) -> field1.string);
    }

    vector_destroy(list);
}
