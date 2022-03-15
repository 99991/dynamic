#include "dynamic.h"

int main(){
    begin_scope();

    var s = str("");

    println(s);

    push_char(s, '1');

    println(s);

    push_char_front(s, '2');

    println(s);

    push_char_front(s, '3');

    println(s);

    push_char_front(s, '4');

    println(s);

    pop_char_front(s);

    println(s);

    pop_char(s);

    println(s);

    end_scope(NULL);
    return 0;
}
