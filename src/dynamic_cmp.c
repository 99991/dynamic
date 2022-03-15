#include "dynamic.h"

int cmp(var object1, var object2){
    assert(object1 && object2 && object1->type_info == object2->type_info);

    return object1->type_info->cmp(object1, object2);
}

bool eq(var object1, var object2){
    return 0 == cmp(object1, object2);
}

int cmp_natural(const char *a, const char *b){
    while (*a != '\0' && *b != '\0'){
        if (!is_digit(*a) || !is_digit(*b)){
            if (*a < *b) return -1;
            if (*a > *b) return +1;
            a++;
            b++;
            continue;
        }

        const char *a_start = a;
        const char *b_start = b;

        while (*a && *a == '0') a++;
        while (*b && *b == '0') b++;

        const char *a_start_nonzero = a;
        const char *b_start_nonzero = b;

        while (*a && is_digit(*a)) a++;
        while (*b && is_digit(*b)) b++;

        size_t na = (size_t)(a - a_start_nonzero);
        size_t nb = (size_t)(b - b_start_nonzero);

        if (na < nb) return -1;
        if (na > nb) return +1;

        int c = memcmp(a_start_nonzero, b_start_nonzero, na);

        if (c != 0) return c;

        na = (size_t)(a - a_start);
        nb = (size_t)(b - b_start);

        if (na < nb) return -1;
        if (na > nb) return +1;
    }

    if (*a == '\0' && *b != '\0') return -1;
    if (*a != '\0' && *b == '\0') return +1;

    return 0;
}

int obj_str_cmp(var object1, var object2){
    return cmp_natural(obj_cstr(object1), obj_cstr(object2));
}

int obj_num_cmp(var object1, var object2){
    if (object1->value < object2->value) return -1;
    if (object1->value > object2->value) return +1;
    return 0;
}

int obj_dbl_cmp(var object1, var object2){
    if (object1->dvalue < object2->dvalue) return -1;
    if (object1->dvalue > object2->dvalue) return +1;
    return 0;
}

int obj_bool_cmp(var object1, var object2){
    if (object1->bvalue < object2->bvalue) return -1;
    if (object1->bvalue > object2->bvalue) return +1;
    return 0;
}

int obj_arr_cmp(var object1, var object2){
    size_t n = (object1->length < object2->length ? object1 : object2)->length;

    for (size_t i = 0; i < n; i++){
        int c = cmp(arr_at(object1, i), arr_at(object2, i));

        if (c != 0) return c;
    }

    if (object1->length < object2->length) return -1;
    if (object1->length > object2->length) return +1;

    return 0;
}

int obj_map_cmp(var object1, var object2){
    MapItem *a = object1->first;
    MapItem *b = object2->first;

    while (a && b){
        int c = cmp(a->key, b->key);

        if (c != 0) return c;

        c = cmp(a->value, b->value);

        if (c != 0) return c;

        a = a->next;
        b = b->next;
    }

    if (object1->length < object2->length) return -1;
    if (object1->length > object2->length) return +1;

    return 0;
}

