#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "error.h"
#include "stringops.h"
#include "vector.h"

#define MAXIMUM_BUFFER 1024
#define OUTPUT stdout

Vector *error_list;

void program_error(char *file_string, char *file_name, Vector *token_list) {
    size_t line;
    size_t col;

    for (int i = 0; i < error_list->size; ++i) {
        ErrorStruct *error = vector_get(error_list, i);
        get_loc(error->error, file_string, &line, &col);

        print_error_desc(file_name, error, token_list, line, col);
        
        if (error->type != LEX_ERROR && error->subtype.lex_error != ILLEGAL_CHARACTER)
        print_error_line(file_string, error, line);
    }

    fprintf(OUTPUT, "\nCompilation failed.\n");
}

void parse_error(ParseErrorType subtype, size_t start_index, size_t error_index, size_t end_index, char *misc) {
    ErrorStruct *error = malloc(sizeof(ErrorStruct));
    if (!error) exit(EXIT_FAILURE);

    error->start = start_index;
    error->error = error_index;
    error->end = end_index;
    error->type = PARSE_ERROR;
    error->subtype.parse_error = subtype;

    if (misc != NULL) error->misc_string = misc;

    if (!error_list) {
        error_list = vector_create();
        if (!error_list) exit(EXIT_FAILURE);
    }
    vector_append(error_list, error);
}
void lex_error(LexErrorType subtype, size_t start_index, size_t error_index, size_t end_index, char *misc) {
    ErrorStruct *error = malloc(sizeof(ErrorStruct));
    if (!error) exit(EXIT_FAILURE);

    error->start = start_index;
    error->error = error_index;
    error->end = end_index;
    error->type = LEX_ERROR;
    error->subtype.lex_error = subtype;

    if (misc != NULL) error->misc_string = misc;

    if (!error_list) {
        error_list = vector_create();
        if (!error_list) exit(EXIT_FAILURE);
    }
    vector_append(error_list, error);
}


void print_error_desc(char *file_name, ErrorStruct *error, Vector *token_list, size_t line, size_t col) {
    switch(error->type) {
        case PARSE_ERROR:
            fprintf(OUTPUT, "\nParsing error at\t %s:%ld:%ld\n\t", file_name, line, col);

            print_parse_error_desc(error, token_list);
            break;
        case LEX_ERROR:
            fprintf(OUTPUT, "\nLexing error at\t %s:%ld:%ld\n\t", file_name, line, col);

            print_lex_error_desc(error, token_list);
            break;
    }
}

void print_parse_error_desc(ErrorStruct *error, Vector *token_list) {
    Token *start_token = vector_get(token_list, error->start);
    Token *error_token = vector_get(token_list, error->error);
    char *start_string = array_from_ref(start_token->strref);
    char *error_string = array_from_ref(error_token->strref);
    char *misc_string = "";

    if (error->misc_string != NULL) misc_string = error->misc_string;

    switch (error->subtype.parse_error) {
        case INVALID_ARG:
            fprintf(OUTPUT, "Unable to parse ARGUMENT.\n\tExpected: '[variable]'.\n\tFound: '%s'.\n", error_string);
            break;
        case INVALID_BIND:
            fprintf(OUTPUT, "Unable to parse VARIABLE BINDING.\n\tExpected: '[LVALUE] : [TYPE]'.\n\tFound: '%s'.\n", error_string);
            break;
        case INVALID_CMD:
            fprintf(OUTPUT, "Unable to parse COMMAND.\n\tExpected: 'read', 'write', 'let', 'assert', 'print', 'show', 'time', 'fn', 'struct'.\n");
            fprintf(OUTPUT, "\tFound: '%s'.\n", error_string);
            break;
        case INVALID_EXPR:
            fprintf(OUTPUT, "Unable to parse EXPRESSION.\n\tExpected: '[integer]', '[float]', 'true', 'false', 'void' ");
            fprintf(OUTPUT, "'[variable]', '[arrayliteral]', '[structliteral]', '[EXPRESSION].[variable]', '[arrayindex]', '[function call]'.\n\t");
            fprintf(OUTPUT, "Found: '%s'.\n", error_string);
            break;
        case INVALID_LVALUE:
            fprintf(OUTPUT, "Unable to parse LVALUE.\n\tExpected: '[variable]' or [variable][array]'.\n\t");
            fprintf(OUTPUT, "Found: '%s'.\n.", error_string);
            break;
        case INVALID_STMT:
            fprintf(OUTPUT, "Unable to parse STATEMENT.\n'\tExpected: 'let', 'assert', 'return'.\n\tFound: '%s'.\n", error_string);
            break;
        case INVALID_TYPE:
            fprintf(OUTPUT, "Unable to parse TYPE.\n\tExpected: 'int', 'float', 'bool', 'void', '[struct variable]' or 'TYPE[array]'.\n\t");
            fprintf(OUTPUT, "Found: '%s'.\n", error_string);
            break;
        case UNEXPECTED_TOKEN:
            fprintf(OUTPUT, "Unexpected token: '%s'.\n", error_string);
            break;
        case MISSING_TOKEN:
            fprintf(OUTPUT, "Missing expected token.\n\tExpected: '%s'.\n\tFound: '%s.\n", misc_string, error_string);
            break;
        case INT_RANGE:
            fprintf(OUTPUT, "Integer value out of range.\n\tValue: '%s'\n", error_string);
            break;
        case FLOAT_RANGE:
            fprintf(OUTPUT, "Floating point value out of range.\n\tValue: '%s'\n", error_string);
            break;
        case MISSING_BRACE:
            fprintf(OUTPUT, "Unclosed brace.\n\tExpected: '}'.\n\tFound: '%c'.\n", error_string[0]);
            break;
        case MISSING_PAREN:
            fprintf(OUTPUT, "Unclosed parenthesis.\n\tExpected: ')'.\n\tFound: '%c'.\n", error_string[0]);
            break;
        case MISSING_BRACKET:
            fprintf(OUTPUT, "Unclosed bracket.\n\tExpected: ']'.\n\tFound: '%c'.\n", error_string[0]);
            break;
        case MISSING_NEWLINE:
            fprintf(OUTPUT, "Missing newline.\n\tExpected: 'Newline'.\n\tFound: '%s'.\n", error_string);
            break;
        case MISSING_COLON:
            fprintf(OUTPUT, "Missing colon in binding.\n\tExpected ':' in '%s %s'.\n", start_string, error_string);
            break;
        case MISSING_STRING:
            fprintf(OUTPUT, "Missing string literal.\n\tExpected: '\"[string]\"' before %s.\n", error_string);
            break;
        case MISSING_COMMA:
            fprintf(OUTPUT, "Missing comma in bindings.\n\tExpected ',' in '%s %s'.\n", start_string, error_string);
            break;
    }

    free(start_string);
    free(error_string);
}

