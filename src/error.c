#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "error.h"
#include "stringops.h"
#include "vector.h"

#define MAXIMUM_BUFFER 1024
#define OUTPUT stdout

Vector *error_list;
extern size_t file_size;
extern Vector *token_list;
extern char *file_string;

void program_error(char *file_name) {
    size_t line;
    size_t col;

    for (size_t i = 0; i < error_list->size; ++i) {
        ErrorStruct *error = vector_get(error_list, i);
        Token *error_token = vector_get(token_list, error->error);
        Token *start_token = vector_get(token_list, error->start);

        get_loc(error_token->byte, file_string, &line, &col);

        print_error_desc(file_name, error, token_list, line, col);
        
        if (error->type != LEX_ERROR || error->subtype.lex_error != ILLEGAL_CHARACTER)
        print_error_line(file_string, start_token->byte, error_token->byte, line, error);
    }

    fprintf(OUTPUT, "\nCompilation failed\n");
}

void parse_error(ParseErrorType subtype, size_t start_index, size_t error_index, char *misc) {
    ErrorStruct *error = malloc(sizeof(ErrorStruct));
    if (!error) exit(EXIT_FAILURE);

    error->start = start_index;
    error->error = error_index;
    error->type = PARSE_ERROR;
    error->subtype.parse_error = subtype;

    if (misc != NULL) error->misc_string = misc;

    if (!error_list) {
        error_list = vector_create();
        if (!error_list) exit(EXIT_FAILURE);
    }
    vector_append(error_list, error);
}
void lex_error(LexErrorType subtype, size_t start_index, size_t error_index, char *misc) {
    ErrorStruct *error = malloc(sizeof(ErrorStruct));
    if (!error) exit(EXIT_FAILURE);

    error->start = start_index;
    error->error = error_index;
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
            fprintf(OUTPUT, "\nParsing error at %s:%ld:%ld\n\t", file_name, line, col);

            print_parse_error_desc(error, token_list);
            break;
        case LEX_ERROR:
            fprintf(OUTPUT, "\nLexing error at %s:%ld:%ld\n\t", file_name, line, col);

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

    if (!strcmp(error_string, "\n")) {
        free(error_string);
        error_string = malloc(8);
        strcpy(error_string, "Newline");
    }

    if (error->misc_string != NULL) misc_string = error->misc_string;

    switch (error->subtype.parse_error) {
        case INVALID_ARG:
            fprintf(OUTPUT, "Unable to parse ARGUMENT\n\tExpected: '[variable]'\n\tFound: '%s'\n", error_string);
            break;
        case INVALID_BIND:
            fprintf(OUTPUT, "Unable to parse VARIABLE BINDING\n\tExpected: '[LVALUE] : [TYPE]'\n\tFound: '%s'\n", error_string);
            break;
        case INVALID_CMD:
            fprintf(OUTPUT, "Unable to parse COMMAND\n\tExpected: 'read', 'write', 'let', 'assert', 'print', 'show', 'time', 'fn', 'struct'\n");
            fprintf(OUTPUT, "\tFound: '%s'\n", error_string);
            break;
        case INVALID_EXPR:
            fprintf(OUTPUT, "Unable to parse EXPRESSION\n\tExpected: '[integer]', '[float]', 'true', 'false', 'void' ");
            fprintf(OUTPUT, "'[variable]', '[arrayliteral]', '[structliteral]', '[EXPRESSION][variable]', '[arrayindex]', '[function call]'\n\t");
            fprintf(OUTPUT, "Found: '%s'\n", error_string);
            break;
        case INVALID_LVALUE:
            fprintf(OUTPUT, "Unable to parse LVALUE\n\tExpected: '[variable]' or [variable][array]'\n\t");
            fprintf(OUTPUT, "Found: '%s'\n", error_string);
            break;
        case INVALID_STMT:
            fprintf(OUTPUT, "Unable to parse STATEMENT\n'\tExpected: 'let', 'assert', 'return'\n\tFound: '%s'\n", error_string);
            break;
        case INVALID_TYPE:
            fprintf(OUTPUT, "Unable to parse TYPE\n\tExpected: 'int', 'float', 'bool', 'void', '[struct variable]' or 'TYPE[array]'\n\t");
            fprintf(OUTPUT, "Found: '%s'\n", error_string);
            break;
        case UNEXPECTED_TOKEN:
            fprintf(OUTPUT, "Unexpected token: '%s'\n", error_string);
            break;
        case MISSING_TOKEN:
            fprintf(OUTPUT, "Missing expected token\n\tExpected: '%s'\n\tFound: '%s'\n", misc_string, error_string);
            break;
        case INT_RANGE:
            fprintf(OUTPUT, "Integer value out of range\n\tValue: '%s'\n", error_string);
            break;
        case FLOAT_RANGE:
            fprintf(OUTPUT, "Floating point value out of range\n\tValue: '%s'\n", error_string);
            break;
        case MISSING_BRACE:
            fprintf(OUTPUT, "Unclosed brace\n\tExpected: '}'\n\tFound: '%c'\n", error_string[0]);
            break;
        case MISSING_PAREN:
            fprintf(OUTPUT, "Unclosed parenthesis\n\tExpected: ')'\n\tFound: '%c'\n", error_string[0]);
            break;
        case MISSING_BRACKET:
            fprintf(OUTPUT, "Unclosed bracket\n\tExpected: ']'\n\tFound: '%c'\n", error_string[0]);
            break;
        case MISSING_NEWLINE:
            fprintf(OUTPUT, "Missing newline\n\tExpected: 'Newline'\n\tFound: '%s'\n", error_string);
            break;
        case MISSING_COLON:
            fprintf(OUTPUT, "Missing colon in binding\n\tExpected ':' in '%s %s'\n", start_string, error_string);
            break;
        case MISSING_STRING:
            fprintf(OUTPUT, "Missing string literal\n\tExpected: '\"[string]\"' before %s\n", error_string);
            break;
        case MISSING_COMMA:
            fprintf(OUTPUT, "Missing comma in bindings\n\tExpected ',' in '%s %s'\n", start_string, error_string);
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
            fprintf(OUTPUT, "Invalid string\n\tExpected: '\"'\n\tFound '%c'\n", string[0]);
            break;
        case INVALID_ESCAPE:
            fprintf(OUTPUT, "Invalid escape\n\tExpected: 'Newline' Found '%s'\n", string);
            break;
        case ILLEGAL_CHARACTER:
            fprintf(OUTPUT, "Illegal character 'u+%d'\n", (int)(string[0]));
            break;
        case INVALID_OPERATOR:
            fprintf(OUTPUT, "Invalid operator\n\tExpected: '&&', '||'\n\tFound: '%s'\n", string);
            break;
        case INVALID_COMMENT:
            fprintf(OUTPUT, "Invalid comment: '%s'\n", string);
    }

    free(string);
}

