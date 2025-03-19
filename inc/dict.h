#ifndef DICT_H
#define DICT_H

#include <stdint.h>
#include <stdlib.h>

#include "stringops.h"

#define BIG_SIZE 0xFFFF
#define SMALL_SIZE 0xFF

typedef struct {
    uint16_t _node;
    uint16_t _tombstone;
    StringRef key;
    void *value;
} Node;

typedef struct {
    uint64_t size;
    uint64_t capacity;
    Node *array;
} Dict;

Dict *dict_create_big();
Dict *dict_create_small();
void dict_free(Dict*);
void dict_expand(Dict*);

int dict_try_string(Dict *, String, void **);
int dict_try_ref(Dict*, StringRef, void **);
int dict_try_array(Dict*, char *, size_t, void **);

int dict_add_ref_if_empty(Dict *, StringRef, void*, void**);
int dict_add_string_if_empty(Dict *, String, void*, void**);
int dict_add_array_if_empty(Dict *, char *, size_t, void*, void**);

void *dict_add_string(Dict*, String, void*);
void *dict_add_ref(Dict*, StringRef, void*);
void *dict_add_array(Dict*, char*, size_t, void*);

void *dict_remove_string(Dict*, String);
void *dict_remove_ref(Dict*, StringRef);
void *dict_remove_array(Dict*, char*, size_t);

uint32_t hash_string(char *, size_t, uint32_t);

#endif // DICT_H
