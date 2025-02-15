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
    StringRef string;

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
    StringRef string;
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

        vector_append(var_list, array_from_ref(string));
        
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
