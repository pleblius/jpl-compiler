#include "vecs.h"

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

    for (size_t i = 0; i < VECTOR_SIZE; ++i) {
        VECTOR_ARRAY[i] = '\0';
    }
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
