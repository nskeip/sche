# Default compiler
COMPILER ?= clang

# Compilation flags
CFLAGS = -Wall -Wextra -g

# Source files
SRC_LIB_FILES = sche_lib.c

# Object files
OBJ_LIB_FILES = $(SRC_LIB_FILES:.c=.o)

# Output executable
EXECUTABLE = scheme-exe

TEST_EXECUTABLE = test-exe

# Targets
all: $(EXECUTABLE) $(TEST_EXECUTABLE)

$(EXECUTABLE): $(OBJ_LIB_FILES) main.o
	$(COMPILER) $(CFLAGS) $^ -o $@

$(TEST_EXECUTABLE): $(OBJ_LIB_FILES) test.o
	$(COMPILER) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJ_FILES) $(EXECUTABLE) $(TEST_EXECUTABLE) main.o test.o

test: clean $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

valgrind-test: clean $(TEST_EXECUTABLE)
	valgrind -s --track-origins=yes ./$(TEST_EXECUTABLE)

.PHONY: all clean test valgrind-test
