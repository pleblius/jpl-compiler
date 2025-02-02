#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "lexer.h"
#include "error.h"
#include "stringops.h"

extern Vector *token_list;
extern FILE* file_ptr;
extern char* file_string;
extern Vector *error_list;

const char* keyword_list[] = { "array", "assert", "bool", "else", "false", "float", "fn", "if", "image", "int", "let",
                            "print", "read", "return", "show", "struct", "sum", "then", "time", "to", "true", "void", "write" };
const TokenType keyword_tokens[] = { ARRAY, ASSERT, BOOL, ELSE, FALSE, FLOAT, FN, IF, IMAGE, INT, LET,
                            PRINT, READ, RETURN, SHOW, STRUCT, SUM, THEN, TIME, TO, TRUE, VOID, WRITE };

int lex() {
    char c;
    char *pc = file_string;
    TokenType type = INVALID;
    size_t count = 1;
    size_t byte = 1;
    size_t token_index = 0;

    while (1) {
        count = 1;
        while ((c = *pc) == ' ') {
            ++byte;
            ++pc;
        }

        if IS_CHAR(c) {
            type = lex_variable(pc, &count);
        }
        else if IS_PUNCTUATION(c) {
            type = lex_punctuation(pc, &count);
        }
        else if IS_NEWLINE(c) {
            count = 1;

            // Consolidate newlines into a single token
            if (vector_peek_last(token_list) != NULL && ((Token *) vector_peek_last(token_list))->type == NEWLINE) {
                pc += count;
                byte += count;
                continue;
            }

            type = NEWLINE;
        }
        else if IS_OPERATOR(c) {
            type = lex_operator(pc, &count);
            if (type == INVALID)
                lex_error(INVALID_OPERATOR, token_index, token_index, token_index+1, NULL);
        }
        else if IS_QUOTE(c) {
            type = lex_string(pc, &count);
            if (type == INVALID)
                lex_error(INVALID_STRING, token_index, token_index, token_index+1, NULL);
        }
        else if IS_DIGIT(c) {
            type = lex_number(pc, &count);
        }
        else if IS_DOT(c) {
            type = lex_dot(pc, &count);
        }
        else if IS_SLASH(c) {
            type = lex_comment(pc, &count);
            if (type == INVALID)
                lex_error(INVALID_COMMENT, token_index, token_index, token_index+1, NULL);
        }
        else if IS_ESCAPE(c) {
            if (!IS_NEWLINE(pc[1])) {
                count = 2;
                type = INVALID;
                lex_error(INVALID_ESCAPE, token_index, token_index, token_index+1, NULL);
            }
            else {
                pc += 2;
                byte += 2;
                continue;
            }
        }
        else if (c == '\0' || c == EOF) break;
        else {
            type = ILLEGAL;
        }
        
        if (type == VARIABLE) {
            type = match_keyword(pc, count);
        }
        else if (type == COMMENT) {
            pc += count;
            byte += count;
            continue;
        }
        else if (type == ILLEGAL) {
            lex_error(ILLEGAL_CHARACTER, token_index, token_index, token_index+1, NULL);
        }

        Token *token = create_token(type, byte, count, pc);
        vector_append(token_list, token);
        ++token_index;

        pc += count;
        byte += count;
    }

    vector_append(token_list, create_token(END_OF_FILE, byte, 1, pc));
    
    if (error_list == NULL) return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

TokenType lex_variable(char *pc, unsigned long *count) {
    unsigned long length = 0;
    char c = pc[0];

    while (IS_CHAR(c) || IS_SCORE(c) || IS_DIGIT(c)) {
        c = *(++pc);
        ++length;
    }

    *count = length;
    return VARIABLE;
}

TokenType lex_number(char *pc, unsigned long *count) {
    unsigned long length = 0;
    TokenType type;
    char c = *pc;

    while IS_DIGIT(c) {
        c = *(++pc);
        ++length;
    }

    if IS_DOT(c) {
        c = *(++pc);
        ++length;

        while IS_DIGIT(c) {
            c = *(++pc);
            ++length;
        }

        type = FLOATVAL;
    }
    else {
        type = INTVAL;
    }

    *count = length;
    return type;
}

TokenType lex_dot(char *pc, unsigned long *count) {
    unsigned long length = 1;
    TokenType type;
    char c = *(++pc);

    if (!IS_DIGIT(c)) {
        type = DOT;
    }
    else {
        while IS_DIGIT(c) {
            c = *(++pc);
            ++length;
        }
        
        type = FLOATVAL;
    }

    *count = length;
    return type;
}

TokenType lex_punctuation(char *pc, unsigned long *count) {
    *count = 1;
    
    switch (*pc) {
        case '[':
            return LSQUARE;
        case ']':
            return RSQUARE;
        case '{':
            return LCURLY;
        case '}':
            return RCURLY;
        case '(':
            return LPAREN;
        case ')':
            return RPAREN;
        case ',':
            return COMMA;
        case ':':
            return COLON;
        default:
            return INVALID;
    }
}

TokenType lex_operator(char *pc, unsigned long *count) {
    unsigned long length = 1;
    TokenType type = INVALID;
    char c = *pc;
    char nc = pc[1];

    if IS_MATH_OPERATOR(c) {
        type = OP;
    }
    else if IS_BOOL_OPERATOR(c) {
        if (nc == '=') {
            ++length;
        }
        
        if (c == '=' && nc != '=')
            type = EQUALS;
        else
            type = OP;
    }
    else if IS_SC_OPERATOR(c) {
        ++length;
        if (nc != c) {
            type = INVALID;
        }
        else
            type = OP;
    }

    *count = length;
    return type;
}

TokenType lex_string(char *pc, unsigned long *count) {
    unsigned long length = 1;
    TokenType type = STRING;
    char c;

    while ((c = *(++pc)) != '"') {
        ++length;
        if IS_ILLEGAL(c) {
            type = ILLEGAL;
        }
        else if (IS_NEWLINE(c) || c == '\0' || c == EOF) {
            type = INVALID;
            *count = length;
            return type;
        }
    }
    ++length;

    *count = length;
    return type;
}

TokenType lex_comment(char *pc, unsigned long *count) {
    unsigned long length = 1;
    char c = *(++pc);

    TokenType type = COMMENT;

    // Line Comment
    if IS_SLASH(c) {
        while (!IS_NEWLINE(c)) {
            c = *(++pc);
            ++length;

            if IS_ILLEGAL(c) {
                *count = length;
                type = ILLEGAL;
            }
            if (c == EOF || c == '\0') {
                *count = length;
                type = INVALID;
            }
        }
    }
    // Block comment
    else if (c == '*') {
        while (1) {
            c = *(++pc);
            ++length;
            if (c == '*' && pc[1] == '/') {
                length += 2;
                break;
            }

            if IS_ILLEGAL(c) {
                *count = length;
                type = ILLEGAL;
            }
            if (c == EOF) {
                *count = length;
                type = INVALID;
            }
        }
    }
    else {
        *count = 1;
        type = OP;
    }

    *count = length;
    return type;
}

// Prints the successfully lexed file's tokens.
void lex_print_output() {
    int i;
    for (i = 0; i < vector_size(token_list); ++i) {
        print_token(vector_get(token_list, i));
    }

    printf("Compilation succeeded: lexical analysis complete.\n");
}

TokenType match_keyword(char *pc, size_t count) {
    TokenType type;
    char string[count+1];
    strncpy(string, pc, count);
    string[count] = '\0';
    for (int i = 0; i < NUM_KEYWORDS; ++i) {
        if (!strcmp(string, keyword_list[i])) {
            type = lex_keyword_match(i);
            break;
        }
        type = VARIABLE;
    }

    return type;
}

TokenType lex_keyword_match(int i) {
    if (i < NUM_KEYWORDS)
        return keyword_tokens[i];
    
    return VARIABLE;
}
