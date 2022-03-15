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
