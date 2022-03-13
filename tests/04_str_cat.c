#include "dynamic.h"

int main(){
    begin_scope();

    var s = cat(str("1234"), str("56"));

    println(s);

    end_scope(NULL);

    return 0;
}
