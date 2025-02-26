#ifndef MAIN_H
#define MAIN_H

typedef enum { HELP_MODE, LEX_MODE, PARSE_MODE, TYPE_MODE, C_MODE, COMPILE_MODE, RUN_MODE } RunMode;
typedef enum { STANDARD_PRINT, NO_PRINT, PRETTY_PRINT, TABBED_PRINT, XML_PRINT } PrintMode;

#define LINE_SIZE 120

int parse_input_args(int, char*[]);
void insufficient_args();
void invalid_args(char*);
void missing_filename();
void missing_runmode();
int run_help();
int open_file();
int run_compilation();
void print_success();
void print_fail();
void gen_defines();

#endif // MAIN_H
