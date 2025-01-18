#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    void **array;
    int size;
    int capacity;
} Vector;

Vector *vector_create_cap(int);
Vector *vector_create();
void vector_expand(Vector*);
void vector_shrink(Vector*);
void vector_insert(Vector*, int, void*);
void vector_append(Vector*, void*);
void *vector_set(Vector*, int, void*);
void *vector_get(Vector*, int);
void *vector_pop_last(Vector*);
void *vector_peek_last(Vector*);
int vector_size(Vector*);
void vector_clear(Vector*);
void vector_print(Vector*);
void vector_destroy(Vector*);
int vector_is_empty(Vector*);

#endif // VECTOR_H
