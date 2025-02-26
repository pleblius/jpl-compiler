#include <stdio.h>
#include <stdlib.h>
#include "token.h"

Token create_token(TokenType type, size_t start, size_t count, char *pc) {
    Token token;

    token.type = type;
    token.loc = start;
    token.strref.length = count;
    token.strref.string = pc;

    return token;
}