void print_lex_error_desc(ErrorStruct *error, Vector *token_list) {
    Token *token = vector_get(token_list, error->error);
    char *string = array_from_ref(token->strref);

    switch (error->subtype.lex_error) {
        case INVALID_STRING:
            fprintf(OUTPUT, "Invalid string.\n\tExpected: '\"'.\n\tFound '%c'.\n", string[0]);
            break;
        case INVALID_ESCAPE:
            fprintf(OUTPUT, "Invalid escape.\n\tExpected: 'Newline'. Found '%s'.\n", string);
            break;
        case ILLEGAL_CHARACTER:
            fprintf(OUTPUT, "Illegal character 'u+%d'.\n", (int)(string[0]));
            break;
        case INVALID_OPERATOR:
            fprintf(OUTPUT, "Invalid operator.\n\tExpected: '&&', '||'.\n\tFound: '%s'.\n", string);
            break;
        case INVALID_COMMENT:
            fprintf(OUTPUT, "Invalid comment: '%s'.\n", string);
    }

    free(string);
}

void print_error_line(char *p_c, ErrorStruct *error, size_t line) {
    size_t line_start = 0;
    size_t line_end = 0;
    size_t i;
    for (i = error->start; i > 0; --i) {
        if (p_c[i] == '\n') {
            line_start = i;
            break;
        }
    }
    for (i = error->end; i > 0; ++i) {
        if (p_c[i] == '\n' || p_c[i] == EOF || p_c[i] == '\0') {
            line_end = i;
            break;
        }
    }

    size_t line_length = line_end - line_start + 10;

    if (line_length + 1> MAXIMUM_BUFFER) {
        fprintf(OUTPUT, "Line too long to print:\n%ld |\n", line);
        return;
    }

    size_t print_end = 0;
    for (size_t i = error->end; i > error->error; --i) {
        if (p_c[i] == ' ' || p_c[i] == '\n') {
            print_end = i;
            break;
        }
    }

    char *pre_line = malloc(MAXIMUM_BUFFER);
    if (!pre_line) return;
    strncpy(pre_line, p_c + line_start, error->start - line_start);
    pre_line[error->start - line_start] = '\0';

    char *err_line = malloc(MAXIMUM_BUFFER);
    if (!err_line) return;
    strncpy(err_line, p_c + error->start, error->end - error->start);
    err_line[error->end - error->start] = '\0';

    char *post_line = malloc(MAXIMUM_BUFFER);
    if (!post_line) return;
    strncpy(post_line, p_c + print_end, line_end - error->end);
    post_line[line_end - error->end] = '\0';

    fprintf(OUTPUT, "%ld | %s", line, pre_line);
    fprintf(OUTPUT, RED_BOLD);
    fprintf(OUTPUT, "%s", err_line);
    fprintf(OUTPUT, RESET);
    fprintf(OUTPUT, "%s", post_line);
    
    for (i = line_start; i < error->start; ++i) {
        fprintf(OUTPUT, " ");
    }
    fprintf(OUTPUT, RED_BOLD);
    for (; i < error->error; ++i) {
        fprintf(OUTPUT, "~");
    }
    fprintf(OUTPUT, "^");
    for (; i < error->end; ++i) {
        fprintf(OUTPUT, "~");
    }
    fprintf(OUTPUT, RESET);
    fprintf(OUTPUT, "\n");

    free(pre_line);
    free(post_line);
    free(err_line);
}

void get_loc(size_t byte, char *p_c, size_t *p_line, size_t *p_col) {
    size_t line = 1;
    size_t col = 1;

    for (size_t i = 0; i < byte; ++i) {
        if (p_c[i] == '\n') {
            col = 1;
            ++line;    
        }
        else {
            ++col;
        }
    }

    *p_line = line;
    *p_col = col;
}
