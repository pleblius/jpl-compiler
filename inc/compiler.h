#ifndef COMPILER_H
#define COMPILER_H

#include "vector.h"

typedef struct {
    int (*run_mode)(char*);
    char* file_name;
} RunType;

void parse_args(int, char**);
int help_mode(char*);
int lex_mode(char*);
int parse_mode(char*);
void free_token_list(Vector*);

#endif // COMPILER_H
