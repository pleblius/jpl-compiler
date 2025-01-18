#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "vector.h"

// Performs and prints lexical analysis of the provided .jpl file.
int lex(char*);
int lex_file();
void lex_print_output();
void lex_print_fail();
void lex_clear_list(Vector*);
token_t lex_parse_variable(char*);
token_t lex_keyword_match(int);
char *lex_create_string(unsigned long);
void lex_create_invalid_string(unsigned long);
void lex_create_illegal_string(char c);
char lex_fpeek();

int lex_variable(char, unsigned long*);
int lex_dot(char, unsigned long*);
int lex_number(char, unsigned long*);
int lex_string(char, unsigned long*);
int lex_operator(char, unsigned long*);
int lex_punctuation(char, unsigned long*);
int lex_comment(char, unsigned long*);
#endif // LEXER_H
