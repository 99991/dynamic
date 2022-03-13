#include "dynamic.h"

int main(){
    begin_scope();

    println(NULL);
    println(obj_cstr(obj_from_void(NULL)));
    println(obj_cstr(NULL));

    end_scope(NULL);

    return 0;
}