void print_error_line(char *p_c, size_t startloc, size_t errorloc, size_t line, ErrorStruct *error) {
    size_t i;
    Token *error_token = vector_get(token_list, error->error);
    size_t error_end = errorloc + error_token->strref.length;
    size_t line_start = 0;

    if (startloc != 0) {
        for (i = startloc-1; i > 0; --i) {
            if (p_c[i] == '\n') break;
        }
        if (p_c[i] == '\n') line_start = i+1;
    }


    for (i = errorloc; i < file_size; ++i) {
        if (p_c[i] == '\n' || p_c[i] == '\0' || p_c[i] == EOF) break;
    }
    size_t line_end = i;

    char *header = malloc(10);
    if (!header) exit(EXIT_FAILURE);
    if (snprintf(header, 10, "%6ld | ", line) < 0) exit(EXIT_FAILURE);

    putc('\n', OUTPUT);
    fprintf(OUTPUT, "%s", header);
    for (i = line_start; i < startloc; ++i)
        putc(p_c[i], OUTPUT);
    fprintf(OUTPUT, RED_BOLD);
    for (; i < error_end; ++i)
        putc(p_c[i], OUTPUT);
    fprintf(OUTPUT, RESET);
    for (; i < line_end; ++i)
        putc(p_c[i], OUTPUT);
    putc('\n', OUTPUT);

    for (i = 0; i < 9; ++i)
        putc(' ', OUTPUT);
    for (i = line_start; i < startloc; ++i)
        putc(' ', OUTPUT);
    fprintf(OUTPUT, RED_BOLD);
    for (; i < errorloc; ++i)
        putc('~', OUTPUT);
    for (; i == errorloc; ++i)
        putc('^', OUTPUT);
    for (; i < error_end; ++i)
        putc('~', OUTPUT);
    fprintf(OUTPUT, RESET);
    putc('\n', OUTPUT);

    free(header);
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
