#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "prod.h"
#include "parser.h"
#include "stringops.h"
#include "vector.h"
#include "error.h"

const char *arg_strings[] = { "(VarArg" };

extern Vector *token_list;

int parse_arg(uint64_t *p_index, Arg* node) {
    uint64_t index = *p_index;
    int status = EXIT_SUCCESS;
    char *string = NULL;
    node -> type = VAR_ARG;

    if (expect_token(index, VARIABLE, &string) == EXIT_FAILURE) {
        parse_error(MISSING_TOKEN, *p_index, index, index+1, "[variable]");
        return EXIT_FAILURE;
    }
    else ++index;

    node -> string = string;
    *p_index = index;
    return status;
}

char *arg_string(Arg* node) {
    const char *type_string = arg_strings[node->type];
    
    return string_combine(4, type_string, " ", node->string, ")");
}

void free_arg(Arg* node) {
    if (!node) return;
    free(node->string);
}
