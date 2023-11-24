# Default compiler
COMPILER ?= clang

# Compilation flags
CFLAGS = -Wall -Wextra -g

# Source files
SRC_FILES = main.c memory_tracker.c

# Object files
OBJ_FILES = $(SRC_FILES:.c=.o)

# Output executable
EXECUTABLE = scheme-exe

# Targets
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES)
	$(COMPILER) $(CFLAGS) $^ -o $@

%.o: %.c
	$(COMPILER) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_FILES) $(EXECUTABLE)

.PHONY: all clean
