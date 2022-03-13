#include "dynamic.h"

int main(){
    begin_scope();

    var a = num(1337);

    assert(a->value == 1337);

    println(a);

    end_scope(NULL);

    return 0;
}
