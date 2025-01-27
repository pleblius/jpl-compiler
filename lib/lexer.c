#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "lexer.h"
#include "error.h"

extern Vector *token_list;
extern FILE* file_ptr;
extern char* fail_output;

const char* keyword_list[] = { "array", "assert", "bool", "else", "false", "float", "fn", "if", "image", "int", "let",
                            "print", "read", "return", "show", "struct", "sum", "then", "time", "to", "true", "void", "write" };
const TokenType keyword_tokens[] = { ARRAY, ASSERT, BOOL, ELSE, FALSE, FLOAT, FN, IF, IMAGE, INT, LET,
                            PRINT, READ, RETURN, SHOW, STRUCT, SUM, THEN, TIME, TO, TRUE, VOID, WRITE };

// Steps through the global file pointer, lexing tokens into the global token vector.
int lex_file() {
    char c = '\0';
    unsigned long count = 1;
    static unsigned long byte = 1;
    char *buffer;
    TokenType type;

    // Token identification
    while (1) {
        while ((c = fgetc(file_ptr)) == ' ') {
            ++byte;
        }

        if (c == EOF) 
            break;
        else if IS_ILLEGAL(c) {
            type = ILLEGAL;
        }
        else if IS_NEWLINE(c) {
            // Consolidate newlines into a single token
            if (vector_is_empty(token_list) || ((Token*) vector_peek_last(token_list)) -> type != NEWLINE)
                vector_append(token_list, create_token(NEWLINE, byte, NULL));
            
            ++byte;

            continue;
        }
        else if IS_ESCAPE(c) {
            if (!IS_NEWLINE(lex_fpeek())) {
                count = 1;
                type = INVALID;
            }
            else {
                fgetc(file_ptr);
                continue;
            }
        }
        else if IS_SLASH(c) {
            type = lex_comment(c, &count);
        }
        else if IS_OPERATOR(c) {
            type = lex_operator(c, &count);
        }
        else if IS_PUNCTUATION(c) {
            type = lex_punctuation(c, &count);
        }
        else if IS_CHAR(c) {
            type = lex_variable(c, &count);
        }
        else if IS_DIGIT(c) {
            type = lex_number(c, &count);
        }
        else if IS_DOT(c) {
            type = lex_dot(c, &count);
        }
        else if IS_QUOTE(c) {
            type = lex_string(c, &count);
        }
        else {
            type = ILLEGAL;
        }

        if (type == ILLEGAL) {
            lex_create_illegal_string(c, byte);

            return EXIT_FAILURE;
        }
        else if (type == INVALID) {
            lex_create_invalid_string(c, count, byte);

            return EXIT_FAILURE;   
        }
        else if (type == VARIABLE) {
            buffer = lex_create_string(count);
            if (!buffer)
                return EXIT_FAILURE;

            vector_append(token_list, create_token(lex_parse_variable(buffer), byte, buffer));
        }
        else if(type == COMMENT) {}
        else {
            buffer = lex_create_string(count);
            if (!buffer) 
                return EXIT_FAILURE;
                
            vector_append(token_list, create_token(type, byte, buffer));
        }

        byte += count;
    }

    vector_append(token_list, create_token(END_OF_FILE, byte, NULL));
    return EXIT_SUCCESS;
}

TokenType lex_variable(char c, unsigned long *count) {
    unsigned long length = 1;
    c = lex_fpeek();

    while (IS_CHAR(c) || IS_SCORE(c) || IS_DIGIT(c)) {
        fgetc(file_ptr);
        c = lex_fpeek();
        ++length;
    }

    if (c == EOF) fgetc(file_ptr);

    *count = length;
    return VARIABLE;
}

