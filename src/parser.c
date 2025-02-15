#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "error.h"
#include "stringops.h"

extern Vector *command_list;
static TokenVec *token_vec;
int fail_status = EXIT_SUCCESS;

int parse(TokenVec *vector) {
    if (!vector) return EXIT_FAILURE;

    uint64_t index = 0;
    token_vec = vector;
    size_t size = vector -> size;
    TokenType type;

    command_list = vector_create_cap(size/2);
    if (!command_list) MALLOC_FAILURE;

    while (index < size) {
        type = peek_token(index);

        while (type == NEWLINE) {
            type = peek_token(++index);
        }

        if (type == END_OF_FILE)
            break;

        Cmd *node = malloc(sizeof(Cmd));

        if (parse_cmd(&index, node) == EXIT_FAILURE) {
            fail_status = EXIT_FAILURE;

            while (peek_token(index) != NEWLINE && peek_token(index) != END_OF_FILE) {
                ++index;
            }
        }

        if (expect_token(index, NEWLINE, NULL) == EXIT_FAILURE) {
            fail_status = EXIT_FAILURE;

            while (peek_token(index) != NEWLINE && peek_token(index) != END_OF_FILE) {
                ++index;
            }
        }
        else ++index;

        vector_append(command_list, node);
    }

    return fail_status;
}

TokenType peek_token(uint64_t index) {
    if (index >= tokenvec_size(token_vec)) return INVALID;
    return tokenvec_get(token_vec, index).type;
}

int expect_token(uint64_t index, TokenType type, StringRef *string) {
    if (index >= tokenvec_size(token_vec)) return EXIT_FAILURE;

    Token token = tokenvec_get(token_vec, index);
    if (token.type != type) {
        parse_error(UNEXPECTED_TOKEN, index, index, NULL);
        set_fail(EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    
    if (string != NULL)
        *string = token.strref;
    return EXIT_SUCCESS;
}

void set_fail(int status) {
    fail_status = status;
}
