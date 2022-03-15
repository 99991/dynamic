#include "dynamic.h"

int main(){
    begin_scope();

    var string = str("Hello, World!/\\\"\b\f\n\r\t$☃£ह€한𐍈🐈🤔👻🤖🧙‍♂️");

    var json_encoded_string = json(string);

    println(json_encoded_string);

    var decoded_string = parse(json_encoded_string);

    assert(eq(string, decoded_string));

    end_scope(NULL);
    return 0;
}
