#include "dynamic.h"

TypeInfo dbl_type_info = {
    "num",
    NULL,
    NULL,
    obj_dbl_cmp,
    obj_dbl_str,
    obj_dbl_json,
    obj_dbl_hash,
};

bool obj_is_dbl(var object){
    return object && object->type_info == &dbl_type_info;
}

var dbl(double value){
    var object = obj_new(&dbl_type_info);
    object->dvalue = value;
    return object;
}

