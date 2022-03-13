#include "dynamic.h"

var all_objects = NULL;
size_t num_old_objects = 0;
size_t num_new_objects = 0;

Scope *scopes = NULL;
size_t num_scopes = 1;
size_t max_scopes = 0;

uint8_t magic[17] = {
    130, 132, 136, 33,
    16, 65, 144, 32,
    17, 140, 9, 5,
    8, 152, 13, 49,
    0,
};

bool is_object(void *ptr){
    // objects must start with magic string
    return ptr && 0 == strcmp((char*)magic, (char*)((var)ptr)->magic);
}

static inline size_t current_scope(){
    return num_scopes - 1;
}

var obj_new(TypeInfo *type_info){
    var object = malloc(sizeof(*object));
    object->reachable = false;
    object->type_info = type_info;

    memcpy(object->magic, magic, sizeof(magic));

    // Connect object to current scope for garbage collection
    object->scope = current_scope();

    // Link object into all objects
    object->next = all_objects;
    all_objects = object;

    num_new_objects++;

    return object;
}

void begin_scope(){
    if (num_scopes >= max_scopes){
        // TODO check overflows
        max_scopes = num_scopes * 3u / 2u + 1;

        scopes = realloc(scopes, max_scopes * sizeof(*scopes));
    }

    scopes[num_scopes++] = (Scope){NULL};
}

void mark(var object){
    if (object && !object->reachable){
        object->reachable = true;

        if (object->type_info->mark){
            object->type_info->mark(object);
        }
    }
}

void gc(void){
    for (var object = all_objects; object; object = object->next){
        if (object->scope <= current_scope()){
            mark(object);
        }
    }

    num_old_objects = 0;
    num_new_objects = 0;
    var new_all_objects = NULL;
    for (var object = all_objects; object;){
        var next = object->next;

        if (object->reachable){
            // Keep reachable object
            num_old_objects++;

            object->reachable = false;

            object->next = new_all_objects;
            new_all_objects = object;

            object = next;
        }else{
            // Free unreachable object
            if (object->type_info->free){
                object->type_info->free(object);
            }
            free(object);
            object = next;
        }
    }

    all_objects = new_all_objects;
}

var end_scope(var object_to_preserve){
    num_scopes--;

    if (object_to_preserve){
        // Reassign object to be preserved to current scope
        object_to_preserve->scope = current_scope();
    }

    // Only run GC in global scope or if there are more new than old objects
    if (num_new_objects < num_old_objects && current_scope() != 0) return object_to_preserve;

    gc();

    return object_to_preserve;
}

void obj_arr_mark(var object){
    for (size_t i = 0; i < object->length; i++){
        mark(arr_at(object, i));
    }
}

void obj_map_mark(var object){
    for (MapItem *item = object->first; item; item = item->next){
        mark(item->key);
        mark(item->value);
    }
}
