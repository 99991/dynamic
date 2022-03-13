#include "dynamic.h"

var read_file(var path){
    assert(obj_is_str(path));
    FILE *f = fopen(path->string, "rb");
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
    FILE *f = fopen(path->string, "wb");
    if (!f) return false;
    if (data->length != fwrite(data->string, 1, data->length, f)){
        fclose(f);
        return false;
    }
    return 0 == fclose(f);
}
