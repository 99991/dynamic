#include "dynamic.h"

var count(var words){
    // The garbage collector will collect all variables between begin_scope() and
    // end_scope() which are not reachable by variables from another scope.
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
