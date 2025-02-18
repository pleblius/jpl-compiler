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
int lex();
void lex_clear_list(Vector*);
TokenType lex_keyword_match(int);
TokenType match_keyword(char *, size_t);
char *lex_create_string(unsigned long);

TokenType lex_variable(char*, unsigned long*);
TokenType lex_dot(char*, unsigned long*);
TokenType lex_number(char*, unsigned long*);
TokenType lex_string(char*, unsigned long*);
TokenType lex_operator(char*, unsigned long*);
TokenType lex_punctuation(char*, unsigned long*);
TokenType lex_comment(char*, unsigned long*);

void free_list(Vector*);

#endif // LEXER_H
