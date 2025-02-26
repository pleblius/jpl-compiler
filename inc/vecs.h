#ifndef VECS_H
#define VECS_H

#include <stdlib.h>
#include <stdio.h>

#include "token.h"
#include "stringops.h"

typedef struct {
    char *array;
    size_t size;
    size_t capacity;
} CVec;

typedef struct {
    Token *array;
    size_t size;
    size_t capacity;
} TokenVec;

CVec *cvec_create_cap(size_t);
CVec *cvec_create();
void cvec_expand(CVec*);
void cvec_shrink(CVec*);
void cvec_append(CVec*, char);
void cvec_append_ref(CVec*, StringRef);
void cvec_append_array(CVec*, const char*, size_t);
void cvec_append_ref_line(CVec*, StringRef);
void cvec_append_array_line(CVec*, const char*, size_t);
size_t cvec_size(CVec*);
void cvec_clear(CVec*);
void cvec_print(CVec*);
void cvec_destroy(CVec*);
int cvec_is_empty(CVec*);

TokenVec *tokenvec_create_cap(size_t);
TokenVec *tokenvec_create();
void tokenvec_expand(TokenVec*);
void tokenvec_shrink(TokenVec*);
void tokenvec_append(TokenVec*, Token);
Token *tokenvec_get(TokenVec*, size_t);
Token *tokenvec_peek_last(TokenVec*);
size_t tokenvec_size(TokenVec*);
void tokenvec_clear(TokenVec*);
void tokenvec_destroy(TokenVec*);
int tokenvec_is_empty(TokenVec*);

#endif // VECS_H
