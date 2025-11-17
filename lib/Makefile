# Library Makefile
# This makefile compiles all libraries in the lib/ directory
# Each library should have its own subdirectory with src/ and include/ folders

CC = gcc
CFLAGS = -std=c11 -Wall -fPIC -O2

# Library directories
DATA_LIB = data

.PHONY: all clean data

all: data

# Compile data library
data:
	@echo "--- Compiling data library ---"
	@mkdir -p $(DATA_LIB)
	$(CC) $(CFLAGS) -c $(DATA_LIB)/src/data.c -I$(DATA_LIB)/include -o $(DATA_LIB)/data.o
	@echo "--- data library compiled successfully ---"

clean:
	@echo "--- Cleaning library build files ---"
	rm -f $(DATA_LIB)/*.o
	@echo "--- Clean complete ---"

help:
	@echo "Library Makefile - Usage:"
	@echo "  make          - Compile all libraries"
	@echo "  make data     - Compile data library only"
	@echo "  make clean    - Remove all compiled object files"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Adding a new library:"
	@echo "  1. Create a new directory: lib/mylib/"
	@echo "  2. Add subdirectories: lib/mylib/src/ and lib/mylib/include/"
	@echo "  3. Place source files in lib/mylib/src/"
	@echo "  4. Place header files in lib/mylib/include/"
	@echo "  5. Add a new target in this Makefile similar to the 'data' target"
	@echo "  6. Add the new target to the 'all' dependency list"
