#include "dynamic.h"

var foo(){
    begin_scope();

    println("foo");

    var s = str("bar");

    // Return values must be passed through end_scope.
    // Otherwise, the garbage collector will eat them.
    return end_scope(s);
}

int main(){
    begin_scope();

    var s = foo();

    println(s);

    end_scope(NULL);

    return 0;
}
