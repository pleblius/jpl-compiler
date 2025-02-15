#include <stdio.h>
#include "dict.h"

#define SEED 0x9747b28c
#define MAX_LOAD_FACTOR 0.75

TypeDict *dict_create_big() {
    TypeDict* dict = (TypeDict*) malloc(sizeof(TypeDict));
    if (!dict) return NULL;

    dict->array = (TypeNode*) calloc(BIG_SIZE, sizeof(TypeNode));
    if (!dict->array) {
        free(dict);
        return NULL;
    }

    dict->capacity = BIG_SIZE;
    dict->size = 0;
    return dict;
}
TypeDict *dict_create_small() {
    TypeDict* dict = (TypeDict*) malloc(sizeof(TypeDict));
    if (!dict) return NULL;

    dict->array = (TypeNode*) calloc(SMALL_SIZE, sizeof(TypeNode));
    if (!dict->array) {
        free(dict);
        return NULL;
    }

    dict->capacity = SMALL_SIZE;
    dict->size = 0;
    return dict;
}
TypeDict *dict_create_cap(size_t capacity) {
    TypeDict* dict = (TypeDict*) malloc(sizeof(TypeDict));
    if (!dict) return NULL;

    dict->array = (TypeNode*) calloc(capacity, sizeof(TypeNode));
    if (!dict->array) {
        free(dict);
        return NULL;
    }

    dict->capacity = BIG_SIZE;
    dict->size = 0;
    return dict;
}
void dict_free(TypeDict *dict) {
    if (!dict) return;

    free(dict->array);
    free(dict);
}

void dict_expand(TypeDict *dict) {
    if (!dict) return;

    size_t old_cap = dict->capacity;
    size_t old_size = dict->size;
    TypeNode *old_array = dict->array;

    size_t new_cap = ((old_cap + 1) << 2) - 1;
    size_t new_size = 0;
    TypeNode *new_array = (TypeNode*) calloc(new_cap, sizeof(TypeNode));
    if (!new_array) return;

    *dict = (TypeDict) {new_size, new_cap, new_array};

    TypeNode node;
    size_t count = 0;
    for (size_t i = 0; i < old_cap; ++i) {
        node = old_array[i];
        if (!node._node || node._tombstone) continue;

        dict_add_string(dict, node.key, node.value);
        ++count;
        if (count == old_size) break;
    }

    free(old_array);
}

int dict_try_string(TypeDict *dict, String key, Type *value) {
    if (!dict || !key.string) return 0;

    return dict_try_array(dict, key.string, key.length, value);
}
int dict_try_ref(TypeDict *dict, StringRef key, Type *value) {
    if (!dict || !key.string) return 0;

    return dict_try_array(dict, key.string, key.length, value);
}
int dict_try_array(TypeDict *dict, char *key, size_t len, Type *value) {
    if (!dict || !key || !len) return 0;

    uint32_t hash = hash_string(key, len, SEED);

    size_t base_index = hash % dict->capacity;
    size_t index;
    TypeNode node;
    for (size_t i = 0; i < dict->capacity; ++i) {
        index = (base_index + i*i) % dict->capacity;
        node = dict->array[index];
        if (!node._node) break;
        if (node._tombstone) continue;
        if (node.key.length != len) continue;
        if (!strncmp(key, node.key.string, len)) {
            if (value != NULL) *value = node.value;
            return 1;
        }
    }

    return 0;
}

Type dict_add_string(TypeDict *dict, String key, Type value) {
    Type stored = (Type) {0, 0, {{0}}, 0};
    if (!dict || !key.string) return stored;

    return dict_add_array(dict, key.string, key.length, value);
}

Type dict_add_ref(TypeDict *dict, StringRef key, Type value) {
    Type stored = (Type) {0, 0, {{0}}, 0};
    if (!dict || !key.string) return stored;

    return dict_add_array(dict, key.string, key.length, value);
}

Type dict_add_array(TypeDict *dict, char *key, size_t len, Type value) {
    Type stored = (Type) {0, 0, {{0}}, 0};
    if (!dict || !key || len == 0) return stored;

    uint32_t hash = hash_string(key, len, SEED);
    size_t base_index = hash % dict->capacity;
    size_t index;
    TypeNode node;

    for (size_t i = 0; i < dict->capacity; ++i) {
        index = (base_index + i*i) % dict->capacity;
        node = dict->array[index];

        if (!node._node || node._tombstone) {
            ++dict->size;
            dict->array[index] = (TypeNode) {1, 0, (String){len, key}, value};
            break;
        }
        else if (node._node) {
            if (node.key.length != len) continue;
            else if (!strncmp(key, node.key.string, len)) {;
                dict->array[index] = (TypeNode) {1, 0, (String){len, key}, value};
                stored = node.value;
                break;
            }
            else continue;
        }
    }

    double alpha = ((double) dict->size) / ((double) dict->capacity);
    if (alpha > MAX_LOAD_FACTOR) dict_expand(dict);

    return stored;
}

Type dict_remove_string(TypeDict *dict, String key) {
    Type stored = (Type) {0, 9, {{0}}, 0};
    if (!dict || !key.string) return stored;

    return dict_remove_array(dict, key.string, key.length);
}
Type dict_remove_ref(TypeDict *dict, StringRef key) {
    Type stored = (Type) {0, 0, {{0}}, 0};
    if (!dict || !key.string) return stored;

    return dict_remove_array(dict, key.string, key.length);
}
Type dict_remove_array(TypeDict *dict, char *key, size_t len) {
    Type stored = (Type) {0, 0, {{0}}, 0};
    if (!dict || !key || len == 0) return stored;

    uint32_t hash = hash_string(key, len, SEED);

    size_t base_index = hash % dict->capacity;
    size_t index;
    TypeNode node;
    for (size_t i = 0; i < dict->capacity; ++i) {
        index = (base_index + i*i) % dict->capacity;
        node = dict->array[index];
        if (!node._node) break;
        if (node._tombstone) continue;
        if (node.key.length != len) continue;
        if (!strncmp(key, node.key.string, len)) {
            stored = node.value;
            dict->array[index]._tombstone = 1;
            --dict->size;
            break;
        }
    }

    return stored;
}

static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}
uint32_t hash_string(char* key, size_t len, uint32_t seed)
{
	uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

void print_dict(TypeDict *dict) {
    printf("Dict:\tSize=%ld\tCapacity=%ld\nIndex\tKey\tValue\tTombstone\n",dict->size, dict->capacity);
    for (size_t i = 0; i < dict->capacity; ++i) {
        if (dict->array[i]._node == 0) continue;
        printf("%ld\t%s\t%d\t%d\n", i, dict->array[i].key.string, dict->array[i].value.type, dict->array[i]._tombstone);
    }
}
