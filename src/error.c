#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "stringops.h"
#include "vecs.h"
#include "error.h"
#include "vector.h"

#define LINE_WIDTH 120
#define PRINT_WIDTH 100

static char *file_string;
static char *file_name;
static Vector *error_tokens;

void error_setup(char *file, char *string) {
    if (!file || !string) return;

    file_string = string;
    file_name = file;
    error_tokens = vector_create();
}

void add_lex_error(LexErrorType type, Token *token) {
    if (!token) return;

    ErrorToken *new_error = malloc(sizeof(ErrorToken)); if (!new_error) exit(EXIT_FAILURE);
    *new_error = (ErrorToken) {LEX_ERROR, {.lex_error = type}, token, NULL};

    vector_append(error_tokens, new_error);
}

void add_parse_error(ParseErrorType type, Token *start_token, Token *end_token) {
    if (!start_token || !end_token) return;
    (void) type;
}

void add_type_error(TypeErrorType type, Token *ref_token, Token *error_token) {
    if (!ref_token || !error_token) return;
    (void) type;
}

void print_errors(TokenVec *tokens) {
    if (!tokens) return;
    
    ErrorToken *error;
    for (size_t i = 0; i < tokens->size; ++i) {
        error = vector_get(error_tokens, i);

        switch (error->error_type) {
            case LEX_ERROR:
                print_lex_error(error, tokens);
                break;
            case PARSE_ERROR:
                print_parse_error(error, tokens);
                break;
            case TYPE_ERROR:
                print_type_error(error, tokens);
                break;
        }
    }
}

void clear_errors() {
    for (size_t i = 0; i < error_tokens->size; ++i) {
        free(vector_get(error_tokens, i));
    }

    vector_destroy(error_tokens);
}

void print_lex_error(ErrorToken *error, TokenVec *tokens) {
    if (!error || !tokens) return;

    Token *token = error->first_token;

    uint32_t line = 1;
    uint32_t col = 1;
    get_error_loc(token, &col, &line);

    printf("Lex error at %s:%d:%d\n\n", file_name, line, col);

    char c;
    switch (error->error_subtype.lex_error) {
        case INVALID_LEX:
            print_one_token_line(token);
            break;
        case ILLEGAL_LEX:
            c = *token->strref.string;
            printf("\tIllegal character: '%d'\n\n", c);
            break;
    }
}

void print_parse_error(ErrorToken *error, TokenVec *tokens) {
    if (!error || !tokens) return;
}

void print_type_error(ErrorToken *error, TokenVec *tokens) {
    if (!error || !tokens) return;
}

void print_one_token_line(Token *token) {
    if (!token) return;
}

void print_two_token_line(Token *start_token, Token *end_token) {
    if (!start_token|| !end_token) return;
}

void get_error_loc(Token *token, uint32_t *col, uint32_t *line) {
    if (!token || !col || !line) return;

    uint32_t loc = token->loc;

    *line = 1;
    *col = 1;

    for (uint32_t i = 0; i < loc; ++i) {
        if (file_string[i] == '\n') {
            ++(*line);
            *col = 1;
        }
        else
            ++(*col);
    }
}
