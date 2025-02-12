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
    char* string;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE) {
        parse_error(INVALID_LVALUE, *p_index, index, NULL);
        return EXIT_FAILURE;
    }
    
    ++index;
    node->type = VAR_LVALUE;
    node->field1.string = string;
    
    if (peek_token(index) == LSQUARE) {
        ++index;

        if (parse_arraylvalue(&index, node) == EXIT_FAILURE)
            goto lvalue_exit1;
        
        ++index;
    }

    *p_index = index;
    node -> field1.string = string;
    return EXIT_SUCCESS;

lvalue_exit1:
    free_lvalue(node);
    *p_index = index;
    return EXIT_FAILURE;
}

int parse_arraylvalue(uint64_t *p_index, LValue *node) {
    uint64_t index = *p_index;
    char* string;
    int status = EXIT_SUCCESS;

    Vector *var_list = vector_create();
    if (!var_list) MALLOC_FAILURE;

    TokenType type;

    while (1) {
        type = peek_token(index);
        if (type == RSQUARE) break;

        if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE) {
            parse_error(MISSING_TOKEN, index, index, "[variable]");
            status = EXIT_FAILURE;
        }
        else ++index;

        vector_append(var_list, string);
        
        type = peek_token(index);

        if (type == COMMA)
            ++index;
        else if (type == RSQUARE) {
            break;
        }
        else {
            parse_error(UNEXPECTED_TOKEN, index, index, NULL);
            break;
        }
    }

    *p_index = index;

    if (status == EXIT_FAILURE) {
        vector_destroy(var_list);
        return EXIT_FAILURE;
    }
    
    node->type = ARRAY_LVALUE;
    node->field2.var_list = var_list;
    return EXIT_SUCCESS;
}

void free_lvalue(LValue* node) {
    if (!node) return;
    switch (node->type) {
        case ARRAY_LVALUE:
            vector_destroy(node->field2.var_list);
            break;
        case VAR_LVALUE:
            break;
    }
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

    for (size_t i = 1; i < list->size; ++i) {
        temp1 = vector_get(list, i);
        temp2 = output;
        output = string_combine(3, temp2, " ", temp1);
        free(temp1);
        free(temp2);
    }

    return output;
}
