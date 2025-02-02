#include <stdio.h>
#include <stdlib.h>

#include "token.h"

const char* token_strings[] = { "ARRAY", "ASSERT", "BOOL", "COLON", "COMMA", "DOT", "ELSE", "END_OF_FILE",
    "EQUALS", "FALSE", "FLOAT", "FLOATVAL", "FN", "IF", "IMAGE", "INT", "INTVAL", "LCURLY", "LET",
    "LPAREN", "LSQUARE", "NEWLINE", "OP", "PRINT", "RCURLY", "READ", "RETURN", "RPAREN", "RSQUARE",
    "SHOW", "STRING", "STRUCT", "SUM", "THEN", "TIME", "TO", "TRUE", "VARIABLE", "VOID", "WRITE" };

void print_token(Token *token) {
    if (!token) return;
    char *str_array = array_from_ref(token->strref);

    if (token -> type == NEWLINE || token -> type == END_OF_FILE) {
        printf("%s\n", token_strings[token->type]);
    }
    else {
        if (!(token->strref.string)) return;

        printf("%s '%s'\n", token_strings[token->type], str_array);
    }

    free(str_array);
}

Token *create_token(TokenType type, size_t start, size_t count, char *pc) {
    Token *token = (Token *) malloc(sizeof(Token));
    if (!token) return NULL;

    token->type = type;
    token->byte = start;
    token->strref.length = count;
    token->strref.string = pc;

    return token;
}
