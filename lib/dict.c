#include <stdio.h>
#include "dict.h"

#define SEED 0x9747b28c
#define MAX_LOAD_FACTOR 0.75

Dict *dict_create_big() {
    Dict* dict = (Dict*) malloc(sizeof(Dict));
    if (!dict) return NULL;

    dict->array = (Node*) calloc(BIG_SIZE, sizeof(Node));
    if (!dict->array) {
        free(dict);
        return NULL;
    }

    dict->capacity = BIG_SIZE;
    dict->size = 0;
    return dict;
}
Dict *dict_create_small() {
    Dict* dict = (Dict*) malloc(sizeof(Dict));
    if (!dict) return NULL;

    dict->array = (Node*) calloc(SMALL_SIZE, sizeof(Node));
    if (!dict->array) {
        free(dict);
        return NULL;
    }

    dict->capacity = SMALL_SIZE;
    dict->size = 0;
    return dict;
}
Dict *dict_create_cap(size_t capacity) {
    Dict* dict = (Dict*) malloc(sizeof(Dict));
    if (!dict) return NULL;

    dict->array = (Node*) calloc(capacity, sizeof(Node));
    if (!dict->array) {
        free(dict);
        return NULL;
    }

    dict->capacity = BIG_SIZE;
    dict->size = 0;
    return dict;
}
void dict_free(Dict *dict) {
    if (!dict) return;

    free(dict->array);
    free(dict);
}

void dict_expand(Dict *dict) {
    if (!dict) return;

    size_t old_cap = dict->capacity;
    size_t old_size = dict->size;
    Node *old_array = dict->array;

    size_t new_cap = ((old_cap + 1) << 2) - 1;
    size_t new_size = 0;
    Node *new_array = (Node*) calloc(new_cap, sizeof(Node));
    if (!new_array) return;

    *dict = (Dict) {new_size, new_cap, new_array};

    Node *node;
    size_t count = 0;
    for (size_t i = 0; i < old_cap; ++i) {
        node = &old_array[i];
        if (!node->_node || node->_tombstone) continue;

        dict_add_ref(dict, node->key, node->value);
        ++count;
        if (count == old_size) break;
    }

    free(old_array);
}

int dict_try_string(Dict *dict, String key, void **value) {
    if (!dict || !key.string) return 0;

    return dict_try_array(dict, key.string, key.length, value);
}
int dict_try_ref(Dict *dict, StringRef key, void **value) {
    if (!dict || !key.string) return 0;

    return dict_try_array(dict, key.string, key.length, value);
}
int dict_try_array(Dict *dict, char *key, size_t len, void **value) {
    if (!dict || !key || !len) return 0;

    uint32_t hash = hash_string(key, len, SEED);

    size_t base_index = hash % dict->capacity;
    size_t index;
    Node node;
    for (size_t i = 0; i < dict->capacity; ++i) {
        index = (base_index + i*i) % dict->capacity;
        node = dict->array[index];

        // No matching node
        if (!node._node) break;

        // Check if node matches key
        if (node.key.length != len) continue;
        if (strncmp(key, node.key.string, len)) continue;

        // If node is not tombstoned then node matches
        if (node._tombstone) break;

        if (value != NULL)
            *value = node.value;
        return 1;
    }

    return 0;
}

// Adds value to dictionary if key is not associated with another value. Returns 1 on success, 0 on failure.
// On failure, existing value is returned in output.
int dict_add_string_if_empty(Dict *dict, String key, void *value, void **output) {
    return dict_add_ref_if_empty(dict, (StringRef) {key.length, key.string}, value, output);
}

