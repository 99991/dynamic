#include "dynamic.h"

TypeInfo map_type_info = {
    "map",
    obj_map_free,
    obj_map_mark,
    obj_map_cmp,
    obj_map_str,
    obj_map_json,
    obj_map_hash,
};

bool obj_is_map(var object){
    return object && object->type_info == &map_type_info;
}

void obj_map_free(var object){
    free(object->items);
}

void obj_map_init(var object, size_t capacity){
    object->items = calloc(sizeof(*object->items), capacity);
    object->capacity = capacity;
    object->length = 0;
    object->first = NULL;
    object->last = NULL;
}

var map(){
    var object = obj_new(&map_type_info);

    obj_map_init(object, 0);

    return object;
}

void map_clear_item(MapItem *item){
    *item = (MapItem){0, NULL, NULL, NULL, NULL};
}

void map_rebuild(var map, size_t capacity){
    Object tmp[1] = {*map};
    obj_map_init(tmp, capacity);

    for (MapItem *item = map->first; item; item = item->next){
        map_put(tmp, item->key, item->value);
    }

    free(map->items);
    *map = *tmp;
}

MapItem* map_find_item(var map, var key, size_t key_hash){
    if (map->capacity == 0) return NULL;

    size_t i = key_hash % map->capacity;

    for (size_t k = 0; k < map->capacity; k++){
        MapItem *item = map->items + i;

        if (!item->key) return item;

        if (item->key_hash == key_hash && eq(item->key, key)) return item;

        // Current entry is occupied with incorrect key. Try next one.
        i++;
        // If the end has been reached, wrap around to beginning.
        if (i >= map->capacity) i = 0;
    }

    return NULL;
}

var map_get(var dict, var key){
    size_t key_hash = hash(key);

    MapItem *item = map_find_item(dict, key, key_hash);

    return item && item->key ? item->value : NULL;
}

void map_put(var map, var key, var value){
    if (map->length * 3 >= map->capacity * 2){
        size_t new_capacity = map->capacity < 2 ? 2 : (map->capacity * 7 / 4);

        map_rebuild(map, new_capacity);
    }

    size_t key_hash = hash(key);

    MapItem *item = map_find_item(map, key, key_hash);

    // item != NULL because map_rebuild makes map->length < map->capacity
    assert(item);

    if (item->key){
        // Reuse item if it already exists.
        item->value = value;
    }else{
        // Populate unused item.
        map->length++;
        *item = (MapItem){key_hash, key, value, map->last, NULL};

        // Update map first and last pointers.
        if (map->last){
            map->last->next = item;
        }else{
            map->first = item;
        }
        map->last = item;
    }
}


static inline void unlink_item(var dict, MapItem *item){
    if (item->next){
        item->next->prev = item->prev;
    }
    if (item->prev){
        item->prev->next = item->next;
    }
    if (item == dict->first){
        dict->first = item->next;
    }
    if (item == dict->last){
        dict->last = item->prev;
    }
}

static inline void relink_item(var dict, MapItem *new_item, MapItem *old_item){
    if (new_item->prev){
        new_item->prev->next = new_item;
    }
    if (new_item->next){
        new_item->next->prev = new_item;
    }
    if (old_item == dict->first){
        dict->first = new_item;
    }
    if (old_item == dict->last){
        dict->last = new_item;
    }
}

bool map_del(var dict, var key){
    // Half dict capacity if it is three times larger than it needs to be.
    if (dict->length * 3 < dict->capacity){
        map_rebuild(dict, dict->capacity / 2);
    }

    size_t key_hash = hash(key);

    MapItem *item = map_find_item(dict, key, key_hash);

    if (!item || !item->key) return false;

    size_t i = (size_t)(item - dict->items);

    unlink_item(dict, item);
    map_clear_item(item);
    dict->length--;

    if (dict->length == 0) return true;

    // Reposition all following entries until we find an empty item.
    while (1){
        // Go to next item
        i++;
        if (i >= dict->capacity) i = 0;
        MapItem *old_item = dict->items + i;

        // Stop if we find an empty item.
        if (!old_item->key) return true;

        // Skip entries which already are at best possible locations.
        if (old_item->key_hash % dict->capacity == i) continue;

        // Move item to potentially better location
        MapItem tmp_item = *old_item;
        map_clear_item(old_item);
        MapItem *new_item = map_find_item(dict, tmp_item.key, tmp_item.key_hash);
        *new_item = tmp_item;
        relink_item(dict, new_item, old_item);
    }
}

var map_get_default(var map, var key, var default_value){
    size_t key_hash = hash(key);

    MapItem *item = map_find_item(map, key, key_hash);

    return item && item->key ? item->value : default_value;
}

var map_items(var map){
    var items = arr0();

    foreachitem(item, map)
        push(items, arr(item->key, item->value));
    forend

    return items;
}

var map_keys(var map){
    var keys = arr0();

    foreachitem(item, map)
        push(keys, item->key);
    forend

    return keys;
}

var map_values(var map){
    var values = arr0();

    foreachitem(item, map)
        push(values, item->value);
    forend

    return values;
}

var zipmap(var keys, var values){
    assert(keys->length == values->length);
    var m = map();
    for (size_t i = 0; i < keys->length; i++){
        map_put(m, arr_at(keys, i), arr_at(values, i));
    }
    return m;
}