TokenType lex_number(char c, unsigned long *count) {
    unsigned long length = 1;
    TokenType type;
    c = lex_fpeek();

    while IS_DIGIT(c) {
        fgetc(file_ptr);
        c = lex_fpeek();
        ++length;
    }

    if IS_DOT(c) {
        c = fgetc(file_ptr);
        ++length;
        c = lex_fpeek();

        while IS_DIGIT(c) {
            fgetc(file_ptr);
            c = lex_fpeek();
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

TokenType lex_dot(char c, unsigned long *count) {
    unsigned long length = 1;
    TokenType type;

    c = lex_fpeek();

    if (!IS_DIGIT(c)) {
        type = DOT;
    }
    else {
        while IS_DIGIT(c) {
            fgetc(file_ptr);
            c = lex_fpeek();
            ++length;
        }
        
        type = FLOATVAL;
    }

    *count = length;
    return type;
}

TokenType lex_punctuation(char c, unsigned long *count) {
    *count = 1;
    
    switch (c) {
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

TokenType lex_operator(char c, unsigned long *count) {
    unsigned long length = 1;
    TokenType type;

    if IS_MATH_OPERATOR(c) {
        type = OP;
    }
    else if IS_BOOL_OPERATOR(c) {
        if (lex_fpeek() == '=') {
            ++length;
            fgetc(file_ptr);
        }
        
        if (c == '=' && length == 1)
            type = EQUALS;
        else
            type = OP;
    }
    else if IS_SC_OPERATOR(c) {
        if (fgetc(file_ptr) != c) {
            type = INVALID;
        }
        ++length;

        type = OP;
    }
    else {
        type = INVALID;
    }

    *count = length;
    return type;
}

TokenType lex_string(char c, unsigned long *count) {
    unsigned long length = 1;
    TokenType type = STRING;

    while ((c = fgetc(file_ptr)) != '"') {
        if IS_ILLEGAL(c) {
            type =  ILLEGAL;
            break;
        }
        else if (IS_NEWLINE(c) || c == EOF) {
            type = INVALID;
            break;
        }
        ++length;
    }
    ++length;

    *count = length;
    return type;
}

TokenType lex_comment(char c, unsigned long *count) {
    unsigned long length = 1;

    c = lex_fpeek();

    // Line Comment
    if IS_SLASH(c) {
        while (!IS_NEWLINE(c)) {
            fgetc(file_ptr);
            c = lex_fpeek();
            ++length;

            if IS_ILLEGAL(c) {
                *count = length;
                return ILLEGAL;
            }
            if (c == EOF) {
                *count = length;
                return INVALID;
            }
        }
    }
    // Block comment
    else if (c == '*') {
        ++length;
        while (1) {
            c = fgetc(file_ptr);
            ++length;
            if (c == '*' && lex_fpeek() == '/') break;

            if IS_ILLEGAL(c) {
                *count = length;
                return ILLEGAL;
            }
            if (c == EOF) {
                *count = length;
                return INVALID;
            }
        }

        fgetc(file_ptr);
    }
    else {
        *count = 1;
        return OP;
    }

    *count = length;
    return COMMENT;
}

// Peeks at the next character without advancing the filestream.
char lex_fpeek() {
    char c = fgetc(file_ptr);
    fseek(file_ptr, -1, SEEK_CUR);

    return c;
}

// Creates a string of the given length plus the string escape character using the current file pointer position.
char *lex_create_string(unsigned long length) {
    char *buffer = NULL;

    fseek(file_ptr, -length, SEEK_CUR);
    
    buffer = (char *) malloc(length + 1);
    if (!buffer) return NULL;

    unsigned long i;
    for (i = 0; i < length; ++i) {
        buffer[i] = fgetc(file_ptr);
    }
    buffer[i] = '\0';

    return buffer;
}

// Creates a string of the given length plus the string escape character for the invalid token pointer.
void lex_create_invalid_string(char c, unsigned long length, unsigned long byte) {
    char *buffer;
    char c2;
    unsigned long  i;

    if (c == '\n') {
        buffer = "\'\\n\'";
    }
    else {
        fseek(file_ptr, -length, SEEK_CUR);

        buffer = (char *) malloc(length + 1);
            if (!buffer) return;
    
        for (i = 0; i < length; ++i) {
            c2 = fgetc(file_ptr);
            if (c2 == '\n') break;

            buffer[i] = c2;
        }
        buffer[i] = '\0';
    }
    unsigned long line = get_line(byte);
    unsigned long col = get_col(byte);

    if (0 > asprintf(&fail_output, "Unable to lex. Invalid token %s at line: %lu, col: %lu", buffer, line, col))
        fail_output = NULL;
    
    free(buffer);
}

// Creates a string for when an illegal or unprintable token is encountered.
void lex_create_illegal_string(char c, unsigned long byte) {
    int i = (int) c;
    unsigned long line = get_line(byte);
    unsigned long col = get_col(byte);

    if (0 > asprintf(&fail_output, "Unable to lex. Illegal token char: '%d' at line: %lu, col: %lu", i, line, col))
        fail_output = NULL;
}

// Prints the successfully lexed file's tokens.
void lex_print_output() {
    int i;
    for (i = 0; i < vector_size(token_list); ++i) {
        print_token(vector_get(token_list, i));
    }

    printf("Compilation succeeded: lexical analysis complete.\n");
}

// Prints a failure message if lexing cannot be completed.
void lex_print_fail() {
    if (fail_output != NULL) 
        printf("Compilation failed: %s.\n", fail_output);
    else
        printf("Compilation failed\n");
}

// Takes a token string and returns the associated token type, either keyword or variable.
TokenType lex_parse_variable(char* string) {
    int i;
    for (i = 0; i < NUM_KEYWORDS; ++i) {
        if (!strcmp(string, keyword_list[i])) 
            return lex_keyword_match(i);
    }

    return VARIABLE;
}

// Return the token type associated with the given index of keyword.
TokenType lex_keyword_match(int i) {
    if (i < NUM_KEYWORDS)
        return keyword_tokens[i];
    
    return VARIABLE;
}

void free_list(Vector *vector) {
    for (int i = 0; i < vector->size; ++i) {
        free_token_string(vector_get(vector, i));
    }

    vector_destroy(vector);
}
