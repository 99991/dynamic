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
    return object ? object->string + object->start : NULL;
}

var obj_num_str(var object){
    return json(object);
}

var obj_dbl_str(var object){
    return json(object);
}

var obj_bool_str(var object){
    return json(object);
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
    object->start = 0;
    object->string = string;
    object->length = length;
    object->capacity = length;
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

        memcpy(ptr, obj_cstr(string), string->length);
        ptr += string->length;

        if (i != length - 1){
            memcpy(ptr, obj_cstr(separator), separator->length);
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

    return  obj_str_from_length(obj_cstr(string) + i, j - i);
}

size_t find(var haystack, var needle, size_t offset){
    assert(obj_is_str(haystack));
    assert(obj_is_str(needle));

    if (haystack->length < needle->length) return NPOS;

    for (size_t i = offset; i <= haystack->length - needle->length; i++){
        if (0 == memcmp(obj_cstr(haystack) + i, obj_cstr(needle), needle->length)){
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
    while (i < string->length && is_space(obj_cstr(string)[i])) i++;
    return substr(string, i, string->length);
}

var rstrip(var string){
    size_t j = string->length;
    while (j > 0 && is_space(obj_cstr(string)[j - 1])) j++;
    return substr(string, 0, j);
}

var strip(var string){
    size_t i = 0;
    while (i < string->length && is_space(obj_cstr(string)[i])) i++;
    size_t j = string->length;
    while (j > 0 && is_space(obj_cstr(string)[j - 1])) j++;
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

var obj_str_reserve(var string, size_t start, size_t capacity){
    if (string->capacity < capacity){
        assert(start + string->length <= capacity);

        // TODO check overflow
        char *new_string = malloc(capacity + 1);
        memcpy(new_string + start, obj_cstr(string), string->length);
        new_string[start + string->length] = '\0';

        free(string->string);

        string->start = start;
        string->string = new_string;
        string->capacity = capacity;
    }
    return string;
}

var push_char(var string, char c){
    assert(obj_is_str(string));

    if (string->length >= string->capacity){
        // TODO check overflow
        if (!obj_str_reserve(string, string->start, string->length * 3u / 2u + 1u)){
            return NULL;
        }
    }

    obj_cstr(string)[string->length] = c;
    string->length++;
    obj_cstr(string)[string->length] = '\0';

    return string;
}

var push_char_front(var string, char c){
    assert(obj_is_str(string));

    if (string->start == 0){
        size_t new_capacity = string->capacity * 3u / 2u + 1u;
        size_t new_start = new_capacity - string->capacity;

        if (!obj_str_reserve(string, new_start, new_capacity)){
            return NULL;
        }
    }

    string->start--;
    string->length++;
    obj_cstr(string)[0] = c;

    return string;
}

char pop_char(var string){
    assert(obj_is_str(string));
    assert(string->length > 0);

    char c = obj_cstr(string)[--string->length];
    obj_cstr(string)[string->length] = '\0';
    return c;
}

char pop_char_front(var string){
    string->length--;
    return obj_cstr(string)[string->start++];
}
#include "dynamic.h"

int cmp(var object1, var object2){
    assert(object1 && object2 && object1->type_info == object2->type_info);

    return object1->type_info->cmp(object1, object2);
}

bool eq(var object1, var object2){
    return 0 == cmp(object1, object2);
}

int cmp_natural(const char *a, const char *b){
    while (*a != '\0' && *b != '\0'){
        if (!is_digit(*a) || !is_digit(*b)){
            if (*a < *b) return -1;
            if (*a > *b) return +1;
            a++;
            b++;
            continue;
        }

        const char *a_start = a;
        const char *b_start = b;

        while (*a && *a == '0') a++;
        while (*b && *b == '0') b++;

        const char *a_start_nonzero = a;
        const char *b_start_nonzero = b;

        while (*a && is_digit(*a)) a++;
        while (*b && is_digit(*b)) b++;

        size_t na = (size_t)(a - a_start_nonzero);
        size_t nb = (size_t)(b - b_start_nonzero);

        if (na < nb) return -1;
        if (na > nb) return +1;

        int c = memcmp(a_start_nonzero, b_start_nonzero, na);

        if (c != 0) return c;

        na = (size_t)(a - a_start);
        nb = (size_t)(b - b_start);

        if (na < nb) return -1;
        if (na > nb) return +1;
    }

    if (*a == '\0' && *b != '\0') return -1;
    if (*a != '\0' && *b == '\0') return +1;

    return 0;
}

int obj_str_cmp(var object1, var object2){
    return cmp_natural(obj_cstr(object1), obj_cstr(object2));
}

int obj_num_cmp(var object1, var object2){
    if (object1->value < object2->value) return -1;
    if (object1->value > object2->value) return +1;
    return 0;
}

int obj_dbl_cmp(var object1, var object2){
    if (object1->dvalue < object2->dvalue) return -1;
    if (object1->dvalue > object2->dvalue) return +1;
    return 0;
}

int obj_bool_cmp(var object1, var object2){
    if (object1->bvalue < object2->bvalue) return -1;
    if (object1->bvalue > object2->bvalue) return +1;
    return 0;
}

int obj_arr_cmp(var object1, var object2){
    size_t n = (object1->length < object2->length ? object1 : object2)->length;

    for (size_t i = 0; i < n; i++){
        int c = cmp(arr_at(object1, i), arr_at(object2, i));

        if (c != 0) return c;
    }

    if (object1->length < object2->length) return -1;
    if (object1->length > object2->length) return +1;

    return 0;
}

int obj_map_cmp(var object1, var object2){
    MapItem *a = object1->first;
    MapItem *b = object2->first;

    while (a && b){
        int c = cmp(a->key, b->key);

        if (c != 0) return c;

        c = cmp(a->value, b->value);

        if (c != 0) return c;

        a = a->next;
        b = b->next;
    }

    if (object1->length < object2->length) return -1;
    if (object1->length > object2->length) return +1;

    return 0;
}

#include "dynamic.h"

bool is_alpha(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_digit(char c){
    return '0' <= c && c <= '9';
}

bool is_space(char c){
    switch (c){
        case ' ': return true;
        case '\f': return true;
        case '\n': return true;
        case '\r': return true;
        case '\t': return true;
        case '\v': return true;
        default: return false;
    }
}

var find_consecutive(var string, bool (*is_wanted_char)(char c)){
    var matches = arr0();
    size_t length = string->length;

    for (size_t i = 0; i < length;){
        begin_scope();

        while (i < length && !is_wanted_char(obj_cstr(string)[i])) i++;

        size_t start = i;
        while (i < length && is_wanted_char(obj_cstr(string)[i])) i++;

        if (start != i){
            push(matches, substr(string, start, i));
        }
    }

    return matches;
}

var slice(var values, size_t i, size_t j){
    assert(obj_is_arr(values));

    if (i > values->length) i = values->length;
    if (j > values->length) j = values->length;

    if (i >= j) return obj_arr_new(NULL, 0);

    return obj_arr_new(values->objects + values->start + i, j - i);
}

void extend(var arr, var more){
    for (size_t i = 0; i < more->length; i++){
        push(arr, arr_at(more, i));
    }
}

var obj_identity(var object){
    return object;
}

var obj_from_void(void *ptr){
    if (!ptr) return NULL;

    if ((size_t)ptr < 10){
        fprintf(stderr, "ERROR: Address %p is not a valid object. Maybe you forgot to box an integer like \"num(123)\"?\n", ptr);
        exit(EXIT_FAILURE);
    }

    if (is_object(ptr)) return ptr;

    return str(ptr);
}

void print_n(int n, ...){
    begin_scope();

    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++){
        void *arg = va_arg(args, void*);
        var object = obj_from_void(arg);
        var s = to_str(object);
        const char *c = obj_cstr(s);

        printf("%s", c ? c : "(NULL)");

        if (i != n - 1){
            printf(" ");
        }
    }
    va_end(args);

    end_scope(NULL);
}
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

#include "dynamic.h"

void obj_num_json(var object, var output, size_t indentation){
    UNUSED(indentation);
    assert(obj_is_num(object));
    assert(obj_is_arr(output));

    char buf[256];
    int length = snprintf(buf, sizeof(buf), "%zd", object->value);
    if (length >= 0){
        var s = obj_str_from_length(buf, (size_t)length);
        push(output, s);
    }
}

void obj_dbl_json(var object, var output, size_t indentation){
    UNUSED(indentation);
    assert(obj_is_dbl(object));
    assert(obj_is_arr(output));

    char buf[256];
    // 20 digits should be enough
    int length = snprintf(buf, sizeof(buf), "%1.20g", object->dvalue);
    if (length >= 0){
        var s = obj_str_from_length(buf, (size_t)length);
        push(output, s);
    }
}

void obj_bool_json(var object, var output, size_t indentation){
    UNUSED(indentation);
    assert(obj_is_bool(object));
    assert(obj_is_arr(output));

    push(output, str(object->bvalue ? "true" : "false"));
}

bool is_json_control_character(char c){
    switch (c){
        case '\\': return true;
        case '"': return true;
        case '/': return true;
        case '\b': return true;
        case '\f': return true;
        case '\n': return true;
        case '\r': return true;
        case '\t': return true;
        default: return false;
    }
}

char* json_escape(char *ptr, char c){
    switch (c){
        case '\\': *ptr++ = '\\'; *ptr++ = '\\'; break;
        case '"': *ptr++ = '\\'; *ptr++ = '"'; break;
        case '/': *ptr++ = '\\'; *ptr++ = '/'; break;
        case '\b': *ptr++ = '\\'; *ptr++ = 'b'; break;
        case '\f': *ptr++ = '\\'; *ptr++ = 'f'; break;
        case '\n': *ptr++ = '\\'; *ptr++ = 'n'; break;
        case '\r': *ptr++ = '\\'; *ptr++ = 'r'; break;
        case '\t': *ptr++ = '\\'; *ptr++ = 't'; break;
        default: *ptr++ = c; break;
    }
    return ptr;
}

static inline void obj_str_json_append_utf16(var s, uint16_t x){
    push_char(s, '\\');
    push_char(s, 'u');

    for (int i = 12; i >= 0; i -= 4){
        uint16_t nibble = (x >> i) & 0xf;
        push_char(s, "0123456789abcdef"[nibble]);
    }
}

void obj_str_json(var object, var output, size_t indentation){
    UNUSED(indentation);
    assert(obj_is_str(object));
    assert(obj_is_arr(output));

    // See https://www.rfc-editor.org/rfc/rfc4627#section-2.5 for escape chars.

    size_t i = 0;
    size_t n = object->length;
    var escaped_string = str("\"");
    unsigned char *values = (unsigned char*)obj_cstr(object);

    while (i < n){
        // Decode UTF-8
        uint32_t a = values[i++], codepoint = 0;
        if ((a >> 7) == 0){
            codepoint = a;
        }else if ((a >> 5) == (0x03 << 1)){
            if (i + 1 > n){
                fprintf(stderr, "Incomplete utf-8.\n");
                return;
            }
            assert(i < n);
            uint32_t b = values[i++];
            codepoint = ((a & 0x1f) << 6) | (b & 0x3f);
        }else if ((a >> 4) == (0x07 << 1)){
            if (i + 2 > n){
                fprintf(stderr, "Incomplete utf-8.\n");
                return;
            }
            uint32_t b = values[i++];
            uint32_t c = values[i++];
            codepoint = ((a & 0x0f) << 12) | ((b & 0x3f) << 6) | (c & 0x3f);
        }else if ((a >> 3) == (0x0f << 1)){
            if (i + 3 > n){
                fprintf(stderr, "Incomplete utf-8\n");
                return;
            }
            uint32_t b = values[i++];
            uint32_t c = values[i++];
            uint32_t d = values[i++];
            codepoint = ((a & 0x07) << 18) | ((b & 0x3f) << 12) | ((c & 0x3f) << 6) | (d & 0x3f);
        }else{
            // Codepoint out of bounds
            fprintf(stderr, "Invalid utf-8\n");
            return;
        }

        var s = escaped_string;

        switch (codepoint){
            // Short escape codes for special chars
            case '\b': push_char(s, '\\'); push_char(s, 'b'); break;
            case '\f': push_char(s, '\\'); push_char(s, 'f'); break;
            case '\n': push_char(s, '\\'); push_char(s, 'n'); break;
            case '\r': push_char(s, '\\'); push_char(s, 'r'); break;
            case '\t': push_char(s, '\\'); push_char(s, 't'); break;
            case '\"': push_char(s, '\\'); push_char(s, '\"'); break;
            case '\\': push_char(s, '\\'); push_char(s, '\\'); break;

            default:
                // Escape small escape codes
                if (codepoint <= 0x1f){
                    obj_str_json_append_utf16(s, codepoint);
                    break;
                }

                // Do not escape "readable" chars
                if (codepoint <= 0x7f){
                    unsigned char u = codepoint;
                    push_char(s, *(char*)&u);
                    break;
                }

                // Escape 16 bit chars
                if (codepoint <= 0xffff){
                    obj_str_json_append_utf16(s, codepoint);
                    break;
                }

                // Escape surrogate pairs
                if (codepoint <= 0x10ffff){
                    uint32_t x = codepoint - 0x10000;
                    uint16_t hi = (x >> 10) | 0xd800;
                    uint16_t lo = (x & 0x3ff) | 0xdc00;
                    obj_str_json_append_utf16(s, hi);
                    obj_str_json_append_utf16(s, lo);
                    break;
                }

                return;
        }
    }

    push_char(escaped_string, '\"');
    push(output, escaped_string);
}

void obj_map_json(var object, var output, size_t indentation){
    indentation += 4;

    push(output, str("{\n"));
    foreachitem(item, object)
        push(output, repeat_char(' ', indentation));
        obj_json(to_str(item->key), output, indentation);
        push(output, str(": "));
        obj_json(item->value, output, indentation);

        if (item->next){
            push(output, str(", "));
        }
        push(output, str("\n")),
    forend
    indentation -= 4;
    push(output, repeat_char(' ', indentation));
    push(output, str("}"));
}

void obj_arr_json(var object, var output, size_t indentation){
    assert(obj_is_arr(object));
    assert(obj_is_arr(output));

    indentation += 4;
    push(output, str("[\n"));
    for (size_t i = 0; i < object->length; i++){
        push(output, repeat_char(' ', indentation));
        obj_json(arr_at(object, i), output, indentation);
        if (i != object->length - 1){
            push(output, str(",\n"));
        }else{
            push(output, str("\n"));
        }
    }
    indentation -= 4;
    push(output, repeat_char(' ', indentation));
    push(output, str("]"));
}

void obj_json(var object, var output, size_t indentation){
    if (object){
        object->type_info->json(object, output, indentation);
    }else{
        push(output, str("null"));
    }
}

var json(var object){
    var array = arr0();
    obj_json(object, array, 0);
    return join(array, str(""));
}

typedef struct CharStream CharStream;

struct CharStream {
    const char *string;
    size_t length;
    size_t pos;
    var error;
};

var parse_json_stream(CharStream *stream);

static inline size_t cstream_available(CharStream *stream){
    return stream->length - stream->pos;
}

static inline char cstream_peek_offset(CharStream *stream, size_t offset){
    return cstream_available(stream) > offset ?
        stream->string[stream->pos + offset] : '\0';
}

static inline char cstream_peek(CharStream *stream){
    return cstream_peek_offset(stream, 0);
}

bool consume_word(CharStream *stream, const char *word){
    size_t offset;
    for (offset = 0; *word; offset++){
        if (cstream_peek_offset(stream, offset) != *word) return false;
    }
    stream->pos += offset;
    return true;
}

void skip_whitespace(CharStream *stream){
    while (is_space(cstream_peek(stream))){
        stream->pos++;
    }
}

static inline char consume(CharStream *stream){
    assert(cstream_available(stream));
    return stream->string[stream->pos++];
}

static inline bool has_fraction_part(CharStream *stream){
    return cstream_peek(stream) == '.' &&
        is_digit(cstream_peek_offset(stream, 1));
}

static inline bool has_exponent_part(CharStream *stream){
    // [eE][+-]?[0-9]
    char first_char = cstream_peek(stream);

    if (first_char != 'e' && first_char != 'E') return false;

    char second_char = cstream_peek_offset(stream, 1);

    if (is_digit(second_char)) return true;

    if (second_char != '+' && second_char != '-') return false;

    return is_digit(cstream_peek_offset(stream, 2));
}

var parse_json_number(CharStream *stream){
    char number[256 + 1];
    size_t n = 0;
    const size_t n_max = sizeof(number) - 1;

    if (cstream_peek(stream) == '-'){
        number[n++] = consume(stream);
    }

    // There must be at least one digit.
    // JSON standard does not allow ".123".
    if (!is_digit(cstream_peek(stream))) return NULL;

    number[n++] = consume(stream);

    // Check for leading zeros (JSON standard does not allow them).
    if (number[n - 1] == '0' && is_digit(cstream_peek(stream))){
        return NULL;
    }

    while (is_digit(cstream_peek(stream))){
        if (n >= n_max) return NULL;
        number[n++] = consume(stream);
    }

    bool is_floating_point = false;

    // parse fractional part, for example .123
    if (has_fraction_part(stream)){
        is_floating_point = true;
        if (n >= n_max) return NULL;
        number[n++] = consume(stream);
        if (n >= n_max) return NULL;
        number[n++] = consume(stream);
        while (is_digit(cstream_peek(stream))){
            if (n >= n_max) return NULL;
            number[n++] = consume(stream);
        }
    }

    // parse exponent part, for example [eE][+-]?[0-9]
    if (has_exponent_part(stream)){
        is_floating_point = true;
        if (n >= n_max) return NULL;
        number[n++] = consume(stream);
        if (n >= n_max) return NULL;
        number[n++] = consume(stream);
        while (is_digit(cstream_peek(stream))){
            if (n >= n_max) return NULL;
            number[n++] = consume(stream);
        }
    }

    number[n] = '\0';

    if (is_floating_point){
        char *endptr = NULL;
        // TODO check over/underflow
        double value = strtod(number, &endptr);
        return endptr == number + n ? dbl(value) : NULL;
    }else{
        char *endptr = NULL;
        // TODO check over/underflow
        long long value = strtoll(number, &endptr, 10);
        return endptr == number + n ? num(value) : NULL;
    }
}

var parse_json_array(CharStream *stream){
    var array = arr0();
    consume(stream);
    while (1){
        skip_whitespace(stream);

        if (!cstream_available(stream)) return NULL;

        if (cstream_peek(stream) == ']'){
            consume(stream);
            return array;
        }

        var object = parse_json_stream(stream);

        push(array, object);

        skip_whitespace(stream);

        if (cstream_peek(stream) == ','){
            consume(stream);
        }else if (cstream_peek(stream) != ']'){
            return NULL;
        }
    }
}

var parse_json_map(CharStream *stream){
    var m = map();
    consume(stream);
    while (1){
        skip_whitespace(stream);

        if (!cstream_available(stream)) return NULL;

        if (cstream_peek(stream) == '}'){
            consume(stream);
            return m;
        }

        var key = parse_json_stream(stream);

        // JSON only allows string keys :(
        if (!obj_is_str(key)) return NULL;

        skip_whitespace(stream);

        if (cstream_peek(stream) != ':') return NULL;

        consume(stream);

        skip_whitespace(stream);

        var value = parse_json_stream(stream);

        // TODO what about duplicate keys?
        map_put(m, key, value);

        skip_whitespace(stream);

        if (cstream_peek(stream) == ','){
            consume(stream);
        }else if (cstream_peek(stream) != '}'){
            return NULL;
        }
    }
}

int char2int(char c){
    switch (c){
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;

        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;

        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;

        default: return -1;
    }
}

int codepoint2utf8(uint32_t codepoint, uint8_t *ptr){
    assert(codepoint <= 0x1fffff);

    if (codepoint <= 0x7f){
        *ptr++ = codepoint;
        return 1;
    }else if (codepoint <= 0x7ff){
        *ptr++ = (0x03 << 6) | ((codepoint >> 1 * 6) & 0x1f);
        *ptr++ = (0x01 << 7) | ((codepoint >> 0 * 6) & 0x3f);
        return 2;
    }else if (codepoint <= 0x00ffff){
        *ptr++ = (0x07 << 5) | ((codepoint >> 2 * 6) & 0x0f);
        *ptr++ = (0x01 << 7) | ((codepoint >> 1 * 6) & 0x3f);
        *ptr++ = (0x01 << 7) | ((codepoint >> 0 * 6) & 0x3f);
        return 3;
    }else if (codepoint <= 0x1fffff){
        *ptr++ = (0x0f << 4) | ((codepoint >> 3 * 6) & 0x07);
        *ptr++ = (0x01 << 7) | ((codepoint >> 2 * 6) & 0x3f);
        *ptr++ = (0x01 << 7) | ((codepoint >> 1 * 6) & 0x3f);
        *ptr++ = (0x01 << 7) | ((codepoint >> 0 * 6) & 0x3f);
        return 4;
    }
    return -1;
}

var parse_json_str(CharStream *stream){
    var string = str("");

    consume(stream);

    while (1){
        if (!cstream_available(stream)) goto premature_end;

        if (cstream_peek(stream) == '"'){
            consume(stream);
            return string;
        }

        char c = consume(stream);

        if (c == '\\'){
            if (!cstream_available(stream)) goto premature_end;

            c = cstream_peek(stream);

            switch (c){
                case '\\': break;
                case '\"': break;
                case '/': break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                case 'u':{
                    if (cstream_available(stream) < 5) goto premature_end;
                    consume(stream);

                    uint32_t codepoint = 0;

                    for (int i = 0; i < 2; i++){
                        for (int j = 0; j < 4; j++){
                            if (!cstream_available(stream)) goto premature_end;
                            int nibble = char2int(consume(stream));
                            if (nibble == -1) goto invalid_escape;
                            codepoint <<= 4;
                            codepoint |= nibble;
                        }

                        if (codepoint < 0xd800 || 0xdfff < codepoint) break;

                        if (cstream_peek(stream) != '\\') goto invalid_escape;
                        consume(stream);

                        if (cstream_peek(stream) != 'u') goto invalid_escape;
                        consume(stream);
                    }

                    if (codepoint > 0xffff){
                        uint32_t hi = codepoint >> 16;
                        uint32_t lo = codepoint & 0xffff;

                        if ((hi & (0x3f << 10)) != 0xd800) goto invalid_escape;
                        if ((lo & (0x3f << 10)) != 0xdc00) goto invalid_escape;

                        codepoint = ((hi & 0x3ff) << 10) | (lo & 0x3ff);
                        codepoint += 0x10000;
                    }

                    uint8_t utf8[4];
                    int n = codepoint2utf8(codepoint, utf8);

                    if (n == -1) goto invalid_escape;

                    for (int i = 0; i < n; i++){
                        push_char(string, *(char*)&utf8[i]);
                    }

                    continue;
                }

                default: goto invalid_escape;
            }

            consume(stream);
            push_char(string, c);
        }else{
            // TODO reject <= 0x20 or > 0x7f?
            push_char(string, c);
        }
    }

premature_end:
    stream->error = str("Premature end of stream while parsing string");
    return NULL;
invalid_escape:
    stream->error = str("Invalid escape code");
    return NULL;
}

var parse_json_stream(CharStream *stream){
    // TODO parse with a heap-allocated stack to avoid stack overflow for deeply nested objects
    skip_whitespace(stream);

    // Premature end of stream
    if (!cstream_available(stream)) return NULL;

    char c = cstream_peek(stream);

    if (is_digit(c) || c == '-'){
        return parse_json_number(stream);
    }else if (c == '['){
        return parse_json_array(stream);
    }else if (c == '"'){
        return parse_json_str(stream);
    }else if (c == '{'){
        return parse_json_map(stream);
    }else if (consume_word(stream, "null")){
        return NULL;
    }else if (consume_word(stream, "true")){
        return obj_bool(true);
    }else if (consume_word(stream, "false")){
        return obj_bool(false);
    }else{
        if (!stream->error){
            stream->error = str("Unexpected character while parsing new object");
        }
        return NULL;
    }
}

var parse_json_stream_strict(CharStream *stream){
    var object = parse_json_stream(stream);

    if (stream->error){
        println("ERROR: Could not parse JSON.", stream->error);
    }

    // Has the entire stream been parsed, or is there something left over?
    if (stream->pos != stream->length){
        println("ERROR: Could not parse JSON completely.");
    }

    return object;
}

var parse_cstr_length(const char *string, size_t length){
    CharStream stream[1] = {{string, length, 0, NULL}};
    return parse_json_stream_strict(stream);
}

var parse_cstr(const char *string){
    return parse_cstr_length(string, strlen(string));
}

var parse_str(var string){
    return parse_cstr_length(obj_cstr(string), string->length);
}

var parse(void *ptr){
    fprintf(stderr, "TODO: Error handling for parse function.\n");
    fprintf(stderr, "TODO: Handle integer/double overflow.\n");
    if (is_object(ptr)){
        return parse_str(ptr);
    }else{
        return parse_cstr(ptr);
    }
}
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
#include "dynamic.h"

var read_file(var path){
    assert(obj_is_str(path));
    FILE *f = fopen(obj_cstr(path), "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long llength = ftell(f);
    if (llength < 0){
        fclose(f);
        return NULL;
    }
    size_t length = (size_t)llength;
    rewind(f);
    char *string = malloc(length + 1u);
    if (!string){
        fclose(f);
        return NULL;
    }
    if (fread(string, 1, length, f) != length){
        free(string);
        fclose(f);
        return NULL;
    }
    string[length] = '\0';
    fclose(f);
    return obj_str_take_ownership(string, length);
}

bool write_file(var path, var data){
    assert(obj_is_str(path));
    assert(obj_is_str(data));
    FILE *f = fopen(obj_cstr(path), "wb");
    if (!f) return false;
    if (data->length != fwrite(obj_cstr(data), 1, data->length, f)){
        fclose(f);
        return false;
    }
    return 0 == fclose(f);
}
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
        assert(start + array->length <= capacity);

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

