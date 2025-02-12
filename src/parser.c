#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "parser.h"
#include "error.h"
#include "stringops.h"
#include "prod.h"

extern Vector *token_list;
extern Vector *error_list;
Vector *command_list;

int parse() {
    uint64_t index = 0;
    size_t size = token_list -> size;
    TokenType type;

    command_list = vector_create();
    if (!command_list) MALLOC_FAILURE;

    while (index < size) {
        uint64_t start_index = index;
        type = peek_token(index);

        while (type == NEWLINE) {
            ++index;
            type = peek_token(index);
        }

        if (type == END_OF_FILE)
            break;

        Cmd *node = (Cmd *) malloc(sizeof(Cmd));
        if (!node) MALLOC_FAILURE;

        if (parse_cmd(&index, node) == EXIT_FAILURE) {
            free(node);
            node = NULL;

            while (peek_token(index) != NEWLINE && peek_token(index) != END_OF_FILE) {
                ++index;
            }
            continue;
        }

        if (expect_token(index, NEWLINE, NULL) == EXIT_FAILURE) {
            parse_error(MISSING_NEWLINE, start_index, index, NULL);

            while (peek_token(index) != NEWLINE && peek_token(index) != END_OF_FILE) {
                ++index;
            }
        }
        ++index;

        vector_append(command_list, node);
    }

    if (error_list == NULL) return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

TokenType peek_token(uint64_t index) {
    return ((Token *) vector_get(token_list, index)) -> type;
}

int expect_token(uint64_t index, TokenType type, char **p_string) {
    Token *token = vector_get(token_list, index);

    if (token->type != type) return EXIT_FAILURE;

    if (p_string != NULL)
        *p_string = array_from_ref(token->strref);
    
    return EXIT_SUCCESS;
}

void parse_print_output() {
    char *print_string;

    for (size_t i = 0; i < command_list->size; ++i) {
        print_string = cmd_string(vector_get(command_list, i));
        
        printf("%s\n", print_string);

        free(print_string);
    }

    printf("Compilation succeeded.\n");
}

void free_command_list() {
    if (!command_list) return;
    for (size_t i = 0; i < command_list -> size; ++i) {
        free_cmd(vector_get(command_list, i));
    }

    vector_destroy(command_list);
}

