#include "dynamic.h"

int main(){
    begin_scope();

    var a = arr(str("123"), num(456), str("789"));

    assert(a->length == 3);

    var a0 = first(a);
    var a1 = second(a);
    var a2 = third(a);

    var b0 = arr_at(a, 0);
    var b1 = arr_at(a, 1);
    var b2 = arr_at(a, 2);

    assert(eq(a0, b0));
    assert(eq(a1, b1));
    assert(eq(a2, b2));

    assert(0 != cmp(a0, b2));
    assert(0 != cmp(a2, b0));

    println("ok");

    end_scope(NULL);

    return 0;
}
