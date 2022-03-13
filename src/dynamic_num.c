#include "dynamic.h"

TypeInfo num_type_info = {
    "num",
    NULL,
    NULL,
    obj_num_cmp,
    obj_num_str,
    obj_num_json,
    obj_num_hash,
};

bool obj_is_num(var object){
    return object && object->type_info == &num_type_info;
}

var num(num_type value){
    var object = obj_new(&num_type_info);
    object->value = value;
    return object;
}

