#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "prod.h"
#include "parser.h"
#include "vector.h"
#include "error.h"
#include "token.h"

const char *bind_strings[] = { "" };

int parse_bind(uint64_t* p_index, Bind *node) {
    uint64_t index = *p_index;

    LValue *lvalue_field = (LValue *) malloc(sizeof(LValue));
    if (!lvalue_field) MALLOC_FAILURE;
    if (parse_lvalue(&index, lvalue_field) == EXIT_FAILURE)
        goto bind_exit1;
    
    if (expect_token(index, COLON, NULL) == EXIT_FAILURE)
        parse_error(MISSING_COLON, index-1, index, NULL);
    else ++index;

    Type *type_field = (Type *) malloc(sizeof(Type));
    if (!type_field) MALLOC_FAILURE;
    if (parse_type(&index, type_field) == EXIT_FAILURE)
        goto bind_exit2;

    *p_index = index;
    node->type = VAR_BIND;
    node->field1 = lvalue_field;
    node->field2 = type_field;
    return EXIT_SUCCESS;

bind_exit2:
    free_lvalue(lvalue_field);
    free(type_field);
bind_exit1:
    free(lvalue_field);
    *p_index = index;
    return EXIT_FAILURE;
}

void free_bind(Bind* node) {
    if (!node) return;
    free_lvalue(node->field1);
    free_type(node->field2);
    free(node->field1);
    free(node->field2);
}
