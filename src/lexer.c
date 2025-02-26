#include <stdio.h>
#include <stdint.h>

#include "lexer.h"
#include "error.h"

static char* keywords[] = { "array", "assert", "bool", "else", "false", "float", "fn", "if", "image", "int", "let", "print",
                                "read", "return", "show", "struct", "sum", "then", "time", "to", "true", "void", "write" };

static TokenType keyword_types[] = { ARRAY, ASSERT, BOOL, ELSE, FALSE, FLOAT, FN, IF, IMAGE, INT, LET, PRINT, READ,
                                RETURN, SHOW, STRUCT, SUM, THEN, TIME, TO, TRUE, VOID, WRITE };

static int lex_fail_status = EXIT_SUCCESS;
static Dict *keyword_dictionary;
static TokenVec *token_vector;

int lex_string(char* string, size_t size, TokenVec **vector) {
    uint32_t byte = 0;
    uint32_t count = 0;
    Token new_token;

    keyword_dictionary = create_keyword_dictionary();
    if (!keyword_dictionary) {
        fprintf(stderr, "keyword dictionary failure\n");
        return EXIT_FAILURE;
    }

    token_vector = tokenvec_create_cap(size);
    if (!token_vector) {
        fprintf(stderr, "tokenvec malloc failure\n");
        return EXIT_FAILURE;
    }

    while (*string) {
        while (*string == SPACE) {
            ++string;
            ++byte;
        }

        switch (*string) {
            // Variables and keywords
            case LOWERCASE:
            case UPPERCASE:
            case SCORE:
                count = lex_var_token(string, byte, &new_token);
                break;

            // Numbers
            case DIGIT:
                count = lex_num_token(string, byte, &new_token);
                break;

            // Floats and Dots
            case DOT_M:
                count = lex_dot_token(string, byte, &new_token);
                break;

            // Punctuation marks
            case LCURLY_M:
            case RCURLY_M:
            case LPAREN_M:
            case RPAREN_M:
            case LSQUARE_M:
            case RSQUARE_M:
            case COLON_M:
            case COMMA_M:
                count = lex_punct_token(string, byte, &new_token);
                break;

            // Boolean operators
            case COMPARISON:
            case BANG:
            case AND:
            case OR:
                count = lex_bool_token(string, byte, &new_token);
                break;

            // Math operators
            case TIMES:
            case ADD:
            case MINUS:
            case MOD:
                count = 1;
                new_token = create_token(OP, byte, count, string);
                break;

            // String literals
            case QUOTE:
                count = lex_string_token(string, byte, &new_token);
                break;

            // Comment and divide
            case SLASH:
                count = lex_slash_token(string, byte, &new_token);
                if (new_token.type == COMMENT) {
                    string += count;
                    byte += count;
                    continue;
                }
                break;

            // Newlines
            case NEWLINE_M:
                if (!tokenvec_is_empty(token_vector) && ((Token*) tokenvec_peek_last(token_vector))->type == NEWLINE) {
                    ++string;
                    ++byte;
                    continue;
                }
                count = 1;
                new_token = create_token(NEWLINE, byte, count, string);
                break;

            // Only escapes newlines
            case ESCAPE:
                if (*(++string) == NEWLINE_M) {
                    byte += 2;
                    ++string;
                    continue;
                }
                else {
                    count = 2;
                    new_token = create_token(INVALID, byte, count, string);
                }
                break;

            // Miscellaneous
            default:
                count = 1;
                new_token = create_token(ILLEGAL, byte, count, string);
        }

        tokenvec_append(token_vector, new_token);

        if (new_token.type == INVALID || new_token.type == ILLEGAL) {
            lex_error(tokenvec_peek_last(token_vector));
        }

        string += count;
        byte += count;
    }

    new_token = create_token(END_OF_FILE, byte, 1, string);
    tokenvec_append(token_vector, new_token);
    *vector = token_vector;
    dict_free(keyword_dictionary);
    
    return lex_fail_status;
}

