#ifndef COMPILER_H
#define COMPILER_H

#include "vector.h"

typedef enum { STANDARD_PRINT, PRETTY_PRINT, TABBED_PRINT, XML_PRINT } PrintMode;

typedef struct {
    int (*run_mode)(char*);
    char* file_name;
    PrintMode print_mode;
} RunType;

void parse_args(int, char**);
int help_mode(char*);
int lex_mode(char*);
int parse_mode(char*);
void free_token_list(Vector*);

#endif // COMPILER_H
