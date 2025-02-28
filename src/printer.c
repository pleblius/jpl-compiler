#include <stdio.h>

#include "printer.h"
#include "token.h"

char *token_output[] = { "ARRAY '", "ASSERT '", "BOOL '", "COLON '", "COMMA '", "DOT '", "ELSE '", "END_OF_FILE",
                        "EQUALS '", "FALSE '", "FLOAT '", "FLOATVAL '", "FN '", "IF '", "IMAGE '", "INT '", "INTVAL '",
                        "LCURLY '",  "LET '", "LPAREN '", "LSQUARE '", "NEWLINE", "OP '", "PRINT '", "RCURLY '", 
                        "READ '", "RETURN '", "RPAREN '", "RSQUARE '", "SHOW '", "STRING '", "STRUCT '", "SUM '", 
                        "THEN '", "TIME '", "TO '", "TRUE '", "VARIABLE '", "VOID '", "WRITE '" };


CVec *print_buffer;

void print_tokens(TokenVec *vector, PrintMode mode) {
    if (mode == NO_PRINT) return;

    Token *token;
    size_t num_tokens = sizeof(token_output)/sizeof(char*);
    print_buffer = cvec_create_cap(vector->size << 8); if (!print_buffer) return;
    
    char *token_string;
    for (size_t i = 0; i < vector->size; ++i) {
        token = tokenvec_get(vector, i);
        if (token->type >= num_tokens) {
            fprintf(stderr, "Token string index out of bounds.\n");
            exit(EXIT_FAILURE);
        }
        token_string = token_output[token->type];
        
        switch (token->type) {
            case NEWLINE:
            case END_OF_FILE:
                cvec_append_array_line(print_buffer, token_string, strlen(token_string));
                break;
            default:
                cvec_append_array(print_buffer, token_string, strlen(token_string));
                cvec_append_ref(print_buffer, token->strref);
                cvec_append_array(print_buffer, "\'\n", 2);
        }
    }
    cvec_print(print_buffer);
    cvec_destroy(print_buffer);
}
