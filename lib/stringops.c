#include <string.h>
#include <stdarg.h>
#include <stdio.h>

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

int string_slice(char *dst, char *src, size_t start, size_t end) {
    size_t len = end - start;

    for (size_t i = 0; i < len; ++i) {
        dst[i] = src[start+i];
    }
    dst[end] = '\0';

    return EXIT_SUCCESS;
}

String *string_from_array(char *p_c) {
    String *string = (String *) malloc(sizeof(String));
    if (!string) return NULL;

    string -> length = strlen(p_c);

    string->string = (char *) malloc(string->length + 1);
    strcpy(string->string, p_c);

    return string;
}
String *string_from_ref (StringRef *strref) {
    String *string = (String *) malloc(sizeof(String));
    if (!string) return NULL;
    string->length = strref->length;

    string->string = (char *) malloc(string->length);
    if (!string->string) return NULL;

    size_t i;
    for (i = 0; i < string->length; ++i) {
        string->string[i] = strref->string[i];
    }
    string->string[i] = '\0';

    return string;
}
StringRef *new_stringref(char *p_c, size_t start, size_t end) {
    StringRef *strref = (StringRef *) malloc(sizeof(StringRef));
    if (!strref) return NULL;

    strref -> length = end - start;
    strref -> string = p_c + start;

    return strref;
}
StringRef *ref_from_string(String *string, size_t start, size_t end) {
    StringRef *strref = (StringRef *) malloc(sizeof(StringRef));
    if (!strref) return NULL;
  
    strref->length = end - start;
    strref->string = string->string + start;

    return strref;
}

void free_string(String *string) {
    free(string->string);
}

void print_string(String *string) {
    printf("%s", string->string);
}
void print_string_ref(StringRef *strref) {
    char string[strref->length + 1];

    size_t i;
    for (i = 0; i < strref->length; ++i) {
        string[i] = strref->string[i];
    }
    string[i] = '\0';

    printf("%s", string);
}
