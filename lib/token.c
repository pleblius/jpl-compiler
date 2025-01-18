#include <stdio.h>
#include <stdlib.h>

#include "token.h"

const char* token_strings[] = { "ARRAY", "ASSERT", "BOOL", "COLON", "COMMA", "DOT", "ELSE", "END_OF_FILE",
    "EQUALS", "FALSE", "FLOAT", "FLOATVAL", "FN", "IF", "IMAGE", "INT", "INTVAL", "LCURLY", "LET",
    "LPAREN", "LSQUARE", "NEWLINE", "OP", "PRINT", "RCURLY", "READ", "RETURN", "RPAREN", "RSQUARE",
    "SHOW", "STRING", "STRUCT", "SUM", "THEN", "TIME", "TO", "TRUE", "VARIABLE", "VOID", "WRITE" };

void print_token(Token *token) {
    if (!token) {
        fprintf(stderr, "Attempt to print non-existent token.\n");
        exit(EXIT_FAILURE);
    }

    if (token -> type == NEWLINE || token -> type == END_OF_FILE) {
        printf("%s\n", token_strings[token->type]);
    }
    else {
        if (!(token->string)) {
            fprintf(stderr, "Token contains invalid string.\n");
            exit(EXIT_FAILURE);
        }

        printf("%s '%s'\n", token_strings[token->type], token->string);
    }
}

Token *create_token (token_t type, unsigned long line, unsigned long col, char *string) {
    Token *token = (Token *) malloc(sizeof(Token));
    if (!token) {
        fprintf(stderr, "Failed to allocate memory for token.\n");
        exit(EXIT_FAILURE);
    }

    token -> type = type;
    token -> line = line;
    token -> col = col;
    token -> string = string;

    return token;
}

void free_token_string(Token *token) {
    if (!token) {
        fprintf(stderr, "Attempt to free string from invalid token.\n");
        exit(EXIT_FAILURE);
    }

    free(token -> string);
    token -> string = NULL;
}
