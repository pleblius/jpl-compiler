#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

typedef struct {
    void **array;
    size_t size;
    size_t capacity;
} Vector;

Vector *vector_create_cap(size_t);
Vector *vector_create();
void vector_expand(Vector*);
void vector_shrink(Vector*);
void vector_insert(Vector*, size_t, void*);
void vector_append(Vector*, void*);
void *vector_set(Vector*, size_t, void*);
void *vector_get(Vector*, size_t);
void *vector_pop_last(Vector*);
void *vector_peek_last(Vector*);
size_t vector_size(Vector*);
void vector_clear(Vector*);
void vector_print(Vector*);
void vector_destroy(Vector*);
void vector_destroy_deep(Vector*);
int vector_is_empty(Vector*);

#endif // VECTOR_H
