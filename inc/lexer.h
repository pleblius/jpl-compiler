#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include "token.h"
#include "dict.h"
#include "vecs.h"
#include "error.h"

#define NEWLINE_M '\n'
#define SCORE '_'
#define IS_END_CHAR(c) (c == '\0' || c == EOF)
#define SPACE ' '
#define SLASH '/'
#define ESCAPE '\\'
#define LOWERCASE 'a' ... 'z'
#define UPPERCASE 'A' ... 'Z'
#define DIGIT '0' ... '9'
#define COMPARISON '<' ... '>'
#define AND '&'
#define OR '|'
#define BANG '!'
#define DOT_M '.'
#define COMMA_M ','
#define ADD '+'
#define TIMES '*'
#define MINUS '-'
#define MOD '%'
#define LCURLY_M '{'
#define RCURLY_M '}'
#define LPAREN_M '('
#define RPAREN_M ')'
#define LSQUARE_M '['
#define RSQUARE_M ']'
#define QUOTE '\"'
#define COLON_M ':'
#define EQUAL_M '='

#define IS_DIGIT(c) (c <= '9' && c >= '0')
#define IS_ILLEGAL(c) (c < 32 || c == 127)

int lex_string(char*, size_t, TokenVec**);

uint32_t lex_var_token(char*, uint32_t, Token*);
uint32_t lex_num_token(char*, uint32_t, Token*);
uint32_t lex_dot_token(char*, uint32_t, Token*);
uint32_t lex_punct_token(char*, uint32_t, Token*);
uint32_t lex_slash_token(char*, uint32_t, Token*);
uint32_t lex_line_comment(char*, uint32_t, Token*);
uint32_t lex_multiline_comment(char*, uint32_t, Token*);
uint32_t lex_string_token(char*, uint32_t, Token*);
uint32_t lex_bool_token(char*, uint32_t, Token*);

int is_keyword_match(char *, uint32_t, uint32_t, Token*);
Dict *create_keyword_dictionary();

void lex_error(LexErrorType, Token*);

#endif // LEXER_H
