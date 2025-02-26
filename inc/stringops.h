#ifndef STRINGOPS_H
#define STRINGOPS_H

#include <stdlib.h>
#include <string.h>

#define MAXIMUM_BUFFER 1024
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
void string_slice(char*, char*, size_t, size_t);
char *dup_string(char*);

String *string_from_array(char*);
String *string_from_ref(StringRef);
StringRef ref_from_array(char* p_c, size_t, size_t);
StringRef ref_from_string(String*, size_t, size_t);
char *array_from_ref(StringRef);

void free_string(String*);
void print_string(String*);
void print_string_ref(StringRef);

int string_ref_cmp(StringRef, StringRef);
int ref_array_cmp(StringRef, char*);

#endif // STRINGOPS_H
