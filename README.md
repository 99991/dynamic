The `dynamic` library turns C into a more dynamic language. It supports:

* Dynamic values, which can be strings (`str`), hashmaps (`map`), arrays (`arr`), integers (`num`) or double precision floating point numbers (`dbl`)
* Garbage collector
* Arrays (`arr`) with both efficient $O(1)$ appending (`push`) and prepending (`push_front`)
* JSON reading and writing
* `println` function that can print dynamic values

#### Example

Here is a valid C99 program to find the 5 most frequently occuring words in a document.

For more examples, refer to [tests/](https://github.com/99991/dynamic/tree/main/tests).

```js
#include "dynamic.h"

var count(var words){
    // The garbage collector will collect all variables between begin_scope() and
    // end_scope() which are not reachable through variables from another scope.
    begin_scope();

    var counts = map();

    foreach(i, word, words)
        var n = map_get_default(counts, word, num(0));

        // The int value of the dynamic object `n` can be accessed with `->value`.
        n->value++;

        map_put(counts, word, n);
    forend

    // The variable "counts" should not be collected by the garbage collector.
    // Passing it through end_scope(counts) prevents that.
    return end_scope(counts);
}

int main(){
    begin_scope();

    var filename = str("example.c");

    var text = read_file(filename);

    var words = find_consecutive(text, is_alpha);

    var counts = count(words);

    // Array of (word, count) pairs
    var pairs = map_items(counts);

    // Sort by count
    var sorted_pairs = sorted_by(pairs, second);

    var reversed_pairs = reversed(sorted_pairs);

    var top_5_pairs = take(reversed_pairs, 5);

    println("Top 5 most frequent words are:");
    println(repeat_char('-', 30), "\n");

    size_t longest = 0;
    foreach(i, pair, top_5_pairs)
        size_t length = first(pair)->length;
        if (length > longest) longest = length;
    forend

    foreach(i, pair, top_5_pairs)
        var word = arr_at(pair, 0);
        var count = arr_at(pair, 1);
        word = rpad(word, longest, ' ');
        count = lpad(to_str(count), 2, ' ');
        println(word, "(occurs", count, "times)");
    forend

    end_scope(NULL);

    return 0;
}
```

#### Scripting

Concatenate your C files and pipe them into [Fabrice Bellard's awesome Tiny C Compiler.](https://bellard.org/tcc/)

```bash
cat dynamic.c example.c | tcc -run -
```

`tcc` will be finished executing this program before an interpreter like CPython has even started.

#### Notes

- `var` is just a `typedef` for `Object*`
- `Object` is a struct that is either an array (`arr`), string (`str`), integer (`num`) or hash map (`map`) depending on which function it was initialized with.
- `println` accepts a variable number of arguments of type `Object*` or char pointers. It works like this:
    - All objects start with 16 magic bytes and a null terminator. This information can be used to [tell them apart from strings](https://github.com/99991/dynamic/blob/a423a04061ee44bad0720fbd29f2321cc276564a/src/dynamic_gc.c#L21).
    - [This fancy macro](https://github.com/99991/dynamic/blob/a423a04061ee44bad0720fbd29f2321cc276564a/dynamic.h#L17) is used to count the number of arguments to enable functions with multiple parameters.

#### Disclaimer

This project was a fun vacation project, but it will probably not receive updates anytime soon. It has not been tested extensively, so be prepared for bugs!

#### Related

If you like this library, you may also like:

* [The Cello High Level C library](https://libcello.org/)
* [The Nim Programming Language](https://nim-lang.org/)
