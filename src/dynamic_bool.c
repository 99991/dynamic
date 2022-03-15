#include "dynamic.h"

TypeInfo bool_type_info = {
    "num",
    NULL,
    NULL,
    obj_bool_cmp,
    obj_bool_str,
    obj_bool_json,
    obj_bool_hash,
};

bool obj_is_bool(var object){
    return object && object->type_info == &bool_type_info;
}

var obj_bool(bool value){
    var object = obj_new(&dbl_type_info);
    object->bvalue = value;
    return object;
}

