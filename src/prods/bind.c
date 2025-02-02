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
    int status = EXIT_SUCCESS;

    LValue *lvalue_field = (LValue *) malloc(sizeof(LValue));
    if (!lvalue_field) MALLOC_FAILURE;
    
    if (parse_lvalue(&index, lvalue_field) == EXIT_FAILURE) {
        free(lvalue_field);
        lvalue_field = NULL;
        return EXIT_FAILURE;
    }
    
    if (expect_token(index, COLON, NULL) == EXIT_FAILURE) {
        parse_error(MISSING_COLON, index-1, index, index+1, NULL);
        status = EXIT_FAILURE;
    }
    else ++index;

    Type *type_field = (Type *) malloc(sizeof(Type));
    if (!type_field) MALLOC_FAILURE;
    
    if (parse_type(&index, type_field) == EXIT_FAILURE) {
        free(type_field);
        type_field = NULL;
        status = EXIT_FAILURE;
    }

    *p_index = index;
    node->type = VAR_BIND;
    node->field1 = lvalue_field;
    node->field2 = type_field;

    return status;
}

char *bind_string(Bind* node) {
    const char *bind_type = bind_strings[node->type];
    char *string1, *string2, *output = NULL;

    string1 = lvalue_string(node->field1);
    string2 = type_string(node->field2);
    output = string_combine(5, bind_type, string1, " ", string2, "");
    free(string1); free(string2);
    return output;
}

void free_bind(Bind* node) {
    if (!node) return;
    free_lvalue(node->field1);
    free_type(node->field2);
    free(node->field1);
    free(node->field2);
}
