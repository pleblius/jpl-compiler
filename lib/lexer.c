#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "lexer.h"

#define IS_MATH_OPERATOR(c) (c == '*' || c == '+' || c == '-' || c == '%')
#define IS_BOOL_OPERATOR(c) (c == '<' || c == '>' || c == '=' || c == '!')
#define IS_SC_OPERATOR(c) (c == '&' || c == '|')
#define IS_OPERATOR(c) (IS_MATH_OPERATOR(c) || IS_BOOL_OPERATOR(c) || IS_SC_OPERATOR(c))
#define IS_DIGIT(c) (c >= '0' && c <= '9')
#define IS_DOT(c) (c == '.')
#define IS_CHAR(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define IS_SCORE(c) (c == '_')
#define IS_NEWLINE(c) (c == '\n')
#define IS_QUOTE(c) (c == '"')
#define IS_SLASH(c) (c == '/')
#define IS_ESCAPE(c) (c == '\\')
#define IS_PUNCTUATION(c) (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == ':')
#define IS_ILLEGAL(c) ((c != 10 && c < 32) || c > 126)

#define NUM_KEYWORDS 23

extern Vector *list;
char* fail_output = NULL;

static FILE *file_ptr;
unsigned long col;
unsigned long line;

const char* keyword_list[] = { "array", "assert", "bool", "else", "false", "float", "fn", "if", "image", "int", "let",
                            "print", "read", "return", "show", "struct", "sum", "then", "time", "to", "true", "void", "write" };

