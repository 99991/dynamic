#include "dynamic.h"

size_t hash(var object){
    return object->type_info->hash(object);
}

size_t obj_num_hash(var object){
    num_type v = object->dvalue;
    size_t h = 0;
    memcpy(&h, &v, sizeof(h) < sizeof(v) ? sizeof(h) : sizeof(v));
    return h;
}

size_t obj_dbl_hash(var object){
    double v = object->dvalue;
    size_t h = 0;
    memcpy(&h, &v, sizeof(h) < sizeof(v) ? sizeof(h) : sizeof(v));
    return h;
}

size_t obj_bool_hash(var object){
    return object->bvalue ? 1 : 0;
}

size_t obj_str_hash(var object){
    const char *string = obj_cstr(object);
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

