#ifndef STRINGOPS_H
#define STRINGOPS_H

#include <stdlib.h>

#define RED "\033[0;31m"
#define RED_BOLD "\033[1;31m"
#define RESET "\x1B[0m"

typedef struct {
    size_t length;
    char *string;
} StringRef;

typedef struct {
    size_t length;
    char *string;
} String;

char *string_combine(size_t, ...);
int string_slice(char*, char*, size_t, size_t);
char *dup_string(char*);

String *string_from_array(char*);
String *string_from_ref(StringRef*);
StringRef *new_stringref(char* p_c, size_t, size_t);
StringRef *ref_from_string(String*, size_t, size_t);

void free_string(String*);
void print_string(String*);
void print_string_ref(StringRef*);
#endif // STRINGOPS_H
