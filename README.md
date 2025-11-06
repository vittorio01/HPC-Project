# HPC Project repository 
This repository will be used for the source code of our High Performance Computing workgroup (exercises and final project). 

## Environment setup
This repository contains two folders:
- `project` contains the source code for the final project.
- `exercises` contains more subfolders for some tests and exercises.

### How to write and compile an exercise

*Important*: Before doing anything remember to execute `module load mpich-3.2`.

The `exercises` folder contains:
- `makefile`, an automation script used for automatically compiling programs and submitting the code to the cluster.
- More `*exercise*` folders with the source code of the exercises (the makefile compiles automatically the `main.c` file).

To add new source code to compile first create a dedicated folder in `\exercises` (or `\project`) and his respective `main.c` file. 
The structure should be similar to that:
- `project\`
  - `project_module\`
    - `main.c`
  - `makefile`
- `exercises\`
  - `hello world\`
    - `main.c`
  - `example1\`
    - `lib.h`
    - `lib.c`
    - `main.c`
  - `makefile`

To compile your program enter in the `\exercise`folder (or `\project`) and run the makefile specifying the name of the folder that you have created.
```
cd exercises
make PRG=*folder*
```
The makefile will automatically compile the source file and the PBS script and (if specified), will also submit the script to the cluster. The script
will also create the subfolders `\log` and `\out` inside the project directory.
- `exercises/`
  - `hello world/`
    - `log/`
    - `out/`
    - `main.c`
  - `makefile`

There are mainly 3 `make` commands which are going to be useful:
- `make PRG=example_folder` or `make compile PRG=example_folder` in case you just want to compile your code and generate the `\log` and `\out` folders.
- `make run PRG=example_folder` if you want to *compile and submit* your code to the cluster. Results are gonna be find in `\example_folder\log`.
- `make clean PRG_example_folder` if you want to delete all directories and residual files from compilation, without deleting `main.c`.


The logs will be placed in the new folder `log/` created by the makefile. 
When compiling the programs, the makefile accepts other optional variables:
- *CHUNKS*, number of chunks to use for the program.
- *PROCESSES*, number of processes per chunk.
- *RAM*, ram to allocate for the execution of the program.
- *RUN*, the name of the generated PBS script.
- *PLACE*, the placing technique for the processes.
For example:
```
make run PRG=hello_world CHUNKS=10 PROCESSES=32 RAM=10gb
```

Remember to rerun `make` everytime that you want to run a different program on the cluster.
  
