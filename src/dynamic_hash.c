#include "dynamic.h"

size_t hash(var object){
    return object->type_info->hash(object);
}

size_t obj_num_hash(var object){
    return (size_t)object->value;
}

size_t obj_str_hash(var object){
    const char *string = object->string;
    size_t length = object->length;

    // This would be the fnv1a32 hash if size_t was 32 bit.
    size_t h = 0x811c9dc5;
    size_t prime = 0x01000193;

    for (size_t i = 0; i < length; i++){
        h = (h ^ *(const unsigned char*)&string[i]) * prime;
    }

    return h;
}

size_t obj_arr_hash(var object){
    size_t h = 0x811c9dc5;
    size_t prime = 0x01000193;

    foreach(i, value, object)
        h = (h ^ hash(value)) * prime;
    forend

    return h;
}

size_t obj_map_hash(var object){
    size_t h = 0x811c9dc5;
    size_t prime = 0x01000193;

    foreachitem(item, object)
        h = (h ^ hash(item->key)) * prime;
        h = (h ^ hash(item->value)) * prime;
    forend

    return h;
}

