#include "dynamic.h"

TypeInfo str_type_info = {
    "str",
    obj_str_free,
    NULL,
    obj_str_cmp,
    obj_str_str,
    obj_str_json,
    obj_str_hash,
};

bool obj_is_str(var object){
    return object && object->type_info == &str_type_info;
}

var to_str(var object){
    return object ? object->type_info->str(object) : NULL;
}

char* obj_cstr(var object){
    return object ? object->string + object->start : NULL;
}

var obj_num_str(var object){
    return json(object);
}

var obj_dbl_str(var object){
    return json(object);
}

var obj_bool_str(var object){
    return json(object);
}

var obj_str_str(var object){
    return object;
}

var obj_arr_str(var object){
    return json(object);
}

var obj_map_str(var object){
    return json(object);
}

var obj_str_take_ownership(char *string, size_t length){
    var object = obj_new(&str_type_info);
    object->start = 0;
    object->string = string;
    object->length = length;
    object->capacity = length;
    return object;
}

var obj_str_from_length(const char *string, size_t length){
    char *copy = malloc(length + 1);
    memcpy(copy, string, length);
    copy[length] = '\0';
    return obj_str_take_ownership(copy, length);
}

var str(const char *string){
    return obj_str_from_length(string, strlen(string));
}

void obj_str_free(var object){
    free(object->string);
}

var join(var array, var separator){
    size_t length = array->length;
    size_t total_length = length > 0 ? (length - 1) * separator->length : 0;

    for (size_t i = 0; i < length; i++){
        var string = arr_at(array, i);

        assert(obj_is_str(string));

        total_length += string->length;
    }

    char *joined = malloc(total_length + 1);
    char *ptr = joined;

    for (size_t i = 0; i < length; i++){
        var string = arr_at(array, i);

        memcpy(ptr, obj_cstr(string), string->length);
        ptr += string->length;

        if (i != length - 1){
            memcpy(ptr, obj_cstr(separator), separator->length);
            ptr += separator->length;
        }
    }
    *ptr = '\0';

    return obj_str_take_ownership(joined, total_length);
}

var substr(var string, size_t i, size_t j){
    assert(obj_is_str(string));

    if (i > string->length) i = string->length;
    if (j > string->length) j = string->length;

    if (i >= j) return str("");

    return  obj_str_from_length(obj_cstr(string) + i, j - i);
}

size_t find(var haystack, var needle, size_t offset){
    assert(obj_is_str(haystack));
    assert(obj_is_str(needle));

    if (haystack->length < needle->length) return NPOS;

    for (size_t i = offset; i <= haystack->length - needle->length; i++){
        if (0 == memcmp(obj_cstr(haystack) + i, obj_cstr(needle), needle->length)){
            return i;
        }
    }

    return NPOS;
}

var split(var string, var separator){
    assert(obj_is_str(string));
    assert(obj_is_str(separator));
    assert(separator->length > 0);

    size_t last_match = 0;
    size_t offset = 0;
    var parts = arr0();

    while ((offset = find(string, separator, offset)) != NPOS){
        push(parts, substr(string, last_match, offset));
        offset += separator->length;
        last_match = offset;
    }

    push(parts, substr(string, last_match, offset));

    return parts;
}

var repeat_char(char c, size_t length){
    char *string = malloc(length + 1u);
    for (size_t i = 0; i < length; i++){
        string[i] = c;
    }
    string[length] = '\0';
    return obj_str_take_ownership(string, length);
}

var repeat(var object, size_t count){
    var repeated = obj_arr_new(NULL, count);
    var *ptr = repeated->objects;
    for (size_t i = 0; i < count; i++){
        *ptr++ = object;
    }
    return repeated;
}

var lstrip(var string){
    size_t i = 0;
    while (i < string->length && is_space(obj_cstr(string)[i])) i++;
    return substr(string, i, string->length);
}

var rstrip(var string){
    size_t j = string->length;
    while (j > 0 && is_space(obj_cstr(string)[j - 1])) j++;
    return substr(string, 0, j);
}

var strip(var string){
    size_t i = 0;
    while (i < string->length && is_space(obj_cstr(string)[i])) i++;
    size_t j = string->length;
    while (j > 0 && is_space(obj_cstr(string)[j - 1])) j++;
    return substr(string, i, j);
}

var lpad(var string, size_t length, char c){
    if (string->length >= length) return string;

    return cat(repeat_char(c, length - string->length), string);
}

var rpad(var string, size_t length, char c){
    if (string->length >= length) return string;

    return cat(string, repeat_char(c, length - string->length));
}

var obj_str_reserve(var string, size_t start, size_t capacity){
    if (string->capacity < capacity){
        assert(start + string->length <= capacity);

        // TODO check overflow
        char *new_string = malloc(capacity + 1);
        memcpy(new_string + start, obj_cstr(string), string->length);
        new_string[start + string->length] = '\0';

        free(string->string);

        string->start = start;
        string->string = new_string;
        string->capacity = capacity;
    }
    return string;
}

var push_char(var string, char c){
    assert(obj_is_str(string));

    if (string->length >= string->capacity){
        // TODO check overflow
        if (!obj_str_reserve(string, string->start, string->length * 3u / 2u + 1u)){
            return NULL;
        }
    }

    obj_cstr(string)[string->length] = c;
    string->length++;
    obj_cstr(string)[string->length] = '\0';

    return string;
}

var push_char_front(var string, char c){
    assert(obj_is_str(string));

    if (string->start == 0){
        size_t new_capacity = string->capacity * 3u / 2u + 1u;
        size_t new_start = new_capacity - string->capacity;

        if (!obj_str_reserve(string, new_start, new_capacity)){
            return NULL;
        }
    }

    string->start--;
    string->length++;
    obj_cstr(string)[0] = c;

    return string;
}

char pop_char(var string){
    assert(obj_is_str(string));
    assert(string->length > 0);

    char c = obj_cstr(string)[--string->length];
    obj_cstr(string)[string->length] = '\0';
    return c;
}

char pop_char_front(var string){
    string->length--;
    return obj_cstr(string)[string->start++];
}
