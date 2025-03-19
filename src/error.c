#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "stringops.h"
#include "error.h"
#include "vector.h"

#define LINE_WIDTH 120
#define PRINT_WIDTH 100

#define RESET_ALL          "\x1b[0m"

#define COLOR_BLACK        "\x1b[30m"
#define COLOR_RED          "\x1b[31m"
#define COLOR_GREEN        "\x1b[32m"
#define COLOR_YELLOW       "\x1b[33m"
#define COLOR_BLUE         "\x1b[34m"
#define COLOR_MAGENTA      "\x1b[35m"
#define COLOR_CYAN         "\x1b[36m"
#define COLOR_WHITE        "\x1b[37m"

#define BACKGROUND_BLACK   "\x1b[40m"
#define BACKGROUND_RED     "\x1b[41m"
#define BACKGROUND_GREEN   "\x1b[42m"
#define BACKGROUND_YELLOW  "\x1b[43m"
#define BACKGROUND_BLUE    "\x1b[44m"
#define BACKGROUND_MAGENTA "\x1b[45m"
#define BACKGROUND_CYAN    "\x1b[46m"
#define BACKGROUND_WHITE   "\x1b[47m"

#define STYLE_BOLD         "\x1b[1m"
#define STYLE_ITALIC       "\x1b[3m"
#define STYLE_UNDERLINE    "\x1b[4m"

static char *file_string;
static char *file_name;
static TokenVec *token_list;

void error_setup(char *file, char *string) {
    if (!file || !string) return;

    file_string = string;
    file_name = file;
}

void token_list_setup(TokenVec* tokens) {
    token_list = tokens;
}

void add_lex_error(LexErrorType type, Token *token) {
    if (!token) return;

    ErrorToken new_error = (ErrorToken) { LEX_ERROR, {.lex_error = type}, token, NULL };
    print_lex_error(&new_error);
}

void add_parse_error(ParseErrorType type, Token *error_token, Token *ref_token) {
    if (!error_token) return;
    ErrorToken new_error = (ErrorToken) { PARSE_ERROR, {.parse_error = type}, error_token, ref_token };
    print_parse_error(&new_error);
}

void add_type_error(TypeErrorType type, AstNode *error_node, AstNode *ref_node) {
    if (!ref_node || !error_node) return;
    ErrorToken new_error = (ErrorToken) { TYPE_ERROR, {.type_error = type}, error_node, ref_node };
    print_type_error(&new_error);
}

void print_lex_error(ErrorToken *error) {
    if (!error) return;

    Token *token = error->first_token;

    uint32_t line = 1;
    uint32_t col = 1;
    get_error_loc(token, &col, &line);

    printf("Lex error at %s:%d:%d\n\n", file_name, line, col);

    char c;
    switch (error->error_subtype.lex_error) {
        case UNCLOSED_STRING:
            printf("\tUnclosed string:\n\n");
            print_one_token_line(token);
            break;
        case INVALID_LEX:
            print_one_token_line(token);
            break;
        case ILLEGAL_LEX:
            c = *token->strref.string;
            printf("\tIllegal character: " COLOR_RED STYLE_BOLD "'%d'\n\n" RESET_ALL, c);
            break;
    }
}

