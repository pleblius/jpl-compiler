#include <stdio.h>
#include <stdlib.h>
#include "token.h"

char *token_names[] = { "ARRAY", "ASSERT", "BOOL", "COLON", "COMMA", "DOT", "ELSE", "END_OF_FILE",
    "EQUALS", "FALSE", "FLOAT", "FLOATVAL", "FN", "IF", "IMAGE", "INT", "INTVAL",
    "LCURLY",  "LET", "LPAREN", "LSQUARE", "NEWLINE", "OP", "PRINT", "RCURLY", 
    "READ", "RETURN", "RPAREN", "RSQUARE", "SHOW", "STRING", "STRUCT", "SUM", 
    "THEN", "TIME", "TO", "TRUE", "VARIABLE", "VOID", "WRITE" };
    
Token create_token(TokenType type, size_t start, size_t count, char *pc) {
    return (Token) {type, start, (StringRef) {count, pc}};
}
