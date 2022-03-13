#include "dynamic.h"

int main(){
    begin_scope();

    var a = arr(str("123"), num(456));

    println(a);

    end_scope(NULL);

    return 0;
}