int lex(char *filename) {
    if (!filename) {
        fprintf(stderr, "Filename not included.\n");
        return EXIT_FAILURE;
    }

    file_ptr = fopen(filename, "r");
    if (!file_ptr) {
        fprintf(stderr, "Failed to open file: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    int return_status = lex_file();

    fclose(file_ptr);

    return return_status;
}

// Steps through the global file pointer, lexing tokens into the global token vector.
int lex_file() {
    char c = '\0';
    unsigned long count = 1;

    line = 1;
    col = 1;

    // Token identification
    while (1) {
        while ((c = fgetc(file_ptr)) == ' ') {
            ++col;
        }

        if (c == EOF) 
            break;
        else if IS_ILLEGAL(c) {
            lex_create_illegal_string(c);

            return EXIT_FAILURE;
        }
        else if IS_NEWLINE(c) {
            // Consolidate newlines into a single token
            if (vector_is_empty(list) || ((Token*) vector_peek_last(list)) -> type != NEWLINE)
                vector_append(list, create_token(NEWLINE, line, col, NULL));
            
            col = 1;
            ++line;

            continue;
        }
        else if IS_ESCAPE(c) {
            if (!IS_NEWLINE(lex_fpeek())) {
                lex_create_invalid_string(1);

                return EXIT_FAILURE;
            }

            fgetc(file_ptr);
            ++line;
            col = 1;

            continue;
        }
        else if IS_SLASH(c) {
            if (lex_comment(c, &count) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
        else if IS_OPERATOR(c) {
            if (lex_operator(c, &count) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
        else if IS_PUNCTUATION(c) {
            if (lex_punctuation(c, &count) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
        else if IS_CHAR(c) {
            if (lex_variable(c, &count) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
        else if IS_DIGIT(c) {
            if (lex_number(c, &count) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
        else if IS_DOT(c) {
            if (lex_dot(c, &count) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
        else if IS_QUOTE(c) {
            if (lex_string(c, &count) == EXIT_FAILURE)
                return EXIT_FAILURE;
        }
        else {
            lex_create_illegal_string(c);
            return EXIT_FAILURE;
        }

        col += count;
    }

    vector_append(list, create_token(END_OF_FILE, line, col, NULL));
    return EXIT_SUCCESS;
}

int lex_variable(char c, unsigned long *count) {
    unsigned long length = 1;
    c = lex_fpeek();

    while (IS_CHAR(c) || IS_SCORE(c) || IS_DIGIT(c)) {
        fgetc(file_ptr);
        c = lex_fpeek();
        ++length;
    }

    if (c == EOF) fgetc(file_ptr);

    char *buffer = lex_create_string(length);
    if (!buffer) 
        return EXIT_FAILURE;
    
    vector_append(list, create_token(lex_parse_variable(buffer), line, col, buffer));

    *count = length;
    return EXIT_SUCCESS;
}

int lex_number(char c, unsigned long *count) {
    unsigned long length = 1;
    token_t type;
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

    char *buffer = lex_create_string(length);
    if (!buffer)
        return EXIT_FAILURE;

    vector_append(list, create_token(type, line, col, buffer));

    *count = length;
    return EXIT_SUCCESS;
}

int lex_dot(char c, unsigned long *count) {
    unsigned long length = 1;
    token_t type;

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

    char *buffer = lex_create_string(length);
    if (!buffer)
        return EXIT_FAILURE;

    vector_append(list, create_token(type, line, col, buffer));

    *count = length;
    return EXIT_SUCCESS;
}

int lex_punctuation(char c, unsigned long *count) {
    unsigned long length = 1;
    token_t type;
    
    switch (c) {
        case '[':
            type = LSQUARE;
            break;
        case ']':
            type = RSQUARE;
            break;
        case '{':
            type = LCURLY;
            break;
        case '}':
            type = RCURLY;
            break;
        case '(':
            type = LPAREN;
            break;
        case ')':
            type = RPAREN;
            break;
        case ',':
            type = COMMA;
            break;
        case ':':
            type = COLON;
            break;
        default:
            lex_create_invalid_string(length);
            return EXIT_FAILURE;
    }
    
    char *buffer = lex_create_string(length);
    if (!buffer)
        return EXIT_FAILURE;
    
    vector_append(list, create_token(type, line, col, buffer));
    
    *count = length;
    return EXIT_SUCCESS;
}

int lex_operator(char c, unsigned long *count) {
    unsigned long length = 1;
    token_t type;

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
            lex_create_invalid_string(length);

            return EXIT_FAILURE;
        }
        ++length;

        type = OP;
    }
    else {
        lex_create_invalid_string(length);

        return EXIT_FAILURE;
    }

    char *buffer = lex_create_string(length);
    if (!buffer)
        return EXIT_FAILURE;

    vector_append(list, create_token(type, line, col, buffer));

    *count = length;
    return EXIT_SUCCESS;
}

int lex_string(char c, unsigned long *count) {
    unsigned long length = 1;

    while ((c = fgetc(file_ptr)) != '"') {
        ++length;
        if IS_ILLEGAL(c) {
            col += length;
            lex_create_illegal_string(c);
            return EXIT_FAILURE;
        }
        else if (IS_NEWLINE(c) || c == EOF) {
            col += length;
            lex_create_invalid_string(1);
            return EXIT_FAILURE;
        }
    }
    ++length;

    char *buffer = lex_create_string(length);
    if (!buffer) 
        return EXIT_FAILURE;

    vector_append(list, create_token(STRING, line, col, buffer));

    *count = length;
    return EXIT_SUCCESS;
}

int lex_comment(char c, unsigned long *count) {
    unsigned long length = 1;

    c = lex_fpeek();

    // Line Comment
    if IS_SLASH(c) {
        while (!IS_NEWLINE(c)) {
            fgetc(file_ptr);
            c = lex_fpeek();
            ++length;

            if IS_ILLEGAL(c) {
                lex_create_illegal_string(c);
                return EXIT_FAILURE;
            }
            if (c == EOF) {
                lex_create_invalid_string(length);
                return EXIT_FAILURE;
            }
        }
    }
    // Block comment
    else if (c == '*') {
        ++length;
        while (1) {
            if IS_NEWLINE(c) {
                ++line;
                col = 1;
            }
            c = fgetc(file_ptr);
            ++length;
            if (c == '*' && lex_fpeek() == '/') break;

            if IS_ILLEGAL(c) {
                lex_create_illegal_string(c);
                return EXIT_FAILURE;
            }
            if (c == EOF) {
                lex_create_invalid_string(1);
                return EXIT_FAILURE;
            }
        }

        fgetc(file_ptr);
    }
    else {
        length = 1;

        char *buffer = lex_create_string(length);
        if (!buffer)
            return EXIT_FAILURE;

        vector_append(list, create_token(OP, line, col, buffer));
    }
    *count = length;
    return EXIT_SUCCESS;
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
void lex_create_invalid_string(unsigned long length) {
    fseek(file_ptr, -length, SEEK_CUR);
    
    char *buffer = (char *) malloc(length + 1);
    if (!buffer) return;

    unsigned long i;
    for (i = 0; i < length; ++i) {
        buffer[i] = fgetc(file_ptr);
    }
    buffer[i] = '\0';

    if (0 > asprintf(&fail_output, "Unable to lex. Invalid token at line: %lu, col: %lu", line, col))
        fail_output = NULL;
    
    free(buffer);
}

// Creates a string for when an illegal or unprintable token is encountered.
void lex_create_illegal_string(char c) {
    int i = (int) c;

    if (0 > asprintf(&fail_output, "Unable to lex. Illegal token char:%d at line: %lu, col: %lu", i, line, col))
        fail_output = NULL;
}

// Prints the successfully lexed file's tokens.
void lex_print_output() {
    int i;
    for (i = 0; i < vector_size(list); ++i) {
        print_token(vector_get(list, i));
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
token_t lex_parse_variable(char* string) {
    int i;
    for (i = 0; i < NUM_KEYWORDS; ++i) {
        if (!strcmp(string, keyword_list[i])) 
            return lex_keyword_match(i);
    }

    return VARIABLE;
}

// Return the token type associated with the given index of keyword.
token_t lex_keyword_match(int i) {
    switch(i) {
        case 0:
            return ARRAY;
        case 1:
            return ASSERT;
        case 2:
            return BOOL;
        case 3:
            return ELSE;
        case 4:
            return FALSE;
        case 5:
            return FLOAT;
        case 6:
            return FN;
        case 7:
            return IF;
        case 8:
            return IMAGE;
        case 9:
            return INT;
        case 10:
            return LET;
        case 11:
            return PRINT;
        case 12:
            return READ;
        case 13:
            return RETURN;
        case 14:
            return SHOW;
        case 15:
            return STRUCT;
        case 16:
            return SUM;
        case 17:
            return THEN;
        case 18:
            return TIME;
        case 19:
            return TO;
        case 20:
            return TRUE;
        case 21:
            return VOID;
        case 22:
            return WRITE;
        default:
            return VARIABLE;
    }
}
