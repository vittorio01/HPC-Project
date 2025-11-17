# HPC Project repository 
This repository will be used for the source code of our High Performance Computing workgroup (exercises and final project). 

## Repository Structure

This repository follows standard C project conventions with the following structure:
- `lib/` - Contains reusable libraries that can be shared across different subprojects
- `project/` - Contains the source code for the final project and its subprojects
- `exercises/` - Contains subfolders for tests and exercises

## Environment setup

*Important*: Before doing anything remember to execute `module load mpich-3.2`.

## Working with Libraries

### Library Structure

Libraries are located in the `lib/` directory. Each library has its own subdirectory with the following structure:

```
lib/
├── Makefile           # Compiles all libraries
└── <library_name>/    # e.g., data/
    ├── src/           # Source files (.c)
    └── include/       # Header files (.h)
```

### How to Compile Libraries

To compile all libraries:
```bash
cd lib
make
```

To compile a specific library (e.g., data):
```bash
cd lib
make data
```

To clean library build files:
```bash
cd lib
make clean
```

To see available library commands:
```bash
cd lib
make help
```

### How to Write a New Library

1. Create a new directory under `lib/`:
   ```bash
   mkdir -p lib/mylib/src lib/mylib/include
   ```

2. Place your source files in `lib/mylib/src/`:
   - Example: `lib/mylib/src/mylib.c`

3. Place your header files in `lib/mylib/include/`:
   - Example: `lib/mylib/include/mylib.h`

4. Update `lib/Makefile`:
   - Add a new target for your library similar to the existing `data` target:
     ```makefile
     mylib:
         @echo "--- Compiling mylib library ---"
         @mkdir -p $(MYLIB_LIB)
         $(CC) $(CFLAGS) -c mylib/src/mylib.c -Imylib/include -o mylib/mylib.o
         @echo "--- mylib library compiled successfully ---"
     ```
   - Add `mylib` to the `all` target dependency list

5. Test your library compilation:
   ```bash
   cd lib
   make mylib
   ```

## Working with Project Code

### Project Structure

Projects are located in the `project/` directory. Each subproject has its own folder:

```
project/
├── makefile                    # Main project makefile
├── library_test/              # Example subproject
│   └── main.c                 # Main source file
└── bat_algorithm_MPI/         # Another subproject
    ├── main.c
    ├── bat.h
    └── bat.c
```

### How to Write Source Code Using Libraries

1. Create a new subproject folder in `project/`:
   ```bash
   mkdir project/my_project
   ```

2. Create your `main.c` file (and any other source files):
   ```bash
   touch project/my_project/main.c
   ```

3. In your source code, include library headers:
   ```c
   #include <data.h>  // From lib/data/include/data.h
   ```

4. The project makefile automatically:
   - Compiles all required libraries
   - Links them with your program
   - Sets up the correct include paths

### How to Compile a Project

Navigate to the `project/` directory and specify your subproject name:

```bash
cd project
make compile PRG=my_project
```

Available make commands:
- `make compile PRG=<folder>` - Compile your code and generate the `log/` and `out/` folders
- `make run PRG=<folder>` - Compile and submit your code to the cluster
- `make clean PRG=<folder>` - Delete all build directories and residual files

The makefile accepts optional variables:
- `CHUNKS` - Number of chunks to use for the program
- `PROCESSES` - Number of processes per chunk
- `RAM` - RAM to allocate for execution
- `PLACE` - Placing technique for processes

Example:
```bash
make run PRG=my_project CHUNKS=10 PROCESSES=32 RAM=10gb
```

The compiled executable will be placed in `project/<folder>/out/`, and logs will be in `project/<folder>/log/`.

### How to write and compile an exercise

*Important*: Before doing anything remember to execute `module load mpich-3.2`.

The `exercises` folder contains:
- `makefile` - An automation script for compiling programs and submitting code to the cluster
- Exercise folders with source code (the makefile compiles the `main.c` file automatically)

To add new exercise code:

1. Create a dedicated folder in `exercises/`:
   ```bash
   mkdir exercises/my_exercise
   ```

2. Create your `main.c` file:
   ```bash
   touch exercises/my_exercise/main.c
   ```

3. Compile and run:
   ```bash
   cd exercises
   make PRG=my_exercise
   ```

The makefile will automatically compile the source file, create the PBS script, and optionally submit it to the cluster. It will also create `log/` and `out/` subdirectories inside the exercise folder.

Available make commands:
- `make PRG=<folder>` or `make compile PRG=<folder>` - Compile your code
- `make run PRG=<folder>` - Compile and submit to the cluster
- `make clean PRG=<folder>` - Clean build files

Example with parameters:
```bash
make run PRG=hello_world CHUNKS=10 PROCESSES=32 RAM=10gb
```

## Quick Start Guide

### Creating a New Project with Library Support

1. **Write or use existing libraries:**
   ```bash
   cd lib
   make              # Compile all libraries
   ```

2. **Create your project:**
   ```bash
   mkdir project/my_new_project
   cd project/my_new_project
   ```

3. **Write your main.c:**
   ```c
   #include <stdio.h>
   #include <data.h>  // Use library headers
   
   int main() {
       Vector* v = NULL;
       initVector(&v, 10);
       // ... your code ...
       destroyVector(&v);
       return 0;
   }
   ```

4. **Compile and run:**
   ```bash
   cd project
   make compile PRG=my_new_project
   # Or to submit to cluster:
   make run PRG=my_new_project
   ```

Remember to rerun `make` every time you want to run a different program on the cluster.
