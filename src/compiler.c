#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vector.h"
#include "lexer.h"
#include "compiler.h"
#include "parser.h"
#include "error.h"
#include "vecs.h"
#include "printer.h"
#include "dict.h"

#define BUFFER 1024

#define DEBUG 0

TokenVec *token_vec;
Vector *command_list;
FILE *file_ptr;
RunType mode;
char *file_string;
size_t file_size;
CVec *print_buffer;
TypeDict *type_dict;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Filename required.\n");
        exit(EXIT_FAILURE);
    }

    // Default mode
    mode.print_mode = STANDARD_PRINT;
    mode.run_mode = TYPE_MODE;

    // Parse cli arguments
    parse_args(argc, argv);

    // Open file
    file_ptr = fopen(mode.file_name, "r");
    if (!file_ptr) {
        fprintf(stderr, "Failed to open file '%s':\n\t%s.\nCompilation failed.\n", mode.file_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Load file into string
    fseek(file_ptr, 0, SEEK_END);
    file_size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);
    file_string = malloc(file_size + 1);
    if (!file_string) return EXIT_FAILURE;
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

    // Token vector
    token_vec = tokenvec_create_cap(file_size/2);
    if (!token_vec) return EXIT_FAILURE;

    // Printout buffer string
    print_buffer = cvec_create_cap(file_size * 2);

    // Typecheck hashmap
    type_dict = dict_create_big();
    if (!type_dict) return EXIT_FAILURE;

    // RUN
    int run_status;
    switch (mode.run_mode) {
        case HELP_MODE:
            run_status = help_mode(mode.file_name);
            break;
        case LEX_MODE:
            run_status = lex_mode();
            break;
        case PARSE_MODE:
            run_status = parse_mode();
            break;
        case TYPE_MODE:
            run_status = type_mode();
            break;
        default:
            return EXIT_FAILURE;
    }
    fflush(stdout);

    #if DEBUG
    free_token_list(token_list);
    free_command_list();
    token_list = NULL;
    free(file_string);
    #endif // DEBUG

    if (run_status != 0)
        program_error(mode.file_name);

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
                        mode.run_mode = LEX_MODE;
                        flag_bool = 1;
                    }
                    break;
                case ('p'):
                    if (!flag_bool) {
                        mode.run_mode = PARSE_MODE;
                        flag_bool = 1;
                    }
                    break;
                case ('t'):
                    if (!flag_bool) {
                        mode.run_mode = TYPE_MODE;
                        flag_bool = 1;
                    }
                    break;
                case ('h'):
                    mode.run_mode = HELP_MODE;
                    mode.file_name = "./HELP.md";
                    return;
                case ('-'):
                    if (!strcmp("pp-sexp", input_args[0]+1))
                        mode.print_mode = PRETTY_PRINT;
                    else if (!strcmp("tab-sexp", input_args[0]+1))
                        mode.print_mode = TABBED_PRINT;
                    else if (!strcmp("standard-sexp", input_args[0]+1))
                        mode.print_mode = STANDARD_PRINT;
                    else if (!strcmp("xml-sexp", input_args[0]+1))
                        mode.print_mode = XML_PRINT;
                    else if (!strcmp("no-print", input_args[0]+1))
                        mode.print_mode = NO_PRINT;
                    break;
                default:
                    printf("Invalid flag.\n\n");
                    mode.run_mode = HELP_MODE;
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

    char buffer[BUFFER];

    char *line;
    while ((line = fgets(buffer, BUFFER, f_ptr)) != NULL) {
        printf("%s", line);
    }

    return EXIT_SUCCESS;
}

int lex_mode() {
    int exit_status;

    exit_status = lex();
    
    if (exit_status == EXIT_SUCCESS && mode.print_mode != NO_PRINT) {
        print_lexed_tokens(token_vec);
        printf("Compilation Succeeded.\n");
    }
    else {
        printf("Compilation Failed.\n");
    }

    return exit_status;
}

int parse_mode() {
    int exit_status;

    lex();
    exit_status = parse(token_vec);
    if (exit_status == EXIT_SUCCESS && mode.print_mode != NO_PRINT) {
        print_parsed_nodes(command_list);
        printf("Compilation Succeeded.\n");
    }

    return exit_status;
}

int type_mode() {
    int exit_status;

    lex();
    exit_status = parse(token_vec);
    if (exit_status == EXIT_SUCCESS && mode.print_mode != NO_PRINT) {
        print_typed_nodes(command_list);
        printf("Compilation Succeeded.\n");
    }

    return exit_status;
}

void free_token_list(Vector *vector) {
    vector_destroy(vector);
}
