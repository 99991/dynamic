CFLAGS      := -g -fsanitize=address -fsanitize=undefined -std=c99 -Wall -Wextra -pedantic
SRC         := $(wildcard src/dynamic_*.c)
TESTS_SRC   := $(sort $(wildcard tests/*.c))
TESTS       := $(patsubst %.c,%,$(TESTS_SRC))

all: example tests

# Combine multiple C files into a single C file.
# Rationale:
# - https://www.sqlite.org/amalgamation.html
# - Easier to deploy
# - Slightly faster
dynamic.c: $(SRC)
	cat $^ > $@

# Compile example
example: dynamic.o example.c
	$(CC) -o $@ $^ $(CFLAGS)

dynamic.o: dynamic.c dynamic.h
	$(CC) -c $< $(CFLAGS)

# Compile tests
tests: $(TESTS)

# Run tests
test: tests
	./tests/run_tests.sh $(TESTS)

# Define rules to build tests
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(TESTS): %: %.c dynamic.o
	$(CC) -I. -o $@ $^ $(CFLAGS)

# Cleanup
.PHONY: clean
clean:
	rm -rf example dynamic.o dynamic.c $(TESTS)
