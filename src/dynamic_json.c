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

void obj_str_json(var object, var output, size_t indentation){
    UNUSED(indentation);
    assert(obj_is_str(object));
    assert(obj_is_arr(output));

    size_t length = 2;

    // Count characters to escape
    for (size_t i = 0; i < object->length; i++){
        if (is_json_control_character(object->string[i])) length++;
        length++;
    }

    char *string = malloc(length + 1u);

    // Build string with leading/trailing/escaped double quotes
    char *ptr = string;
    *ptr++ = '"';
    for (size_t i = 0; i < object->length; i++){
        ptr = json_escape(ptr, object->string[i]);
    }
    *ptr++ = '"';
    *ptr++ = '\0';
    push(output, obj_str_take_ownership(string, length));
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

