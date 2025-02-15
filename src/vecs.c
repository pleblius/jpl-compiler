#include "vecs.h"
#include "parser.h"

#define VECTOR_SIZE (vector->size)
#define VECTOR_CAPACITY (vector->capacity)
#define VECTOR_IS_EMPTY (!VECTOR_SIZE)
#define VECTOR_ARRAY (vector->array)
#define LAST_INDEX (VECTOR_SIZE-1)
#define CAPACITY_DEFAULT 10

CVec *cvec_create_cap(size_t capacity) {
    if (capacity == 0) return NULL;

    CVec *vector = (CVec *) malloc(sizeof(CVec));
    if (!vector) return NULL;

    VECTOR_ARRAY = (char *) malloc(sizeof(char) * capacity);
    if (!VECTOR_ARRAY) {
        free(vector);
        return NULL;
    }

    VECTOR_SIZE = 0;
    VECTOR_CAPACITY = capacity;

    return vector;
}

CVec *cvec_create() {
    return cvec_create_cap(CAPACITY_DEFAULT);
}

void cvec_expand(CVec* vector) {
    if (!vector) return;

    char *temp_array = VECTOR_ARRAY;
    
    VECTOR_ARRAY = (char *) malloc(sizeof(char) * VECTOR_CAPACITY * 2);
    if (!VECTOR_ARRAY) return;

    memcpy(VECTOR_ARRAY, temp_array, VECTOR_CAPACITY * sizeof(char));

    VECTOR_CAPACITY *= 2;
    free(temp_array);
}

void cvec_shrink(CVec* vector) {
    if (!vector) return;

    char *temp_array = VECTOR_ARRAY;

    VECTOR_ARRAY = (char *) malloc(sizeof(char) * VECTOR_CAPACITY / 2);
    if (!VECTOR_ARRAY) return;

    VECTOR_CAPACITY /= 2;

    memcpy(VECTOR_ARRAY, temp_array, VECTOR_CAPACITY * sizeof(char));

    free(temp_array);
}

void cvec_append(CVec* vector, char c) {
    if (!vector) return;

    if (VECTOR_SIZE == VECTOR_CAPACITY) cvec_expand(vector);

    VECTOR_ARRAY[VECTOR_SIZE++] = c;
}

void cvec_append_ref(CVec *vector, StringRef string) {
    size_t cap = VECTOR_CAPACITY;
    if (VECTOR_SIZE + string.length > cap) {
        while (VECTOR_SIZE + string.length > cap) {
            cap *= 2;
        }

        char *temp = malloc(cap); if (!temp) return;
        strncpy(temp, VECTOR_ARRAY, VECTOR_SIZE);
        free(VECTOR_ARRAY);
        VECTOR_ARRAY = temp;
        VECTOR_CAPACITY = cap;
    }
    
    strncpy(&VECTOR_ARRAY[VECTOR_SIZE], string.string, string.length);
    VECTOR_SIZE += string.length;
}

void cvec_append_array(CVec *vector, const char *string, size_t len) {
    size_t cap = VECTOR_CAPACITY;

    if (VECTOR_SIZE + len > cap) {
        while (VECTOR_SIZE + len > cap) {
            cap *= 2;
        }

        char *temp = malloc(cap); if (!temp) return;
        strncpy(temp, VECTOR_ARRAY, VECTOR_SIZE);
        free(VECTOR_ARRAY);
        VECTOR_ARRAY = temp;
        VECTOR_CAPACITY = cap;
    }

    strncpy(&VECTOR_ARRAY[VECTOR_SIZE], string, len);
    VECTOR_SIZE += len;
}

char cvec_pop_last(CVec *vector) {
    if (!vector) return '\0';
    if VECTOR_IS_EMPTY return '\0';

    char c = VECTOR_ARRAY[LAST_INDEX];
    --VECTOR_SIZE;

    if (VECTOR_SIZE < VECTOR_CAPACITY / 3 && VECTOR_CAPACITY >= 2 * CAPACITY_DEFAULT)
        cvec_shrink(vector);

    return c;
}

size_t cvec_size(CVec* vector) {
    if (!vector) return 0;

    return VECTOR_SIZE;
}

int cvec_is_empty(CVec* vector) {
    if (!vector) return 0;

    return VECTOR_IS_EMPTY;
}

void cvec_clear(CVec *vector) {
    if (!vector) return;

    char *temp = malloc(VECTOR_CAPACITY);
    if (!temp) return;
    free(VECTOR_ARRAY);
    VECTOR_ARRAY = temp;

    VECTOR_SIZE = 0;
}

void cvec_print(CVec *vector) {
    if (!vector) return;

    cvec_append(vector, '\0');

    printf("%s", VECTOR_ARRAY);

    cvec_pop_last(vector);
}

void cvec_destroy(CVec *vector) {
    if (!vector) return;
    free(VECTOR_ARRAY);
    free(vector);
}

TokenVec *tokenvec_create_cap(size_t capacity) {
    if (capacity == 0) return NULL;

    TokenVec *vector = (TokenVec *) malloc(sizeof(Token));
    if (!vector) return NULL;

    VECTOR_ARRAY = (Token *) malloc(sizeof(Token) * capacity);
    if (!VECTOR_ARRAY) {
        free(vector);
        return NULL;
    }

    VECTOR_SIZE = 0;
    VECTOR_CAPACITY = capacity;

    return vector;
}

TokenVec *tokenvec_create() {
    return tokenvec_create_cap(CAPACITY_DEFAULT);
}

