#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "vector.h"

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

// Performs and prints lexical analysis of the provided .jpl file.
int lex_file();
void lex_print_output();
void lex_print_fail();
void lex_clear_list(Vector*);
token_t lex_parse_variable(char*);
token_t lex_keyword_match(int);
char *lex_create_string(unsigned long);
void lex_create_invalid_string(char c, unsigned long, unsigned long);
void lex_create_illegal_string(char c, unsigned long);
char lex_fpeek();
unsigned long lex_get_line(unsigned long);
unsigned long lex_get_col(unsigned long);

token_t lex_variable(char, unsigned long*);
token_t lex_dot(char, unsigned long*);
token_t lex_number(char, unsigned long*);
token_t lex_string(char, unsigned long*);
token_t lex_operator(char, unsigned long*);
token_t lex_punctuation(char, unsigned long*);
token_t lex_comment(char, unsigned long*);

void free_list(Vector*);

#endif // LEXER_H
