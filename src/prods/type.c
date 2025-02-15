#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "prod.h"
#include "stringops.h"
#include "parser.h"
#include "vector.h"
#include "error.h"

const char *type_strings[] = { "(IntType", "(FloatType", "(BoolType", "(ArrayType", "(VoidType", "(StructType" };

extern TokenVec *token_vec;

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
            node->field1.string = tokenvec_get(token_vec, index).strref;
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

char *type_string(Type *node) {
    char *string;
    switch(node->type) {
        case FLOAT_TYPE:
            string = malloc(6);
            strcpy(string, "float");
            break;
        case INT_TYPE:
            string = malloc(4);
            strcpy(string, "int");
            break;
        case ARRAY_TYPE:
            string = string_combine(2, type_string(node->field1.type), " array [");
            for (size_t i = 0; i < node->comma_count; ++i) {
                string = string_combine(2, string, ",");
            }
            string = string_combine(2, string, "]");
            break;
        case VOID_TYPE:
            string = malloc(5);
            strcpy(string, "void");
            break;
        case BOOL_TYPE:
            string = malloc(5);
            strcpy(string, "bool");
            break;
        case STRUCT_TYPE:
            string = string_combine(2, "struct ", node->field1.string);
            break;
        default:
            string = calloc(1,1);
            break;
    }

    return string;
}