void tokenvec_expand(TokenVec *vector) {
    if (!vector) return;

    Token *temp_array = VECTOR_ARRAY;
    
    VECTOR_ARRAY = (Token *) malloc(sizeof(Token) * VECTOR_CAPACITY * 2);
    if (!VECTOR_ARRAY) return;

    memcpy(VECTOR_ARRAY, temp_array, VECTOR_CAPACITY * sizeof(Token));

    VECTOR_CAPACITY *= 2;
    free(temp_array);
}

void tokenvec_shrink(TokenVec *vector) {
    if (!vector) return;

    Token *temp_array = VECTOR_ARRAY;

    VECTOR_ARRAY = (Token *) malloc(sizeof(Token) * VECTOR_CAPACITY / 2);
    if (!VECTOR_ARRAY) return;

    VECTOR_CAPACITY /= 2;

    memcpy(VECTOR_ARRAY, temp_array, VECTOR_CAPACITY * sizeof(Token));

    free(temp_array);
}

void tokenvec_append(TokenVec* vector, Token token) {
    if (!vector) return;

    if (VECTOR_SIZE == VECTOR_CAPACITY) tokenvec_expand(vector);

    VECTOR_ARRAY[VECTOR_SIZE].type = token.type;
    VECTOR_ARRAY[VECTOR_SIZE].byte = token.byte;
    VECTOR_ARRAY[VECTOR_SIZE].strref = token.strref;

    ++VECTOR_SIZE;
}

Token tokenvec_get(TokenVec *vector, size_t index) {
    Token null_token = {0, 0, {0, 0}};
    if (!vector || index >= VECTOR_SIZE) return null_token;

    return VECTOR_ARRAY[index];
}

Token tokenvec_peek_last(TokenVec *vector) {
    Token null_token = {0, 0, {0, 0}};
    if (!vector || VECTOR_IS_EMPTY) return null_token;

    return tokenvec_get(vector, LAST_INDEX);
}

size_t tokenvec_size(TokenVec *vector) {
    if (!vector) return 0;

    return VECTOR_SIZE;
}

void tokenvec_clear(TokenVec *vector) {
    if (!vector) return;

    for (size_t i = 0; i < VECTOR_SIZE; ++i) {
        VECTOR_ARRAY[i].byte = 0;
        VECTOR_ARRAY[i].type = 0;
        VECTOR_ARRAY[i].strref.length = 0;
        VECTOR_ARRAY[i].strref.string = NULL;
    }
}

void tokenvec_destroy(TokenVec *vector) {
    if (!vector) return;
    free(VECTOR_ARRAY);
    free(vector);
}

int tokenvec_is_empty(TokenVec *vector) {
    if (!vector) return 0;
    return VECTOR_IS_EMPTY;
}

CmdVec *cmdvec_create_cap(size_t cap) { 
    if (cap <= 0) return NULL;
    
    CmdVec *vector = (CmdVec*) malloc(sizeof(CmdVec));
    if (!vector) return NULL;
    VECTOR_ARRAY = (Cmd*) malloc(sizeof(Cmd) * cap);
    if (!VECTOR_ARRAY) return NULL;
    VECTOR_CAPACITY = cap;
    VECTOR_SIZE = 0;

    return vector;
}

CmdVec *cmdvec_create() {
    return cmdvec_create_cap(CAPACITY_DEFAULT);
}

void cmdvec_expand(CmdVec *vector) {
    if (!vector) return;
    Cmd *temp_array = VECTOR_ARRAY;
    VECTOR_ARRAY = (Cmd*) malloc(sizeof(Cmd) * 2 * VECTOR_CAPACITY);
    if (!VECTOR_ARRAY) return;
    memcpy(VECTOR_ARRAY, temp_array, VECTOR_CAPACITY * sizeof(Cmd));
    VECTOR_CAPACITY *= 2;
    free(temp_array);
}

void cmdvec_shrink(CmdVec *vector) {
    if (!vector) return;
    Cmd *temp_array = VECTOR_ARRAY;
    VECTOR_ARRAY = (Cmd*) malloc(sizeof(Cmd) * VECTOR_CAPACITY / 2);
    if (!VECTOR_ARRAY) return;
    VECTOR_CAPACITY /= 2;
    memcpy(VECTOR_ARRAY, temp_array, VECTOR_CAPACITY * sizeof(Cmd));
    free(temp_array);
}

void cmdvec_append(CmdVec *vector, Cmd node)  {
    if (!vector) return;

    if (VECTOR_SIZE == VECTOR_CAPACITY) cmdvec_expand(vector);

    VECTOR_ARRAY[VECTOR_SIZE++] = node;
}

Cmd cmdvec_get(CmdVec *vector, size_t index) {
    return VECTOR_ARRAY[index];
}

Cmd cmdvec_peek_last(CmdVec *vector) {
    return VECTOR_ARRAY[LAST_INDEX];
}

Cmd cmdvec_pop_last(CmdVec *vector) {
    Cmd ret = VECTOR_ARRAY[LAST_INDEX];
    --VECTOR_SIZE;

    if (VECTOR_SIZE >= 2 * CAPACITY_DEFAULT && VECTOR_SIZE <= VECTOR_CAPACITY / 3)
        cmdvec_shrink(vector);

    return ret;
}

size_t cmdvec_size(CmdVec *vector) {
    if (!vector) return 0;
    return VECTOR_SIZE;
}

int cmdvec_is_empty(CmdVec *vector) {
    if (!vector) return 0;
    return VECTOR_IS_EMPTY;
}

void cmdvec_clear(CmdVec *vector) {
    if (!vector) return;
    free(VECTOR_ARRAY);
    VECTOR_ARRAY = malloc(sizeof(Cmd) * VECTOR_CAPACITY);
    if (!VECTOR_ARRAY) return;
    VECTOR_SIZE = 0;
}

void cmdvec_destroy(CmdVec *vector) {
    if (!vector) return;
    free(VECTOR_ARRAY);
    free(vector);
}
