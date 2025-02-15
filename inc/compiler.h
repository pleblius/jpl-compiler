#ifndef COMPILER_H
#define COMPILER_H

#include "vector.h"

typedef enum { NO_PRINT, STANDARD_PRINT, PRETTY_PRINT, TABBED_PRINT, XML_PRINT } PrintMode;
typedef enum { HELP_MODE, LEX_MODE, PARSE_MODE, TYPE_MODE } RunMode;

typedef struct {
    RunMode run_mode;
    char* file_name;
    PrintMode print_mode;
} RunType;

void parse_args(int, char**);
int help_mode(char*);
int lex_mode();
int parse_mode();
int type_mode();
void free_token_list(Vector*);

#endif // COMPILER_H
