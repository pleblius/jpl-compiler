#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include "stringops.h"

char *string_combine(size_t num, ...) {
    va_list args;
    va_start(args, num);
    size_t len = 1; // Null Term
    size_t lens[num];
    char *strings[num];

    for (unsigned int i = 0; i < num; ++i) {
        strings[i] = va_arg(args, char*);
        lens[i] = strlen(strings[i]);
        len += lens[i];
    }
    va_end(args);

    char *output = malloc(len); if (!output) return NULL;

    size_t index = 0;
    for (unsigned int i = 0; i < num; ++i) {
        strcpy(output + index, strings[i]);
        index += lens[i];
    }

    return output;
}

char *string_dup(char* src) {
    char *dst = malloc(strlen(src) + 1);
    if (!dst) return NULL;
    strcpy(dst, src);
    return dst;
}

// Slices the src string from [start, end) into dst
void string_slice(char *dst, char *src, size_t start, size_t end) {
    strncpy(dst, src + start, end-start);
}

// Generates a new string from the character array.
String *string_from_array(char *p_c) {
    String *string = (String *) malloc(sizeof(String));
    if (!string) return NULL;

    string -> length = strlen(p_c);

    string->string = (char *) malloc(string->length + 1);
    strncpy(string->string, p_c, string->length+1);

    return string;
}
// Generates a new string as a copy of strref.
String *string_from_ref (StringRef strref) {
    String *string = (String *) malloc(sizeof(String));
    if (!string) return NULL;

    string->length = strref.length;
    string->string = (char *) malloc(string->length+1);
    if (!string->string) return NULL;
    strncpy(string->string, strref.string, strref.length);
    string->string[string->length] = '\0';
    return string;
}
// Generates a reference to [start, end) of the given array.
StringRef ref_from_array(char *p_c, size_t start, size_t end) {
    StringRef strref;

    strref.length = end - start;
    strref.string = p_c + start;

    return strref;
}
// Generates a reference to [start, end) of the given String.
StringRef ref_from_string(String *string, size_t start, size_t end) {
    StringRef strref;

    strref.length = end - start;
    strref.string = string->string + start;

    return strref;
}
char *array_from_ref(StringRef strref) {
    if (!strref.string) return NULL;
    char *string = malloc(strref.length + 1);

    if (strncpy(string, strref.string, strref.length) == NULL) return NULL;
    string[strref.length] = '\0';
    return string;
}

void free_string(String *string) {
    free(string->string);
}

void print_string(String *string) {
    printf("%s", string->string);
}
void print_string_ref(StringRef strref) {
    size_t length = strref.length;
    char string[length + 1];

    strncpy(string, strref.string, length);
    string[length] = '\0';

    printf("%s", string);
}

int string_ref_cmp(StringRef string1, StringRef string2) {
    return (string1.length != string2.length || strncmp(string1.string, string2.string, string1.length));
}

int ref_array_cmp(StringRef string1, char* string2) {
    if (!string2) return 0;

    size_t len = strlen(string2);
    if (len != string1.length) return 0;

    return strncmp(string1.string, string2, len);
}
