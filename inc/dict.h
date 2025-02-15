#include <stdint.h>
#include <stdlib.h>

#include "vector.h"
#include "prod.h"
#include "stringops.h"

#define BIG_SIZE 1048575
#define SMALL_SIZE 127

typedef struct {
    uint16_t _node;
    uint16_t _tombstone;
    String key;
    Type value;
} TypeNode;

typedef struct {
    uint64_t size;
    uint64_t capacity;
    TypeNode *array;
} TypeDict;

TypeDict *dict_create_big();
TypeDict *dict_create_small();
void dict_free(TypeDict*);
void dict_expand(TypeDict*);

int dict_try_string(TypeDict *, String, Type *);
int dict_try_ref(TypeDict*, StringRef, Type *);
int dict_try_array(TypeDict*, char *, size_t, Type *);

Type dict_add_string(TypeDict*, String, Type);
Type dict_add_ref(TypeDict*, StringRef, Type);
Type dict_add_array(TypeDict*, char*, size_t, Type);

Type dict_remove_string(TypeDict*, String);
Type dict_remove_ref(TypeDict*, StringRef);
Type dict_remove_array(TypeDict*, char*, size_t);

uint32_t hash_string(char *, size_t, uint32_t);

void print_dict(TypeDict*);
