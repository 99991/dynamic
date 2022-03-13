#include "dynamic.h"

int main(){
    begin_scope();

    var m = map();

    map_put(m, str("key"), str("value"));
    map_put(m, num(1), str("one"));
    map_put(m, num(2), str("two"));
    map_put(m, num(3), str("three"));

    for (int i = 0; i <= 3; i++){
        var value = map_get(m, num(i));

        println(value);
    }

    println(map_get(m, str("key")));

    end_scope(NULL);

    return 0;
}
