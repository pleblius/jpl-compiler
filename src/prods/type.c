#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "prod.h"
#include "stringops.h"
#include "parser.h"
#include "vector.h"
#include "error.h"

const char *type_strings[] = { "(IntType", "(FloatType", "(BoolType", "(ArrayType", "(VoidType", "(StructType" };

extern Vector *token_list;

int parse_type(uint64_t *p_index, Type *node) {
    uint64_t index = *p_index;
    TokenType type = peek_token(index);
    int status = EXIT_SUCCESS;

    switch (type) {
        case INT:
            node->type = INT_TYPE;
            break;
        case FLOAT:
            node->type = FLOAT_TYPE;
            break;
        case BOOL:
            node->type = BOOL_TYPE;
            break;
        case VOID:
            node->type = VOID_TYPE;
            break;
        case VARIABLE:
            node->type = STRUCT_TYPE;
            node->field1.string = array_from_ref(((Token *)vector_get(token_list, index))->strref);
            break;
        default:
            parse_error(INVALID_TYPE, *p_index, index, NULL);
            status = EXIT_FAILURE;
    }
    node->comma_count = 0;
    ++index;

    while (peek_token(index) == LSQUARE) {
        ++index;
        if (parse_arraytype(&index, node) == EXIT_FAILURE)
            status = EXIT_FAILURE;
    }

    *p_index = index;
    return status;
}

int parse_arraytype(uint64_t *p_index, Type *node) {
    uint64_t index = *p_index;
    uint16_t count = 1;
    
    while (peek_token(index) == COMMA) {
        ++index;
        ++count;
    }

    if (expect_token(index, RSQUARE, NULL) == EXIT_FAILURE)
        parse_error(MISSING_BRACKET, *p_index, index, NULL);
    
    ++index;

    Type *new_child = malloc(sizeof(Type));
    if (!new_child) MALLOC_FAILURE;

    memcpy(new_child, node, sizeof(Type));

    node->type = ARRAY_TYPE;
    node->field1.type = new_child;
    node->comma_count = count;
    *p_index = index;
    return EXIT_SUCCESS;
}

void free_type(Type* node){
    if (!node) return;
    switch (node->type) {
        case STRUCT_TYPE:
            break;
        case ARRAY_TYPE:
            free_type(node->field1.type);
            free(node->field1.type);
            break;
        default:
            return;
    }
}

char *type_string(Type* node) {
    const char *type_type = type_strings[node->type];
    char *string1, *string2, *output = NULL;
    int rem;

    switch (node->type) {
        case INT_TYPE:
        case FLOAT_TYPE:
        case BOOL_TYPE:
        case VOID_TYPE:
            output = string_combine(2, type_type, ")");
            break;
        case STRUCT_TYPE:
            string1 = node->field1.string;
            output = string_combine(4, type_type, " ", string1, ")");
            free(string1);
            break;
        case ARRAY_TYPE:
            string2 = (char*) malloc(MAXIMUM_BUFFER); if (!string2) fprintf(stderr, "Buffer error.\n");;
            string1 = type_string(node->field1.type);
            rem = snprintf(string2, MAXIMUM_BUFFER, "%d", node->comma_count); if (rem < 0) fprintf(stderr, "Buffer error.\n");
            output = string_combine(6, type_type, " ", string1, " ", string2, ")");
            free(string1); free(string2);
    }

    return output;
}
