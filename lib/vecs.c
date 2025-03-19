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

void cvec_append(CVec* vector, char c) {
    if (!vector) return;

    if (VECTOR_SIZE == VECTOR_CAPACITY) cvec_expand(vector);

    VECTOR_ARRAY[VECTOR_SIZE++] = c;
}

void cvec_append_ref(CVec *vector, StringRef string) {
    cvec_append_array(vector, string.string, string.length);
}

void cvec_append_array(CVec *vector, const char *string, size_t len) {
    if (!vector || !string) return;

    while (VECTOR_SIZE + len > VECTOR_CAPACITY) {
        cvec_expand(vector);
    }
    
    memcpy(&VECTOR_ARRAY[VECTOR_SIZE], string, len);
    VECTOR_SIZE += len;
}

void cvec_append_ref_line(CVec *vector, StringRef string) {
    cvec_append_array_line(vector, string.string, string.length);
}

void cvec_append_array_line(CVec *vector, const char *string, size_t len) {
    cvec_append_array(vector, string, len);
    cvec_append(vector, '\n');
}

char cvec_pop_last(CVec *vector) {
    if (!vector) return '\0';
    if VECTOR_IS_EMPTY return '\0';

    char c = VECTOR_ARRAY[LAST_INDEX];
    --VECTOR_SIZE;

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

    fputs(vector->array, stdout);
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

void tokenvec_append(TokenVec* vector, Token token) {
    if (!vector) return;

    if (VECTOR_SIZE == VECTOR_CAPACITY) tokenvec_expand(vector);

    VECTOR_ARRAY[VECTOR_SIZE].type = token.type;
    VECTOR_ARRAY[VECTOR_SIZE].loc = token.loc;
    VECTOR_ARRAY[VECTOR_SIZE].strref = token.strref;

    ++VECTOR_SIZE;
}

Token *tokenvec_get(TokenVec *vector, size_t index) {
    if (!vector || index >= VECTOR_SIZE) return NULL;

    return VECTOR_ARRAY + index;
}

Token *tokenvec_peek_last(TokenVec *vector) {
    if (!vector || VECTOR_IS_EMPTY) return NULL;

    return tokenvec_get(vector, LAST_INDEX);
}

size_t tokenvec_size(TokenVec *vector) {
    if (!vector) return 0;

    return VECTOR_SIZE;
}

void tokenvec_clear(TokenVec *vector) {
    if (!vector) return;

    for (size_t i = 0; i < VECTOR_SIZE; ++i) {
        VECTOR_ARRAY[i].loc = 0;
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

NodeVec *nodevec_create_cap(size_t capacity) {
    if (!capacity) return NULL;
    NodeVec *vector = malloc(sizeof(NodeVec));
    if (!vector) return NULL;

    VECTOR_ARRAY = malloc(sizeof(AstNode) * capacity);
    if (!VECTOR_ARRAY) {
        free(vector);
        return NULL;
    }

    VECTOR_CAPACITY = capacity;
    VECTOR_SIZE = 0;
    return vector;
}
NodeVec *nodevec_create() {
    return nodevec_create_cap(CAPACITY_DEFAULT);
}

void nodevec_expand(NodeVec *vector) {
    if (!vector) return;

    AstNode *temp = VECTOR_ARRAY;
    VECTOR_ARRAY = malloc(sizeof(AstNode) * 2 * VECTOR_CAPACITY);
    if (!VECTOR_ARRAY) {
        free(temp);
        return;
    }

    memcpy(VECTOR_ARRAY, temp, VECTOR_CAPACITY * sizeof(AstNode));

    VECTOR_CAPACITY *= 2;
    free(temp);
}

size_t nodevec_append(NodeVec *vector, AstNode node) {
    if (!vector) return 0;
    if (VECTOR_SIZE == VECTOR_CAPACITY)
        nodevec_expand(vector);
        
    VECTOR_ARRAY[VECTOR_SIZE++] = node;
    return VECTOR_SIZE - 1;
}

AstNode *nodevec_get(NodeVec *vector, size_t index) {
    if (!vector || index >= VECTOR_SIZE) return NULL;

    return &VECTOR_ARRAY[index];
}
int nodevec_remove(NodeVec *vector, size_t index, AstNode *output) {
    if (!vector || index >= VECTOR_SIZE) return 0;

    *output = VECTOR_ARRAY[index];
    --VECTOR_SIZE;
    memmove(VECTOR_ARRAY + index, VECTOR_ARRAY+index+1, VECTOR_SIZE - index);

    return 1;
}
AstNode *nodevec_peek_last(NodeVec *vector) {
    if (!vector || VECTOR_IS_EMPTY) return NULL;

    return &VECTOR_ARRAY[VECTOR_SIZE-1];
}

AstNode nodevec_pop_last(NodeVec *vector) {
    if (!vector || VECTOR_IS_EMPTY) return get_empty_node();

    return VECTOR_ARRAY[--VECTOR_SIZE];
}

void nodevec_destroy(NodeVec *vector) {
    free(VECTOR_ARRAY);
    free(vector);
}

int nodevec_is_empty(NodeVec *vector) {
    if (!vector || VECTOR_IS_EMPTY) return 0;
    return 1;
}
