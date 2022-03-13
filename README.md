#### Want to write C99 compatible C code that looks vaguely like Python?
#### Want to have Python-like glacial performance but don't want to miss C's foot guns?
#### Do you want a garbage collector that simply invalidates your objects if you are not careful?
#### Is 2^-128 an acceptable error rate for type checking to you?
#### Do you want a scripting language so fast that it is already done before your average interpreter has even started?
### Do you want to push to the front of your arrays in O(1)?
### Do you think that insertion order is the only true iteration order for hash maps?
## Do you want to gloat over lesser programming languages without `lpad` function?
# Does your life feel empty without segfaults?
# Then this is for you!

#### Installation

Include `dynamic.h` and `dynamic.c` into your project and you, too, can live in this magical Christmas land that is dynamic C!

#### Example

Here is an example program to find the 5 most frequently occuring words in a document.

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

`tcc` will be finished before CPython has even started!

#### Notes

- Lol no generics (this isn't Go or C11)
- `var` is just a `typedef` for `Object*`
- `Object` is a struct that is either an array (`arr`), string (`str`), integer (`num`) or hash map (`map`) depending on which function it was initialized with.
- `println` accepts a variable number of arguments of type `Object*` or char pointers. How does that even work?
    - All objects start with 16 magic bytes and a null terminator. This information can be used to [tell them apart from strings](https://github.com/99991/dynamic/blob/a423a04061ee44bad0720fbd29f2321cc276564a/src/dynamic_gc.c#L21).
    - [This fancy macro](https://github.com/99991/dynamic/blob/a423a04061ee44bad0720fbd29f2321cc276564a/dynamic.h#L17) is used to count the number of arguments so you do not have to count them and pass them to the function yourself like some caveman.

#### Disclaimer

This project is obviously satire and probably full of bugs, but my vacation is almost over, so this won't see any further work for a while.

You are probably better served with a programming language like [Nim](https://nim-lang.org/).
