#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vector.h"
#include "lexer.h"
#include "compiler.h"
#include "parser.h"
#include "error.h"

#define MAXIMUM_BUFFER 1024

#define DEBUG 0

Vector *token_list;
FILE *file_ptr;
RunType mode;
char *file_string;
size_t file_size;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Filename required.\n");
        exit(EXIT_FAILURE);
    }
    
    token_list = vector_create();    
    if (!token_list) {
        fprintf(stderr, "Failed to create vector.\n");
        exit(EXIT_FAILURE);
    }

    // Parse cli arguments
    parse_args(argc, argv);

    // Run compiler
    if (mode.run_mode == NULL) return EXIT_FAILURE;
    
    file_ptr = fopen(mode.file_name, "r");
    if (!file_ptr) {
        fprintf(stderr, "Failed to open file '%s':\n\t%s.\nCompilation failed.\n", mode.file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fseek(file_ptr, 0, SEEK_END);
    file_size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);
    file_string = malloc(file_size + 1);
    if (!file_string) {
        fprintf(stderr, "Failed to allocated string memory.\n");
        return EXIT_FAILURE;
    }

    int run_status = fread(file_string, 1, file_size, file_ptr);
    if (run_status == -1) {
        fprintf(stderr, "String read error: %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ((size_t) run_status < file_size) {
        fprintf(stderr, "Insufficient buffer space.\n");
        exit(EXIT_FAILURE);
    }
    file_string[file_size] = '\0';

    run_status = mode.run_mode(mode.file_name);

    #if DEBUG
    free_token_list(token_list);
    free_command_list();
    token_list = NULL;
    free(file_string);
    #endif // DEBUG

    if (run_status != 0)
        program_error(file_string, mode.file_name, token_list);

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
                case ('p'):
                    if (!flag_bool) {
                        mode.run_mode = &parse_mode;
                        flag_bool = 1;
                    }
                    break;
                case ('h'):
                    mode.run_mode = &help_mode;
                    mode.file_name = "./HELP.md";
                    break;
                default:
                    printf("Invalid flag.\n\n");
                    mode.run_mode = &help_mode;
                    mode.file_name = "./HELP.md";
                    return;
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
        fprintf(stderr, "Failed to open file '%s':\n\t%s.\nCompilation failed.\n", mode.file_name, strerror(errno));
        return 1;
    }

    char buffer[MAXIMUM_BUFFER];

    char *line;
    while ((line = fgets(buffer, MAXIMUM_BUFFER, f_ptr)) != NULL) {
        printf("%s", line);
    }

    return EXIT_SUCCESS;
}

int lex_mode(char* filename) {
    (void) filename;

    int exit_status;

    exit_status = lex();
    
    if (exit_status == EXIT_SUCCESS) {
        lex_print_output();
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

int parse_mode(char* filename) {
    (void) filename;

    int exit_status;

    lex();
    exit_status = parse();
    if (exit_status == EXIT_SUCCESS)
        parse_print_output();

    return exit_status;
}

void free_token_list(Vector *vector) {
    vector_destroy(vector);
}
