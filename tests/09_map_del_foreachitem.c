#include "dynamic.h"

int main(){
    begin_scope();

    var m = map();

    map_put(m, str("key"), str("value"));

    map_put(m, str("1"), str("one"));
    map_put(m, str("2"), str("two"));
    map_put(m, str("3"), str("three"));

    map_del(m, str("key"));
    map_del(m, str("2"));

    foreachitem(item, m)
        println(item->key, item->value);
    forend

    end_scope(NULL);

    return 0;
}
