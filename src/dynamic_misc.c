#include "dynamic.h"

bool is_alpha(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_digit(char c){
    return '0' <= c && c <= '9';
}

bool is_space(char c){
    switch (c){
        case ' ': return true;
        case '\f': return true;
        case '\n': return true;
        case '\r': return true;
        case '\t': return true;
        case '\v': return true;
        default: return false;
    }
}

var find_consecutive(var string, bool (*is_wanted_char)(char c)){
    var matches = arr0();
    size_t length = string->length;

    for (size_t i = 0; i < length;){
        begin_scope();

        while (i < length && !is_wanted_char(string->string[i])) i++;

        size_t start = i;
        while (i < length && is_wanted_char(string->string[i])) i++;

        if (start != i){
            push(matches, substr(string, start, i));
        }
    }

    return matches;
}

var slice(var values, size_t i, size_t j){
    assert(obj_is_arr(values));

    if (i > values->length) i = values->length;
    if (j > values->length) j = values->length;

    if (i >= j) return obj_arr_new(NULL, 0);

    return obj_arr_new(values->objects + values->start + i, j - i);
}

void extend(var arr, var more){
    for (size_t i = 0; i < more->length; i++){
        push(arr, arr_at(more, i));
    }
}

var obj_identity(var object){
    return object;
}

var obj_from_void(void *ptr){
    if (!ptr) return NULL;

    if ((size_t)ptr < 10){
        fprintf(stderr, "ERROR: Address %p is not a valid object. Maybe you forgot to box an integer like \"num(123)\"?\n", ptr);
        exit(EXIT_FAILURE);
    }

    if (is_object(ptr)) return ptr;

    return str(ptr);
}

void print_n(int n, ...){
    begin_scope();

    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++){
        void *arg = va_arg(args, void*);
        var object = obj_from_void(arg);
        var s = to_str(object);
        const char *c = obj_cstr(s);

        printf("%s", c ? c : "(NULL)");

        if (i != n - 1){
            printf(" ");
        }
    }
    va_end(args);

    end_scope(NULL);
}
