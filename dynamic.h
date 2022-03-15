#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

// Count number of arguments. For example,
// COUNT_ARGS(1, 2, "chicken", "duck") becomes 4.
// Gotcha: Incorrectly returns 1 for zero arguments!
// There is a workaround for that, but it does not work with C99.
#define COUNT_ARGS(...) COUNT_GET_20TH(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define COUNT_GET_20TH(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, count, ...) count

// Largest possible value to indicate end of string or not found
#define NPOS (~(size_t)0)

// Silence compiler warning about unused variable
#define UNUSED(x) ((void)(x))

// Print one or more objects
#define print(...) print_n(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

// Like print, but with a trailing new line
#define println(...) do { print(__VA_ARGS__); printf("\n"); } while(0)

// Create an array of objects
#define arr(...) obj_arr_new((var[]){__VA_ARGS__}, COUNT_ARGS(__VA_ARGS__))

// Concatenate objects to string
#define cat(...) join(arr(__VA_ARGS__), str(""))

// Iterate over values in an array
#define foreach(i, value, values) for (size_t i = 0; i < (values)->length; i++){ var value = arr_at(values, i); { begin_scope();

// Iterate over items (key, value)-pairs of a hash map. Can access items like
// item->key and item->value.
#define foreachitem(item, map) for (MapItem *item = (map)->first; item; item = item->next){{ begin_scope();

// End a for loop
#define forend end_scope(NULL); }}

// Get ith element of an array
#define arr_at(array, i) (*arr_ptr((array), (i)))

typedef struct Object Object;
typedef struct TypeInfo TypeInfo;
typedef struct MapItem MapItem;

typedef Object* var;

typedef void (*object_free_func)(var object);
typedef var (*object_str_func)(var object);
typedef int (*object_cmp_func)(var object1, var object2);
typedef void (*object_json_func)(var object, var output, size_t indentation);
typedef size_t (*object_hash_func)(var object);
typedef void (*object_mark_func)(var object);

typedef ptrdiff_t num_type;

struct TypeInfo {
    const char *name;
    object_free_func free;
    object_mark_func mark;
    object_cmp_func cmp;
    object_str_func str;
    object_json_func json;
    object_hash_func hash;
};

struct MapItem {
    size_t key_hash;
    var key;
    var value;
    MapItem *prev;
    MapItem *next;
};

struct Object {
    // Objects are marked with magic bytes to differentiate them from strings.
    uint8_t magic[17];
    // To please some compilers.
    uint8_t _align[6];

    // Indicate whether the garbage collector (GC) can free this object.
    // Will be set by the mark(object) function.
    bool reachable;

    // Type information about this object (str, arr, num, map).
    TypeInfo *type_info;

    // All objects are linked in a linked list for memory management.
    var next;

    // Scope in which the object was created (needed by GC).
    size_t scope;

    // Object-specific struct members below.

    // These values could be in a union, but unnamed unions are not C99.
    char *string;
    var *objects;
    num_type value;
    double dvalue;
    bool bvalue;
    MapItem *items;

    size_t start;
    size_t capacity;
    size_t length;
    // These could be stored in MapItems* with some type casting
    MapItem *first;
    MapItem *last;
};

typedef struct Scope Scope;

struct Scope {
    var objects;
};

// String functions

var obj_str_take_ownership(char *string, size_t length);
var obj_str_from_length(const char *string, size_t length);
char* obj_cstr(var object);
var str(const char *string);
void obj_str_free(var object);
var split(var string, var separator);
var join(var array, var separator);
var substr(var string, size_t i, size_t j);
size_t find(var haystack, var needle, size_t offset);
var repeat_char(char c, size_t length);
var repeat(var object, size_t count);
var read_file(var path);
bool write_file(var path, var data);
var lstrip(var string);
var rstrip(var string);
var strip(var string);
var lpad(var string, size_t length, char c);
var rpad(var string, size_t length, char c);
var push_char(var string, char c);
var push_char_front(var string, char c);
char pop_char(var string);
char pop_char_front(var string);