uint32_t lex_var_token(char *string, uint32_t loc, Token *out) {
    uint32_t count = 1;
    char *c_ptr = string + 1;
    
    int is_valid_char = 1;
    while (is_valid_char) {
        switch (*c_ptr) {
            case LOWERCASE:
            case UPPERCASE:
            case DIGIT:
            case SCORE:
                ++c_ptr;
                ++count;
                break;
            default:
                is_valid_char = 0;
                break;
        }
    }

    if (!is_keyword_match(string, loc, count, out))
        *out = create_token(VARIABLE, loc, count, string);

    return count;
}
uint32_t lex_num_token(char *string, uint32_t loc, Token *out) {
    uint32_t count = 1;
    TokenType type = INTVAL;
    char *c_ptr = string + 1;
    char c = *c_ptr;

    while (IS_DIGIT(c)) {
        ++count;
        c = *(++c_ptr);
    }

    if (c == DOT_M) {
        c = *(++c_ptr);
        ++count;
        while (IS_DIGIT(c)) {
            ++count;
            c = *(++c_ptr);
        }

        type = FLOATVAL;
    }
    else
        type = INTVAL;
    
    *out = create_token(type, loc, count, string);
    return count;
}
uint32_t lex_dot_token(char *string, uint32_t loc, Token *out) {
    uint32_t count = 1;
    TokenType type = INTVAL;
    char *c_ptr = string + 1;
    char c = *c_ptr;

    if (IS_DIGIT(c)) {
        while (IS_DIGIT(c)) {
            ++count;
            c = *(++c_ptr);
        }

        type = FLOATVAL;
    }
    else
        type = DOT;

    *out = create_token(type, loc, count, string);
    return count;
}
uint32_t lex_punct_token(char *string, uint32_t loc, Token *out) {
    uint32_t count = 1;
    TokenType type;

    switch (*string) {
        case LPAREN_M:
            type = LPAREN;
            break;
        case RPAREN_M:
            type = RPAREN;
            break;
        case LSQUARE_M:
            type = LSQUARE;
            break;
        case RSQUARE_M:
            type = RSQUARE;
            break;
        case LCURLY_M:
            type = LCURLY;
            break;
        case RCURLY_M:
            type = RCURLY;
            break;
        case COMMA_M:
            type = COMMA;
            break;
        case COLON_M:
            type = COLON;
            break;
        default:
            type = INVALID;
            break;
    }

    *out = create_token(type, loc, count, string);
    return count;
}
uint32_t lex_slash_token(char *string, uint32_t loc, Token *out) {
    uint32_t count = 1;
    char *c_ptr = string + 1;
    char c = *c_ptr;

    switch (c) {
        case SLASH:
            return lex_line_comment(string, loc, out);
        case TIMES:
            return lex_multiline_comment(string, loc, out);
        default:
            *out = create_token(OP, loc, count, string);
            return count;
    }
}
uint32_t lex_line_comment(char *string, uint32_t loc, Token *out) {
    uint32_t count = 2;
    TokenType type = COMMENT;
    char *c_ptr = string + 2;
    char c = *c_ptr;

    while (c != NEWLINE_M) {
        if (IS_ILLEGAL(c)) {
            type = INVALID;
            break;
        }

        ++count;
        c = *(++c_ptr);
    }

    // Dummy token
    *out = create_token(type, loc, count, string);
    return count;
}
uint32_t lex_multiline_comment(char *string, uint32_t loc, Token *out) {
    uint32_t count = 2;
    TokenType type = COMMENT;
    char *c_ptr = string + 2;
    char c = *c_ptr;

    while (1) {
        if (c != NEWLINE_M && IS_ILLEGAL(c)) {
            type = INVALID;
            break;
        }
        if (c == TIMES && *(c_ptr + 1) == SLASH) {
            count += 2;

            break;
        }
        ++count;
        c = *(++c_ptr);
    }

    // Dummy token
    *out = create_token(type, loc, count, string);
    return count;
}
uint32_t lex_string_token(char *string, uint32_t loc, Token *out) {
    uint32_t count = 1;
    TokenType type = STRING;
    char *c_ptr = string + 1;
    char c = *c_ptr;

    while (c != QUOTE) {
        if (IS_ILLEGAL(c) || c == NEWLINE_M) {
            type = INVALID;
            break;
        }

        ++count;
        c = *(++c_ptr);
    }

    ++count;
    *out = create_token(type, loc, count, string);
    return count;
}

uint32_t lex_bool_token(char *string, uint32_t loc, Token *out) {
    uint32_t count = 1;
    TokenType type = OP;
    char *c_ptr = string + 1;
    char c = *c_ptr;

    switch (*string) {
        case EQUAL_M:
            if (c == EQUAL_M)
                count = 2;
            else
                type = EQUALS;
            break;
        case AND:
        case OR:
            if (*string == c)
                count = 2;
            else
                type = ILLEGAL;
            break;
        default:
            if (c == EQUAL_M)
                count = 2;
    }
    
    *out = create_token(type, loc, count, string);
    return count;
}

int is_keyword_match(char *string, uint32_t loc, uint32_t len, Token *out) {
    if (!string || !len || !out) return 0;

    TokenType *type;
    if (dict_try_array(keyword_dictionary, string, len, (void**) &type)) {
        *out = create_token(*type, loc, len, string);
        return 1;
    }

    return 0;
}

Dict *create_keyword_dictionary() {
    Dict *dict = dict_create_small();
    if (!dict) return NULL;

    char *word;
    size_t len = sizeof(keywords) / sizeof(size_t);
    for (size_t i = 0; i < len; ++i) {
        word = keywords[i];
        dict_add_array(dict, word, strlen(word), (void**) &keyword_types[i]);
    }

    return dict;
}

void lex_error(Token *token) {
    switch (token->type) {
        case INVALID:
            add_lex_error(INVALID_LEX, token);
            break;
        case ILLEGAL:
            add_lex_error(ILLEGAL_LEX, token);
            break;
        default:
            return;
    }

    lex_fail_status = EXIT_FAILURE;
}