// Adds value to dictionary if key is not associated with another value. Returns 1 on success, 0 on failure.
// On failure, existing value is returned in output.
int dict_add_ref_if_empty(Dict *dict, StringRef key, void *value, void **output) {
    if (!dict || !key.string || !key.length) return 0;

    uint32_t hash = hash_string(key.string, key.length, SEED);
    size_t base_index = hash % dict->capacity;
    size_t index = 0;
    size_t tomb_index = 0;
    size_t tomb_flag = 0;
    Node *node;
    for (size_t i = 0; i < dict->capacity; ++i) {
        index = (base_index + i*i) % dict->capacity;
        node = &dict->array[index];

        if (tomb_flag) index = tomb_index;

        // No matches
        if (!node->_node) {
            ++dict->size;
            dict->array[index] = (Node) { 1, 0, key, value };
            break;
        }

        // Store first available tombstone node
        if (node->_tombstone && !tomb_flag) {
            tomb_flag = 1;
            tomb_index = index;
        }

        // Check if keys match
        if (node->key.length != key.length) continue;
        if (strncmp(node->key.string, key.string, key.length)) continue;

        // Keys must match - If not a tombstone then is an active node
        if (!node->_tombstone) {
            *output = node->value;
            return 0;
        }
        ++dict->size;
        dict->array[index] = (Node) { 1, 0, key, value };
        break;
    }

    double alpha = ((double) dict->size) / ((double) dict->capacity);
    if (alpha > MAX_LOAD_FACTOR) dict_expand(dict);
    return 1;
}

// Adds value to dictionary if key is not associated with another value. Returns 1 on success, 0 on failure.
// On failure, existing value is returned in output.
int dict_add_array_if_empty(Dict *dict, char *key, size_t len, void *value, void **output) {
    return dict_add_ref_if_empty(dict, (StringRef) {len, key}, value, output);
}

void *dict_add_string(Dict *dict, String key, void *value) {
    return dict_add_ref(dict, (StringRef) {key.length, key.string}, value);
}

void *dict_add_ref(Dict *dict, StringRef key, void *value) {
    if (!dict || !key.string || !key.length) return NULL;

    uint32_t hash = hash_string(key.string, key.length, SEED);
    size_t base_index = hash % dict->capacity;
    size_t tomb_index = 0;
    size_t tomb_flag = 0;
    size_t index;
    Node *node;
    void *ret_value = NULL;

    for (size_t i = 0; i < dict->capacity; ++i) {
        index = (base_index + i*i) % dict->capacity;
        node = &dict->array[index];

        if (tomb_flag) index = tomb_index;

        // No match
        if (!node->_node) {
            ++dict->size;
            dict->array[index] = (Node) { 1, 0, key, value };
            ret_value = NULL;
            break;
        }

        if (node->_tombstone && !tomb_flag) {
            tomb_flag = 1;
            tomb_index = index;
        }

        if (node->key.length != key.length) continue;
        if (strncmp(key.string, node->key.string, key.length)) continue;

        if (node->_tombstone) 
            ++dict->size;
        else
            ret_value = node->value;

        dict->array[index] = (Node) {1, 0, key, value};
        break;
    }

    double alpha = ((double) dict->size) / ((double) dict->capacity);
    if (alpha > MAX_LOAD_FACTOR) dict_expand(dict);

    return ret_value;
}

void *dict_add_array(Dict *dict, char *key, size_t len, void *value) {
    return dict_add_ref(dict, (StringRef) {len, key}, value);
}

void *dict_remove_string(Dict *dict, String key) {
    return dict_remove_ref(dict, (StringRef) {key.length, key.string});
}

void *dict_remove_ref(Dict *dict, StringRef key) {
    if (!dict || !key.string || !key.length) return NULL;

    uint32_t hash = hash_string(key.string, key.length, SEED);
    size_t base_index = hash % dict->capacity;
    size_t index;
    Node *node;

    for (size_t i = 0; i < dict->capacity; ++i) {
        index = (base_index + i*i) % dict->capacity;
        node = &dict->array[index];
       
        if (!node->_node) break;
        if (node->key.length != key.length) continue;
        if (strncmp(node->key.string, key.string, key.length)) continue;
        if (node->_tombstone) break;

        node->_tombstone = 1;
        --dict->size;
        return node->value;
    }

    return NULL;
}
void *dict_remove_array(Dict *dict, char *key, size_t len) {
    return dict_remove_ref(dict, (StringRef) {len, key});
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