// Object to string functions

var to_str(var object);
var obj_num_str(var object);
var obj_str_str(var object);
var obj_arr_str(var object);
var obj_map_str(var object);
var obj_dbl_str(var object);
var obj_bool_str(var object);

// Num functions

var num(num_type value);

// Double functions

var dbl(double value);

// Boolean functions

var obj_bool(bool value);

// Array functions

var obj_arr_new(var objects[], size_t length);
var arr0(void);
void obj_arr_free(Object *object);
var obj_arr_reserve(var array, size_t start, size_t capacity);
var push(var array, var object);
var push_front(var array, var object);
var pop(var arr);
var pop_front(var arr);
var first(var arr);
var second(var arr);
var third(var arr);
var last(var arr);

// Hash map functions

void obj_map_free(var object);
void obj_map_init(var object, size_t capacity);
var map();
void map_clear_item(MapItem *item);
void map_rebuild(var map, size_t capacity);
MapItem* map_find_item(var map, var key, size_t key_hash);
var map_get(var dict, var key);
void map_put(var map, var key, var value);
bool map_del(var dict, var key);
var map_items(var map);
var map_keys(var map);
var map_values(var map);
var zipmap(var keys, var values);

// Comparison

int cmp(var object1, var object2);
bool eq(var object1, var object2);
int cmp_natural(const char *a, const char *b);
int obj_str_cmp(var object1, var object2);
int obj_num_cmp(var object1, var object2);
int obj_arr_cmp(var object1, var object2);
int obj_map_cmp(var object1, var object2);
int obj_dbl_cmp(var object1, var object2);
int obj_bool_cmp(var object1, var object2);

// Hashing

size_t hash(var object);
size_t obj_num_hash(var object);
size_t obj_str_hash(var object);
size_t obj_arr_hash(var object);
size_t obj_map_hash(var object);
size_t obj_dbl_hash(var object);
size_t obj_bool_hash(var object);

// Type checking

bool obj_is_str(var object);
bool obj_is_num(var object);
bool obj_is_arr(var object);
bool obj_is_map(var object);
bool obj_is_dbl(var object);
bool obj_is_bool(var object);

// JSON

void obj_num_json(var object, var output, size_t indentation);
void obj_str_json(var object, var output, size_t indentation);
void obj_map_json(var object, var output, size_t indentation);
void obj_arr_json(var object, var output, size_t indentation);
void obj_dbl_json(var object, var output, size_t indentation);
void obj_bool_json(var object, var output, size_t indentation);
void obj_json(var object, var output, size_t indentation);
var json(var object);
var parse_str(var string);
var parse_cstr(const char *string);
var parse_cstr_length(const char *string, size_t length);
var parse(void *ptr);

// GC

bool is_object(void *ptr);
void obj_arr_mark(var object);
void obj_map_mark(var object);
void mark(var object);
void begin_scope();
var end_scope(var object_to_preserve);

// Miscellaneous

var obj_new(TypeInfo *type_info);
var find_consecutive(var string, bool (*is_wanted_char)(char c));
var map_get_default(var map, var key, var default_value);
void reverse(var arr);
var reversed(var arr);
var slice(var values, size_t i, size_t j);
void extend(var arr, var more);
var obj_identity(var object);
var arr_copy(var arr);
var sorted_by(var values, var (*get_key)(var));
var sorted(var values);
var take(var values, size_t n);
var fmap(var (*func)(var), var values);
bool is_digit(char c);
bool is_alpha(char c);
bool is_space(char c);
var obj_from_void(void *ptr);
void print_n(int n, ...);

static inline var* arr_ptr(var array, size_t i){
    assert(obj_is_arr(array));
    assert(i < array->length);

    return array->objects + array->start + i;
}
#endif
