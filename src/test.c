#include "dict.h"
#include <stdio.h>

int main() {
    TypeDict *dict = dict_create_small();

    dict_add_array(dict, "a", 1, (Type) {1, 0, {0}, 0});
    dict_add_array(dict, "b", 1, (Type) {2, 0 ,{0}, 0});
    dict_add_array(dict, "c", 1, (Type) {1, 0, {0}, 0});
    dict_add_array(dict, "d", 1, (Type) {2, 0, {0}, 0});
    dict_add_array(dict, "e", 1, (Type) {1, 0, {0}, 0});
    dict_add_array(dict, "f", 1, (Type) {2, 0, {0}, 0});
    dict_add_array(dict, "g", 1, (Type) {1, 0, {0}, 0});
    dict_add_array(dict, "h", 1, (Type) {2, 0, {0}, 0});
    dict_add_array(dict, "i", 1, (Type) {1, 0, {0}, 0});
    dict_add_array(dict, "j", 1, (Type) {2, 0, {0}, 0});
    dict_add_array(dict, "k", 1, (Type) {1, 0, {0}, 0});
    dict_add_array(dict, "l", 1, (Type) {2, 0, {0}, 0});

    print_dict(dict);

    dict_remove_array(dict, "f", 1);
    dict_remove_array(dict, "g", 1);
    dict_remove_array(dict, "e", 1);

    print_dict(dict);

    dict_add_array(dict, "a", 1, (Type) {1, 2, {0}, 0});
    dict_add_array(dict, "f", 1, (Type) {2, 5, {0}, 0});
    dict_add_array(dict, "l", 1, (Type) {1, 10, {0}, 0});

    print_dict(dict);

    
    dict_add_array(dict, "ab", 2, (Type) {1, 2, {0}, 0});
    dict_add_array(dict, "fa", 2, (Type) {2, 5, {0}, 0});
    dict_add_array(dict, "le", 2, (Type) {1, 10, {0}, 0});
    
    dict_add_array(dict, "ad", 2, (Type) {1, 2, {0}, 0});
    dict_add_array(dict, "fs", 2, (Type) {2, 5, {0}, 0});
    dict_add_array(dict, "lf", 2, (Type) {1, 10, {0}, 0});
    
    dict_add_array(dict, "ac", 2, (Type) {1, 2, {0}, 0});
    dict_add_array(dict, "fx", 2, (Type) {2, 5, {0}, 0});
    dict_add_array(dict, "lx", 2, (Type) {1, 10, {0}, 0});
    
    dict_add_array(dict, "ae", 2, (Type) {1, 2, {0}, 0});
    dict_add_array(dict, "fe", 2, (Type) {2, 5, {0}, 0});
    dict_add_array(dict, "lw", 2, (Type) {1, 10, {0}, 0});

    print_dict(dict);
    Type type;
    if (dict_try_array(dict, "fs", 2, &type))
        printf("%d\n", type.type);
    if (dict_try_array(dict, "l", 1, &type))
        printf("%ld\n", type.loc);
    if (dict_try_array(dict, "x", 2, &type))
        printf("%d\n", type.type);
    else (printf("Not found\n"));
    if (dict_try_array(dict, "g", 2, &type))
        printf("%d", type.type);
    else (printf("Not found\n"));
}
