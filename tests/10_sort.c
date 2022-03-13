#include "dynamic.h"

int main(){
    begin_scope();

    var src = str("tests/10_sort_input.txt");

    var text = read_file(src);

    var lines = split(text, str("\n"));

    var sorted_lines = sorted(lines);

    var output_text = join(sorted_lines, str("\n"));

    println(lstrip(output_text));

    end_scope(NULL);

    return 0;
}

