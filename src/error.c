#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "stringops.h"
#include "error.h"
#include "vector.h"

#define LINE_WIDTH 120
#define PRINT_WIDTH 100

#define RESET_ALL          "\x1b[0m"

#define COLOR_BLACK        "\x1b[30m"
#define COLOR_RED          "\x1b[31m"
#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW       "\x1b[33m"
#define COLOR_BLUE         "\x1b[34m"
#define COLOR_MAGENTA      "\x1b[35m"
#define COLOR_CYAN         "\x1b[36m"
#define COLOR_WHITE        "\x1b[37m"

#define BACKGROUND_BLACK   "\x1b[40m"
#define BACKGROUND_RED     "\x1b[41m"
#define BACKGROUND_GREEN   "\x1b[42m"
#define BACKGROUND_YELLOW  "\x1b[43m"
#define BACKGROUND_BLUE    "\x1b[44m"
#define BACKGROUND_MAGENTA "\x1b[45m"
#define BACKGROUND_CYAN    "\x1b[46m"
#define BACKGROUND_WHITE   "\x1b[47m"

#define STYLE_BOLD         "\x1b[1m"
#define STYLE_ITALIC       "\x1b[3m"
#define STYLE_UNDERLINE    "\x1b[4m"

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

    ErrorToken *new_error = malloc(sizeof(ErrorToken)); if (!new_error) return;
    *new_error = (ErrorToken) { LEX_ERROR, {.lex_error = type}, token, NULL };

    vector_append(error_tokens, new_error);
}

void add_parse_error(ParseErrorType type, Token *error_token, Token *ref_token) {
    ErrorToken *new_error = malloc(sizeof(ErrorToken)); if (!new_error) return;
    *new_error = (ErrorToken) { PARSE_ERROR, {.parse_error = type}, error_token, ref_token };

    vector_append(error_tokens, new_error);
}

void add_type_error(TypeErrorType type, Token *error_token, Token *ref_token) {
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
    for (size_t i = 0; i < error_tokens->size; ++i) {
        ErrorToken *token = vector_get(error_tokens, i);
        if (token->error_type == PARSE_ERROR && token->error_subtype.parse_error == UNEXPECTED_TOKEN)
            free(token->second_token);
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
        case UNCLOSED_STRING:
            printf("\tUnclosed string:\n\n");
            print_one_token_line(token);
            break;
        case INVALID_LEX:
            print_one_token_line(token);
            break;
        case ILLEGAL_LEX:
            c = *token->strref.string;
            printf("\tIllegal character: " COLOR_RED STYLE_BOLD "'%d'\n\n" RESET_ALL, c);
            break;
    }
}

void print_parse_error(ErrorToken *error, TokenVec *tokens) {
    if (!error || !tokens || !error->first_token) return;

    Token *first_token = error->first_token;
    Token *second_token = error->second_token;

    uint32_t line = 1;
    uint32_t col = 1;
    get_error_loc(first_token, &col, &line);

    printf("Parse error at %s:%d:%d\n\n", file_name, line, col);

    size_t length1 = first_token->strref.length;
    size_t length2 = 0;
    if (second_token != NULL) length2 = second_token->strref.length;
    char misc_1[length1 + 1];
    char misc_2[length2 + 1];

    strncpy(misc_1, first_token->strref.string, length1);
    misc_1[length1] = '\0';

    if (second_token != NULL)
        strncpy(misc_2, second_token->strref.string, length2);
    misc_2[length2] = '\0';

    char *string1;

    switch (first_token->type) {
        case NEWLINE:
            string1 = "NEWLINE";
            break;
        case END_OF_FILE:
            string1 = "END_OF_FILE";
            break;
        default:
            string1 = misc_1;
    }

    char *string2;

    if (second_token == NULL) string2 = misc_2;
    else {
        switch (second_token->type) {
            case NEWLINE:
                string2 = "NEWLINE";
                break;
            case END_OF_FILE:
                string2 = "END_OF_FILE";
                break;
            default:
                string2 = misc_2;
        }
    }

    switch (error->error_subtype.parse_error) {
        case UNEXPECTED_TOKEN:
            printf("\tUnexpected token type encountered.\n\tExpected type '%s'\n\tFound: '%s'\n\n", string2, string1);
            print_one_token_line(first_token);
            break;
        case UNCLOSED_PAREN:
            printf("\tUnclosed parenthetical.\n\n");
            print_two_token_line(second_token, first_token);
            break;
        case INT_RANGE:
            printf("\tInteger value exceeds INTMAX.\n\n");
            print_one_token_line(first_token);
            break;
        case FLOAT_RANGE:
            printf("\tFloating point value exceeds DOUBLEMAX.\n\n");
            print_one_token_line(first_token);
            break;
        case BAD_BIND:
            printf("\tExpected BINDING, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_CMD:
            printf("\tExpected COMMAND, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_EXPR:
            printf("\tExpected EXPRESSION, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_LVALUE:
            printf("\tExpected LVALUE, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_STMT:
            printf("\tExpected STATEMENT, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;  
        case BAD_TYPE:
            printf("\tExpected TYPE, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_UNARY:
            printf("\tInvalid unary operator; expected '-' or '!', found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_BINARY:
            printf("\tInvalid binary operator; expected boolean or math operator, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        default:
            return;
    }
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

    uint32_t post_span = 0;
    while (1) {
        if (token->type == NEWLINE || token->type == END_OF_FILE) break;
        char c = *(string.string + string.length + post_span);
        if (c == '\n' || c == '\0') break;
        ++post_span;
    }

    char prefix[col];
    char postfix[post_span + 1];
    char error[span + 1];

    strncpy(prefix, string.string-(col-1), col);

    if (token->type == NEWLINE || token->type == END_OF_FILE)
        error[0] = ' ';
    else
        strncpy(error, string.string, string.length);

    strncpy(postfix, string.string + string.length, post_span);

    prefix[col-1] = '\0';
    error[span] = '\0';
    postfix[post_span] = '\0';

    printf("%6d | %s" COLOR_RED STYLE_BOLD "%s" RESET_ALL "%s\n", line, prefix, error, postfix);

    size_t prefix_len = strlen(prefix) + 9;
    size_t i;

    printf(COLOR_RED STYLE_BOLD);
    for (i = 0; i < prefix_len; ++i) {
        fputc(' ', stdout);
    }
    putc ('^', stdout);
    for (i = 0; i < span-1; ++i) {
        fputc('~', stdout);
    }
    printf(RESET_ALL "\n\n");
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
