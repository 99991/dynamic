#include "dynamic.h"

int main(){
    begin_scope();

    assert(eq(parse("0"), num(0)));
    assert(eq(parse("123"), num(123)));
    assert(eq(parse("-1234"), num(-1234)));

    assert(eq(parse("\"\""), str("")));
    assert(eq(parse("\"asdf\""), str("asdf")));

    // All numbers are exactly representable with IEEE 754 floating point.
    // Exact comparison is therefore allowed.
    assert(eq(parse("-2.0"), dbl(-2.0)));
    assert(eq(parse("-0.5"), dbl(-0.5)));
    assert(eq(parse("-0.5"), dbl(-0.5)));
    assert(eq(parse("0.125"), dbl(0.125)));
    assert(eq(parse("-1e1"), dbl(-10.0)));
    assert(eq(parse("-1e+1"), dbl(-10.0)));
    assert(eq(parse("-1.25e-1"), dbl(-0.125)));

    assert(eq(parse("[]"), arr0()));
    assert(eq(parse("[   ]"), arr0()));
    assert(eq(parse("[1,2]"), arr(num(1), num(2))));
    assert(eq(parse("[ 1 , 2 ]"), arr(num(1), num(2))));

    assert(eq(parse("{}"), map()));
    var m = map();
    map_put(m, str("one"), num(1));
    assert(eq(parse("{\"one\": 1}"), m));

    println("ok");
    end_scope(NULL);
    return 0;
}

