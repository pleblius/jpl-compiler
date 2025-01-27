#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "error.h"
#include "compiler.h"
#include "lexer.h"
#include "stringops.h"

#define MAXIMUM_BUFFER 1024

extern RunType mode;
extern FILE* file_ptr;
extern char* token_strings[];

void parse_error(ParseError error, TokenType expected, Token *token) {
    switch(error) {
        case INVALID_COMMAND:
        case INVALID_EXPRESSION:
        case INVALID_ARG:
        case INVALID_TOKEN:
            parse_token_error(expected, token);
            break; 
        case MISSING_NUMBER:
            parse_missing_number(expected, token);
            break;
        case MISSING_STRING:
            parse_missing_string(expected, token);
            break;
        case OUT_OF_RANGE:
            parse_range_error(token);
    }
}

void parse_missing_number(TokenType expected, Token *token) {
    char *string;
    unsigned long loc = token->byte;
    switch (expected) {
        case FLOATVAL:
            string = "float";
            break;
        case INTVAL:
            string = "int";
            break;
        default:
            string = "";
    }
    unsigned long line = get_line(loc);
    unsigned long col = get_col(loc);

    printf("\nCompilation error at %s:%ld:%ld\n", mode.file_name, get_line(loc), get_col(loc));
    printf("\tExpected: %s\n\n", string);
    print_error_line(line, col, token);
    printf("Compilation failed\n");
}

void parse_range_error(Token *token) {
    unsigned long loc = token->byte;
    unsigned long line = get_line(loc);
    unsigned long col = get_col(loc);

    printf("\nCompilation error at %s:%ld:%ld\n", mode.file_name, line, col);
    printf("\tNumeric value out of range: %s\n\n", token->string);
    print_error_line(line, col, token);
    printf("Compilation failed\n");

}

void parse_missing_token(TokenType expected, Token *token) {
    unsigned long loc = token->byte;
    unsigned long line = get_line(loc);
    unsigned long col = get_col(loc);

    printf("\nCompilation error at %s:%ld:%ld\n", mode.file_name, get_line(loc), get_col(loc));
    printf("\tMissing token. Expected: %s\n\n", token_strings[expected]);
    print_error_line(line, col, token);
    printf("Compilation failed\n");

}

void parse_missing_string(TokenType expected, Token *token) {
    unsigned long loc = token->byte;
    unsigned long line = get_line(loc);
    unsigned long col = get_col(loc);

    printf("\nCompilation error at %s:%ld:%ld\n", mode.file_name, line, col);
    printf("\tMissing token string. Expected type %s\n\n", token_strings[expected]);
    print_error_line(line, col, token);
    printf("Compilation failed\n");

}

void parse_token_error(TokenType expected, Token *token) {
    unsigned long loc = token -> byte;
    unsigned long line = get_line(loc);
    unsigned long col = get_col(loc);
    
    printf("\nCompilation error at %s:%ld:%ld\n", mode.file_name, get_line(loc), get_col(loc));
    printf("\tExpected: %s\n", token_strings[expected]);
    printf("\tFound: %s\n\n", token_strings[token->type]);
    print_error_line(line, col, token);
    printf("Compilation failed\n");

}

void parse_arg_error() {
    printf("Compilation failed.\n");
}

void parse_cmd_error() {
    printf("Compilation failed.\n");
}

void parse_expr_error() {
    printf("Compilation failed.\n");
}

void lex_error() {

}

unsigned long get_col(unsigned long byte) {
    unsigned long col = 1;
    unsigned long count = 1;
    char c;

    fseek(file_ptr, 0, SEEK_SET);

    while (count < byte) {
        c = fgetc(file_ptr);
        ++col;
        ++count;

        if (c == '\n')
            col = 1;
    }

    return col;
}

unsigned long get_line(unsigned long byte) {
    unsigned long line = 1;
    unsigned long count = 1;

    fseek(file_ptr, 0, SEEK_SET);

    while (count < byte) {
        if (fgetc(file_ptr) == '\n')
            ++line;
        
        ++count;
    }

    return line;
}

void print_error_line(unsigned long line, unsigned long col, Token *token) {
    if (fseek(file_ptr, token->byte - col, SEEK_SET) != 0) return;

    unsigned long print_start = 0;
    unsigned long print_end = 0;
    unsigned long print_point = 0;
    
    char *buffer = malloc(MAXIMUM_BUFFER);
    char *string_line = malloc(MAXIMUM_BUFFER);
    char *error_line = malloc(MAXIMUM_BUFFER);
    char *post_line = malloc(MAXIMUM_BUFFER);

    if (fgets(string_line, MAXIMUM_BUFFER, file_ptr) == NULL) return;

    int check = snprintf(buffer, MAXIMUM_BUFFER, "%ld | %s", line, string_line);
    if (check < 0) {
        fprintf(stderr, "Buffer failer.\n");
        exit(EXIT_FAILURE);
    }

    print_point = col + strlen(buffer) - strlen(string_line) - 1;
    unsigned int i;
    for (i = 0; i < print_point; ++i) {
        if (buffer[i] == ' ') {
            print_start = i + 1;
        }
    }
    while (i < strlen(buffer)) {
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == EOF) {
            print_end = i-1; break;
        }
        ++i;
    }
    strcpy(post_line, buffer + print_end+1);
    buffer[print_end+1] = '\0';
    strcpy(error_line, buffer + print_start);
    buffer[print_start] = '\0';

    printf("%s", buffer);
    printf(RED_BOLD);
    printf("%s", error_line);
    printf(RESET);
    printf("%s", post_line);

    for (i = 0; i < print_start; ++i) {
        putc(' ', stdout);
    }
    printf(RED_BOLD);
    for (; i < print_point; ++i) {
        printf("~");
    }
    printf("^");
    for (; i < print_end; ++i) {
        printf("~");
    }
    printf(RESET "\n\n");

    free(buffer);
    free(string_line);
    free(post_line);
    free(error_line);
}
