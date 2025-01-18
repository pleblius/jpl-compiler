#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vector.h"
#include "lexer.h"

#define MAXIMUM_BUFFER 1024

typedef struct {
    int (*run_mode)(char*);
    char* file_name;
} run_t;

void parse_args(int, char**);

int help_mode(char*);
int lex_mode(char*);
int parse_mode(char*);

void free_token_list(Vector*);

Vector *list;
run_t mode;
char buffer[MAXIMUM_BUFFER];

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Filename required.\n");
        exit(EXIT_FAILURE);
    }
    
    list = vector_create();    
    if (!list) {
        fprintf(stderr, "Failed to create vector.\n");
        exit(EXIT_FAILURE);
    }

    // Parse cli arguments
    parse_args(argc, argv);

    // Run compiler
    if (mode.run_mode == NULL) return EXIT_FAILURE;

    int run_status = mode.run_mode(mode.file_name);

    free_token_list(list);
    list = NULL;

    return run_status;
}

// Parses the command line arguments for this program, setting flags as appropriate. Arguments necessary for operation are stored in output_args.
// Returns the number of output arguments. Exits on invalid arguments.
void parse_args(int input_count, char **input_args) {
    char c;
    int flag_bool = 0;
    int file_bool = 0;

    // Default operating mode
    // mode.run_mode = &compile_mode;

    // Loop through args looking for flags. Ignores first arg (filename)
    for (; input_count > 1; --input_count) {
        // Flags
        if ((*++input_args)[0] == '-') {
            c = *++input_args[0];

            switch (c) {
                case ('l'):
                    if (!flag_bool) {
                        mode.run_mode = &lex_mode;
                        flag_bool = 1;
                    }
                    break;

                case ('h'):
                    mode.run_mode = &help_mode;
                    mode.file_name = "./HELP.md";
                    return;

                default:
                    printf("Invalid flag. Use -h for help.\n");
                    exit(EXIT_FAILURE);
            }
        }
        // Filenames
        else {
            if (!file_bool) {
                mode.file_name = *input_args;
                file_bool = 1;
            }
        }
    }
}

int help_mode(char* filename) {
    FILE *f_ptr = fopen(filename, "r");
    if (!f_ptr) {
        fprintf(stderr, "Failed to open file: %s.\n", strerror(errno));
        return 1;
    }

    char *line;
    while ((line = fgets(buffer, MAXIMUM_BUFFER, f_ptr)) != NULL) {
        printf("%s", line);
    }
    return EXIT_SUCCESS;
}

int lex_mode(char* filename) {
    int exit_status;

    exit_status = lex(filename);
    
    if (exit_status == EXIT_SUCCESS)
        lex_print_output();
    else
        lex_print_fail();

    return exit_status;
}

int parse_mode(char* filename) {
    int exit_status;

    exit_status = lex(filename);
    if (exit_status == EXIT_FAILURE)
        return exit_status;

    return exit_status;
}

void free_token_list(Vector *vector) {
    int i;
    for (i = 0; i < vector->size; ++i) {
        free_token_string((Token *) vector_get(vector, i));
    }

    vector_destroy(vector);
}