void print_parse_error(ErrorToken *error) {
    if (!error || !error->first_token) return;

    Token *first_token = error->first_token;
    Token *second_token = error->second_token;

    uint32_t line = 1;
    uint32_t col = 1;
    get_error_loc(first_token, &col, &line);

    printf("Parse error at %s:%d:%d\n", file_name, line, col);

    size_t length1 = first_token->strref.length;
    size_t length2 = 0;
    if (second_token != NULL) length2 = second_token->strref.length;
    char misc_1[length1 + 1];
    char misc_2[length2 + 1];

    strncpy(misc_1, first_token->strref.string, length1);
    misc_1[length1] = '\0';

    if (second_token != NULL)
        strncpy(misc_2, second_token->strref.string, length2);
    misc_2[length2] = '\0';

    char *string1;

    switch (first_token->type) {
        case NEWLINE:
            string1 = "NEWLINE";
            break;
        case END_OF_FILE:
            string1 = "END_OF_FILE";
            break;
        default:
            string1 = misc_1;
    }

    char *string2;

    if (second_token == NULL) string2 = misc_2;
    else {
        switch (second_token->type) {
            case NEWLINE:
                string2 = "NEWLINE";
                break;
            case END_OF_FILE:
                string2 = "END_OF_FILE";
                break;
            default:
                string2 = misc_2;
        }
    }

    switch (error->error_subtype.parse_error) {
        case UNEXPECTED_TOKEN:
            printf("\tUnexpected token type encountered.\n\tExpected type '%s', found: '%s'\n\n", string2, string1);
            print_one_token_line(first_token);
            break;
        case UNCLOSED_PAREN:
            printf("\tUnclosed parenthetical.\n\n");
            print_two_token_line(second_token, first_token);
            break;
        case INT_RANGE:
            printf("\tInteger value exceeds INTMAX.\n\n");
            print_one_token_line(first_token);
            break;
        case FLOAT_RANGE:
            printf("\tFloating point value exceeds DOUBLEMAX.\n\n");
            print_one_token_line(first_token);
            break;
        case BAD_BIND:
            printf("\tExpected BINDING, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_CMD:
            printf("\tExpected COMMAND, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_EXPR:
            printf("\tExpected EXPRESSION, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_LVALUE:
            printf("\tExpected LVALUE, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_STMT:
            printf("\tExpected STATEMENT, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;  
        case BAD_TYPE:
            printf("\tExpected TYPE, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_UNARY:
            printf("\tInvalid unary operator; expected '-' or '!', found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        case BAD_BINARY:
            printf("\tInvalid binary operator; expected boolean or math operator, found '%s'\n\n", string1);
            print_one_token_line(first_token);
            break;
        default:
            return;
    }
}

void print_type_error(ErrorToken *error) {
    AstNode *error_node = error->first_token;
    AstNode *ref_node = error->second_token;
    Token *error_token = tokenvec_get(token_list, error_node->token_index);
    Token *ref_token = tokenvec_get(token_list, ref_node->token_index);
    uint32_t line = 1;
    uint32_t col = 1;
    get_error_loc(error_token, &col, &line);

    printf("Type-check error at %s:%d:%d\n", file_name, line, col);

    if (!error_node->string.string) {
        error_node->string = (StringRef) {0, ""};
    }

    char *string2 = NULL;
    char *string1 = malloc(error_node->string.length + 1); if (!string1) return;
    strncpy(string1, error_node->string.string, error_node->string.length);
    string1[error_node->string.length] = '\0';

    uint32_t string2_len;
    if (!ref_node->string.string || !ref_node->string.length)
        string2_len = 0;
    else
        string2_len = ref_node->string.length;

    string2 = malloc(string2_len + 1); if (!string2) return;
    if (ref_node->string.string != NULL)
        strncpy(string2, ref_node->string.string, string2_len);
    string2[string2_len] = '\0';

    switch (error->error_subtype.type_error) {
        case UNEXPECTED_TYPE:
            printf("\tUnexpected expression type encountered.\n\tExpected: %s\n\tFound: %s\n\n", string2, string1);
            print_one_token_line(error_token);
            break;
        case UNRESOLVED_TYPE:
            printf("\tReference to variable '%s' of unresolved type.\n\n", string1);
            print_one_token_line(error_token);
            break;
        case MISMATCHED_BINOP:
            printf("\tAttempted binary operation on incompatible types: '%s' and '%s'\n\n", string1, string2);
            print_two_token_line(error_token, ref_token);
            break;
        case MISMATCHED_IF:
            printf("\tIf-Then-Else expression returns incompatible types on separate branches\n\n");
            print_two_token_line(ref_token, error_token);
            break;
        case BAD_BINOP:
            printf("\tAttempted binary operation '%s' on invalid type: '%s'\n\n", string2, string1);
            print_two_token_line(error_token, ref_token);
            break;
        case BAD_UNOP:
            printf("\tAttempted unary operation '%s' on invalid type: '%s'\n\n", string1, string2);
            print_two_token_line(ref_token, error_token);
            break;
        case BAD_INDEX:
            printf("\tAttempt to index array with non-integer type\n\n");
            print_one_token_line(error_token);
            break;
        case BAD_DERFERENCE:
            printf("\tAttempt to dereference non-struct variable '%s'\n\n", string1);
            print_one_token_line(error_token);
            break;
        case BAD_MEMBER:
            printf("\tAttempt to dereference invalid member '%s' from struct type '%s'\n\n", string1, string2);
            print_one_token_line(error_token);
            printf("\tStruct type declared here:\n\n");
            print_one_token_line(ref_token);
            break;
        case UNDECLARED_VARIABLE:
            printf("\tAttempt to access undeclared variable: '%s'\n\n", string1);
            print_one_token_line(error_token);
            break;
        case SHADOWED_VARIABLE:
            printf("\tVariable '%s' shadows previously declared variable.\n\n", string1);
            print_one_token_line(error_token);
            printf("\tDeclared here:\n\n");
            print_one_token_line(ref_token);
            break;
        case NO_RETURN:
            printf("\tFunction '%s' missing return statement.\n\n", string1);
            print_one_token_line(error_token);
            break;
        case BAD_RETURN:
            printf("\tStatement returns invalid type for function '%s'\n\n", string1);
            print_one_token_line(error_token);
            printf("\tDeclared here:\n\n");
            print_one_token_line(ref_token);
            break;
        case STRUCT_ASSIGN:
            printf("\tAttempt to assign struct type to variable '%s'\n\n", string1);
            print_one_token_line(error_token);
            printf("\tDeclared here:\n\n");
            print_one_token_line(ref_token);
            break;
        case VOID_ASSIGN:
            printf("\tAttempt to assign VOID type value to variable '%s'\n\n", string1);
            print_one_token_line(error_token);
            break;
        case MISMATCHED_MEMBERS:
            printf("\tStruct members misaligned for struct '%s'\n\n", string2);
            print_one_token_line(error_token);
            printf("\tDeclared here:\n\n");
            print_one_token_line(ref_token);
            break;
        case EMPTY_ARRAY:
            printf("\tArray literal constructor cannot be empty.\n\n");
            print_one_token_line(error_token);
            break;
        case MISMATCHED_ARRAY:
            printf("\tNon-matching array index types\n\n");
            print_two_token_line(ref_token, error_token);
            break;
        case NO_INDEX:
            printf("\tArray index cannot be empty.\n\n");
            print_one_token_line(error_token);
            break;
        case BAD_RANK:
            printf("\tArray rank doesn't match assignment rank.\n\n");
            print_two_token_line(error_token, ref_token);
            break;
        case BAD_FN:
            printf("\tAttempt to call non-function variable '%s'\n\n", string1);
            print_one_token_line(error_token);
            printf("\tDeclared here:\n\n");
            print_one_token_line(ref_token);
            break;
        case BAD_DIMENSION:
            printf("\tAttempt to index array with non-matching dimension.\n\n");
            print_two_token_line(error_token, ref_token);
            break;
        case BAD_SUM:
            printf("\tAttempt to add non-numeric expresion\n\n");
            print_one_token_line(error_token);
            break;
    }

    free(string1);
    free(string2);
}

void print_one_token_line(Token *token) {
    if (!token) return;

    uint32_t line = 1;
    uint32_t col = 1;
    get_error_loc(token, &col, &line);

    StringRef string = token->strref;
    uint32_t span = string.length;

    uint32_t post_span = 0;
    char *pc = string.string + string.length;
    while (1) {
        if (token->type == NEWLINE || token->type == END_OF_FILE) break;
        char c = *(pc + post_span);
        if (c == '\n' || c == '\0') break;
        ++post_span;
    }

    char prefix[col];
    char postfix[post_span + 1];
    char error[span + 1];

    strncpy(prefix, string.string-(col-1), col);

    if (token->type == NEWLINE || token->type == END_OF_FILE)
        error[0] = ' ';
    else
        strncpy(error, string.string, string.length);

    strncpy(postfix, string.string + string.length, post_span);

    prefix[col-1] = '\0';
    error[span] = '\0';
    postfix[post_span] = '\0';

    printf("%6d | %s" COLOR_RED STYLE_BOLD "%s" RESET_ALL "%s\n", line, prefix, error, postfix);

    size_t prefix_len = strlen(prefix) + 9;
    size_t i;

    printf(COLOR_RED STYLE_BOLD);
    for (i = 0; i < prefix_len; ++i) {
        fputc(' ', stdout);
    }
    putc ('^', stdout);
    for (i = 0; i < span-1; ++i) {
        fputc('~', stdout);
    }
    printf(RESET_ALL "\n\n");
}

void print_two_token_line(Token *start_token, Token *end_token) {
    if (!start_token|| !end_token) return;

    uint32_t start_line = 1;
    uint32_t start_col = 1;
    get_error_loc(start_token, &start_col, &start_line);

    uint32_t end_line = 1;
    uint32_t end_col = 1;
    get_error_loc(end_token, &end_col, &end_line);

    if (end_line != start_line) {
        print_one_token_line(start_token);
        print_one_token_line(end_token);
        return;
    }

    StringRef start_string = start_token->strref;
    StringRef end_string = end_token->strref;
    if (!(start_string.string < end_string.string)) {
        StringRef temp = start_string;
        uint32_t temp_col = start_col;
        start_string = end_string;
        start_col = end_col;
        end_string = temp;
        end_col = temp_col;
    }
    uint32_t start_span = start_string.length;
    uint32_t end_span = end_string.length;

    uint32_t mid_span = 0;
    char *pc = start_string.string + start_span;
    while (pc + mid_span != end_string.string) {
        if (start_token->type == NEWLINE || start_token->type == END_OF_FILE) break;
        char c = *(pc + mid_span);
        if (c == '\n' || c == '\0') break;
        ++mid_span;
    }

    uint32_t post_span = 0;
    pc = end_string.string + end_span;
    while (1) {
        if (end_token->type == NEWLINE || end_token->type == END_OF_FILE) break;
        char c = *(pc + post_span);
        if (c == '\n' || c == '\0') break;
        ++post_span;
    }

    char prefix[start_col];
    char start[start_span + 1];
    char midfix[mid_span + 1];
    char end[end_span + 1];
    char postfix[post_span + 1];

    strncpy(prefix, start_string.string-(start_col-1), start_col);
    prefix[start_col-1] = '\0';

    if (start_token->type == NEWLINE || start_token->type == END_OF_FILE)
        start[0] = ' ';
    else
        strncpy(start, start_string.string, start_string.length);
    
    start[start_span] = '\0';

    strncpy(midfix, start_string.string + start_string.length, mid_span);
    midfix[mid_span] = '\0';

    if (end_token->type == NEWLINE || end_token->type == END_OF_FILE)
        end[0] = ' ';
    else
        strncpy(end, end_string.string, end_string.length);

    end[end_span] = '\0';

    strncpy(postfix, end_string.string + end_string.length, post_span);
    postfix[post_span] = '\0';

    printf("%6d | %s" COLOR_RED STYLE_BOLD "%s" RESET_ALL "%s" COLOR_RED STYLE_BOLD "%s" RESET_ALL "%s\n", 
        start_line, prefix, start, midfix, end, postfix);

    size_t prefix_len = strlen(prefix) + 9;
    size_t i;

    printf(COLOR_RED STYLE_BOLD);
    for (i = 0; i < prefix_len; ++i) {
        fputc(' ', stdout);
    }
    fputc ('^', stdout);
    for (i = 0; i < start_span-1; ++i) {
        fputc('~', stdout);
    }
    for (i = 0; i < mid_span; ++i) {
        fputc(' ', stdout);
    }
    fputc('^', stdout);
    for (i = 0; i < end_span-1; ++i) {
        fputc('~', stdout);
    }
    printf(RESET_ALL "\n\n");
}

void get_error_loc(Token *token, uint32_t *col, uint32_t *line) {
    if (!token || !col || !line) return;

    uint32_t loc = token->loc;

    *line = 1;
    *col = 1;

    for (uint32_t i = 0; i < loc; ++i) {
        if (file_string[i] == '\n') {
            ++(*line);
            *col = 1;
        }
        else
            ++(*col);
    }
}
