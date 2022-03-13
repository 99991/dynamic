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
    return object ? object->string : NULL;
}

var obj_num_str(var object){
    char buf[256];
    int length = snprintf(buf, sizeof(buf), "%zd", object->value);
    if (length < 0) return NULL;
    return obj_str_from_length(buf, (size_t)length);
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
    object->string = string;
    object->length = length;
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

        memcpy(ptr, string->string, string->length);
        ptr += string->length;

        if (i != length - 1){
            memcpy(ptr, separator->string, separator->length);
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

    return  obj_str_from_length(string->string + i, j - i);
}

size_t find(var haystack, var needle, size_t offset){
    assert(obj_is_str(haystack));
    assert(obj_is_str(needle));

    if (haystack->length < needle->length) return NPOS;

    for (size_t i = offset; i <= haystack->length - needle->length; i++){
        if (0 == memcmp(haystack->string + i, needle->string, needle->length)){
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
    while (i < string->length && is_space(string->string[i])) i++;
    return substr(string, i, string->length);
}

var rstrip(var string){
    size_t j = string->length;
    while (j > 0 && is_space(string->string[j - 1])) j++;
    return substr(string, 0, j);
}

var strip(var string){
    size_t i = 0;
    while (i < string->length && is_space(string->string[i])) i++;
    size_t j = string->length;
    while (j > 0 && is_space(string->string[j - 1])) j++;
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
