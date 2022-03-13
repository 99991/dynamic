# For each program passed as a command line argument
programs=$@
passed=0
total=0
printf '\n'
for program in $programs
do
    total=$((total+1))
    output="$program""_output.txt"

    # Execute the test program
    eval "$program" > "$output"

    # Check return value
    retval="$?"
    if [ "$retval" -ne 0 ]
    then
        printf '❌ %s failed with error code %s.\n' "$program" "$retval"
        continue
    fi

    expected_output="$program""_expected_output.txt"

    # Complain if file with expected output does not exist
    if [ ! -f "$expected_output" ]
    then
        cp "$output" "$expected_output"
        printf '❌ %s was missing. File has been created automatically.\n' "$expected_output"
        continue
    fi

    # Compare program output to expected output
    if diff "$output" "$expected_output"
    then
        printf '✅ %s passed\n' "$program"
    else
        printf '❌ %s failed with incorrect output. Compare %s and %s\n' "$program" "$output" "$expected_output"
        continue
    fi

    passed=$((passed+1))

    # Cleanup
    rm -f "$output"
done

printf '\n%s/%s tests passed.\n' "$passed" "$total"
