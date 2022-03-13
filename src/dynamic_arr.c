#include "dynamic.h"

TypeInfo arr_type_info = {
    "arr",
    obj_arr_free,
    obj_arr_mark,
    obj_arr_cmp,
    obj_arr_str,
    obj_arr_json,
    obj_arr_hash,
};

bool obj_is_arr(var object){
    return object && object->type_info == &arr_type_info;
}

var obj_arr_new(var objects[], size_t length){
    var object = obj_new(&arr_type_info);
    object->objects = malloc(sizeof(*object->objects) * length);

    object->start = 0;
    object->length = length;
    object->capacity = length;
    for (size_t i = 0; i < length; i++){
        arr_at(object, i) = objects ? objects[i] : NULL;
    }

    return object;
}

var arr0(void){
    return obj_arr_new(NULL, 0);
}

void obj_arr_free(Object *object){
    free(object->objects);
}

var obj_arr_reserve(var array, size_t start, size_t capacity){
    if (array->capacity < capacity){
        assert(start + array->length < capacity);

        // TODO check overflow
        var *objects = malloc(sizeof(*objects) * capacity);

        for (size_t i = 0; i < array->length; i++){
            objects[start + i] = arr_at(array, i);
        }

        free(array->objects);

        array->start = start;
        array->objects = objects;
        array->capacity = capacity;
    }
    return array;
}

var push(var array, var object){
    if (array->length >= array->capacity){
        if (!obj_arr_reserve(array, array->start, array->length * 3u / 2u + 1u)){
            return NULL;
        }
    }

    array->length++;
    arr_at(array, array->length - 1) = object;

    return array;
}

var push_front(var array, var object){
    if (array->start == 0){
        size_t new_capacity = array->capacity * 3u / 2u + 1u;
        size_t new_start = new_capacity - array->capacity;

        if (!obj_arr_reserve(array, new_start, new_capacity)){
            return NULL;
        }
    }

    array->start--;
    array->length++;
    arr_at(array, array->start) = object;

    return array;
}

void reverse(var arr){
    assert(obj_is_arr(arr));

    for (size_t i = 0, j = arr->length - 1; i < arr->length / 2; i++, j--){
        var tmp = arr_at(arr, i);
        arr_at(arr, i) = arr_at(arr, j);
        arr_at(arr, j) = tmp;
    }
}

var reversed(var arr){
    assert(obj_is_arr(arr));

    var result = obj_arr_new(NULL, arr->length);

    for (size_t i = 0, j = arr->length - 1; i < arr->length; i++, j--){
        arr_at(result, j) = arr_at(arr, i);
    }

    return result;
}

var first(var arr){
    return arr_at(arr, 0);
}

var second(var arr){
    return arr_at(arr, 1);
}

var third(var arr){
    return arr_at(arr, 2);
}

var last(var arr){
    return arr_at(arr, arr->length - 1);
}

var pop(var arr){
    var value = last(arr);
    arr->length--;
    return value;
}

var pop_front(var arr){
    var value = first(arr);
    arr->start++;
    arr->length--;
    return value;
}

var arr_copy(var arr){
    return obj_arr_new(arr->objects + arr->start, arr->length);
}

var merge(var a, var b, var (*get_key)(var)){
    var merged = arr0();

    while (a->length > 0 && b->length > 0){
        push(merged, cmp(get_key(first(a)), get_key(first(b))) <= 0 ? pop_front(a) : pop_front(b));
    }

    extend(merged, a);
    extend(merged, b);

    return merged;
}

var sorted_by(var values, var (*get_key)(var)){
    if (values->length <= 1) return values;

    size_t middle = values->length / 2;

    var left = sorted_by(slice(values, 0, middle), get_key);
    var right = sorted_by(slice(values, middle, NPOS), get_key);

    return merge(left, right, get_key);
}

var sorted(var values){
    return sorted_by(values, obj_identity);
}

var take(var values, size_t n){
    return slice(values, 0, n);
}

var fmap(var (*func)(var), var values){
    assert(obj_is_arr(values));

    var result = obj_arr_new(NULL, values->length);

    foreach(i, value, values)
        arr_at(result, i) = func(value);
    forend

    return result;
}

