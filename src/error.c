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

#define ANSI_RESET_ALL          "\x1b[0m"

#define ANSI_COLOR_BLACK        "\x1b[30m"
#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_COLOR_BLUE         "\x1b[34m"
#define ANSI_COLOR_MAGENTA      "\x1b[35m"
#define ANSI_COLOR_CYAN         "\x1b[36m"
#define ANSI_COLOR_WHITE        "\x1b[37m"

#define ANSI_BACKGROUND_BLACK   "\x1b[40m"
#define ANSI_BACKGROUND_RED     "\x1b[41m"
#define ANSI_BACKGROUND_GREEN   "\x1b[42m"
#define ANSI_BACKGROUND_YELLOW  "\x1b[43m"
#define ANSI_BACKGROUND_BLUE    "\x1b[44m"
#define ANSI_BACKGROUND_MAGENTA "\x1b[45m"
#define ANSI_BACKGROUND_CYAN    "\x1b[46m"
#define ANSI_BACKGROUND_WHITE   "\x1b[47m"

#define ANSI_STYLE_BOLD         "\x1b[1m"
#define ANSI_STYLE_ITALIC       "\x1b[3m"
#define ANSI_STYLE_UNDERLINE    "\x1b[4m"

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
    for (size_t i = 0; i < error_tokens->size; ++i) {
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
            printf("\tIllegal character: " ANSI_COLOR_RED ANSI_STYLE_BOLD "'%d'\n\n" ANSI_RESET_ALL, c);
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

    uint32_t line = 1;
    uint32_t col = 1;
    get_error_loc(token, &col, &line);

    StringRef string = token->strref;
    uint32_t span = string.length;

    // Case 1 - Everything fits in one line
    if (col + span < LINE_WIDTH) {
        char prefix[col];
        char postfix[LINE_WIDTH - span - col + 1];
        char error[span + 1];

        strncpy(prefix, string.string-(col-1), col);
        strncpy(error, string.string, string.length);
        strncpy(postfix, string.string + string.length, LINE_WIDTH - span - col);

        prefix[col-1] = '\0';
        error[span] = '\0';
        postfix[LINE_WIDTH - span - col] = '\0';

        printf("%6d | %4d  %s" ANSI_COLOR_RED ANSI_STYLE_BOLD "%s" ANSI_RESET_ALL "%s\n\n", line, col, prefix, error, postfix);
    }
    // Case 2 - Everything fits but needs to be shifted

    // Case 3 - Lines need to be split
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
