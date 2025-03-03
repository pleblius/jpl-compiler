#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "main.h"
#include "lexer.h"
#include "token.h"
#include "vecs.h"
#include "vector.h"
#include "printer.h"
#include "error.h"
#include "parser.h"

static RunMode run_mode = RUN_MODE;
static PrintMode print_mode = STANDARD_PRINT;
static char *file_name;
static char *file_string;
static size_t file_size;
static TokenVec *token_vector;
static NodeVec *node_vector;
static Vector *cmd_vector;

int main(int argc, char *argv[]) {
    if (parse_input_args(argc, argv) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (run_mode == HELP_MODE)
        return run_help();
    
    if (open_file() == EXIT_FAILURE)
        return EXIT_FAILURE;

    if (run_compilation() == EXIT_FAILURE) {
        print_fail();
        return EXIT_FAILURE;
    }
    else if (print_mode != NO_PRINT) {
        print_success();
    }
    
    return EXIT_SUCCESS;
}

int parse_input_args(int argc, char *argv[]) {
    int mode_set = 0;
    int file_set = 0;
    for (int i = 1; i < argc; ++i) {
        char c;

        // Flag
        if (*argv[i] == '-')
            c = *++argv[i];
        // Filename
        else if (!file_set) {
            file_name = argv[i];
            file_set = 1;
            continue;
        }
        // Redundant filename
        else continue;

        switch (c) {
            case 'h':
                run_mode = HELP_MODE;
                return EXIT_SUCCESS;
            case 'l':
                if (!mode_set) {
                    run_mode = LEX_MODE;
                    mode_set = 1;
                }
                break;
            case 'p':
                if (!mode_set) {
                    run_mode = PARSE_MODE;
                    mode_set = 1;
                }
                break;
            case 't':
                if (!mode_set) {
                    run_mode = TYPE_MODE;
                    mode_set = 1;
                }
                break;
            case 'c':
                if (!mode_set) {
                    run_mode = C_MODE;
                    mode_set = 1;
                }
                break;
            case 'r':
                if (!mode_set) {
                    run_mode = RUN_MODE;
                    mode_set = 1;
                }
                break;
            case '-':
                ++argv[i];
                if (!strcmp(argv[i], "no-print")) {
                    print_mode = NO_PRINT;
                } else if (!strcmp(argv[i], "standard-print")) {
                    print_mode = STANDARD_PRINT;
                } else if (!strcmp(argv[i], "tabbed-print")) {
                    // TODO
                } else if (!strcmp(argv[i], "pp-print")) {
                    // TODO 
                } else if (!strcmp(argv[i], "xml-print")) {
                    // TODO
                }
                else {
                    invalid_args(argv[i]);
                    return EXIT_FAILURE;
                }
                break;
            default:
                invalid_args(argv[i]);
                return EXIT_FAILURE;
        }
    }

    if (!file_set) {
        missing_filename();
        return EXIT_FAILURE;
    }
    if (!mode_set) {
        missing_runmode();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void insufficient_args() {
    printf("Insufficient arguments. Need run flag and filename.\n");
}

void invalid_args(char *arg) {
    printf("Invalid argument: %s\n Use '-h' for a list of valid arguments.\n", arg);
}

void missing_filename() {
    printf("No filename provided. Compilation aborted.\n");
}

void missing_runmode() {
    printf("No run flag provided. Compilation aborted.\n");
}

int run_help() {
    file_name = "HELP.md";
    FILE *f_ptr = fopen(file_name, "r");
    if (!f_ptr) {
        printf("Failed to open file 'HELP.md': %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (fseek(f_ptr, 0, SEEK_END) != 0) return EXIT_FAILURE;
    file_size = ftell(f_ptr);
    if (fseek(f_ptr, 0, SEEK_SET) != 0) return EXIT_FAILURE;

    file_string = malloc(file_size); if (!file_string) return EXIT_FAILURE;
    if (!(fread(file_string, 1, file_size, f_ptr))) return EXIT_FAILURE;
    if (fputs(file_string, stdout) == -1) {
        printf("Failed to write: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    fclose(f_ptr);
    tokenvec_destroy(token_vector);
    free(node_vector);

    return EXIT_SUCCESS;
}

int open_file() {
    if (!file_name) return EXIT_FAILURE;

    FILE *f_ptr = fopen(file_name, "r");
    if (!f_ptr) {
        printf("Failed to open file %s: %s\n", file_name, strerror(errno));
        return EXIT_FAILURE;
    }

    if (fseek(f_ptr, 0, SEEK_END) != 0) return EXIT_FAILURE;
    file_size = ftell(f_ptr);
    if (fseek(f_ptr, 0, SEEK_SET) != 0) return EXIT_FAILURE;

    file_string = malloc(file_size + 1); if (!file_string) return EXIT_FAILURE;
    file_string[file_size] = '\0';

    if (fread(file_string, 1, file_size, f_ptr) != file_size)
        if (ferror(f_ptr)) {
            printf("Error occurred while reading file %s\n", file_name);
            return EXIT_FAILURE;
        }

    return EXIT_SUCCESS;
}

int run_compilation() {
    int exit_status = EXIT_SUCCESS;
    error_setup(file_name, file_string);

    switch (run_mode) {
        case LEX_MODE:
            exit_status = lex_string(file_string, file_size, &token_vector);
            break;
        case PARSE_MODE:
            lex_string(file_string, file_size, &token_vector);
            exit_status = parse_tokens(token_vector, &node_vector, &cmd_vector);
            break;
        case TYPE_MODE:
            lex_string(file_string, file_size, &token_vector);
            // return type_tokens(token_vector, token_vector->size, node_vector);
            break;
        case C_MODE:
            // Fall
        case RUN_MODE:
            printf("Not implemented.\n");
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
    }

    return exit_status;
}

void print_fail() {
    if (print_mode == NO_PRINT) {
        clear_errors();
        return;
    }

    print_errors(token_vector);
    clear_errors();

    printf("Compilation failed\n");
}

void print_success() {
    switch (run_mode) {
        case LEX_MODE:
            print_tokens(token_vector);
            break;
        case PARSE_MODE:
            print_nodes(node_vector, cmd_vector, token_vector);
            break;
        case TYPE_MODE:
        case C_MODE:
        case RUN_MODE:
        default:
            return;
    }

    printf("Compilation succeeded\n");
}

void gen_defines();
