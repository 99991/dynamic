# How to run the tests

Go to the parent directory and run `make test` which will automatically find the
test programs and pass them to the test script `run_tests.sh` to run the tests.

# How to make more tests

Create a test program C file like `<filename>.c` and an accompanying text file
with the expected output.
The text file must have the name `<filename>_expected_output.txt`.
If the text file does not exist yet, running the tests will create the file from
the first output.
