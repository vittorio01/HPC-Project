# Library Directory

This directory contains reusable libraries for HPC projects.

## Structure

Each library has its own subdirectory with the following layout:

```
lib/
├── Makefile           # Build system for all libraries
└── <library_name>/    # e.g., data/
    ├── src/           # Source files (.c)
    └── include/       # Header files (.h)
```

## Available Libraries

### data
Location: `lib/data/`

The data library provides data structures and utilities for vector and matrix operations.

**Structures:**
- `Vector` - Dynamic vector with float data
- `Matrix` - Dynamic matrix with float data

**Functions:**
- Vector initialization, manipulation, and destruction
- Matrix initialization, manipulation, and destruction
- Copy operations between vectors and matrices
- Print utilities for debugging

**Headers:** `data.h`

**Usage example:**
```c
#include <data.h>

int main() {
    Vector* v = NULL;
    initVector(&v, 10);
    initVectorData(v, 0.0);
    printVector(v, 0, 10);
    destroyVector(&v);
    return 0;
}
```

## Building Libraries

### Build all libraries
```bash
cd lib
make
```

### Build specific library
```bash
cd lib
make data
```

### Clean build artifacts
```bash
cd lib
make clean
```

### Get help
```bash
cd lib
make help
```

## Adding a New Library

1. **Create directory structure:**
   ```bash
   mkdir -p lib/mylib/src lib/mylib/include
   ```

2. **Add your source files:**
   - Place `.c` files in `lib/mylib/src/`
   - Place `.h` files in `lib/mylib/include/`

3. **Update lib/Makefile:**
   
   Add a new target for your library:
   ```makefile
   MYLIB_LIB = mylib
   
   mylib:
       @echo "--- Compiling mylib library ---"
       @mkdir -p $(MYLIB_LIB)
       $(CC) $(CFLAGS) -c $(MYLIB_LIB)/src/mylib.c -I$(MYLIB_LIB)/include -o $(MYLIB_LIB)/mylib.o
       @echo "--- mylib library compiled successfully ---"
   ```
   
   Add `mylib` to the `all` target:
   ```makefile
   all: data mylib
   ```

4. **Update project Makefile:**
   
   In `project/makefile`, add your library to the build:
   ```makefile
   LIB_INCLUDE = $(LIB_DIR)/data/include $(LIB_DIR)/mylib/include
   LIB_OBJECTS = $(LIB_DIR)/data/data.o $(LIB_DIR)/mylib/mylib.o
   ```
   
   Update the compile command:
   ```makefile
   $(CMP) -std=c11 -g -Wall -o $(PRG)/out/$(PRG) $(PRG)/main.c -lm $(foreach inc,$(LIB_INCLUDE),-I$(inc)) $(LIB_OBJECTS)
   ```

5. **Document your library:**
   - Update this README with your library's description
   - Add usage examples
   - Document all public functions and structures

## Compilation Flags

Libraries are compiled with the following flags:
- `-std=c11` - C11 standard
- `-Wall` - All warnings enabled
- `-fPIC` - Position Independent Code (for potential shared library use)
- `-O2` - Optimization level 2

## Using Libraries in Projects

To use libraries in your project code:

1. Include the library header:
   ```c
   #include <data.h>
   ```

2. The project Makefile automatically:
   - Compiles the libraries
   - Adds include paths
   - Links object files

3. Just compile your project as usual:
   ```bash
   cd project
   make compile PRG=my_project
   ```
