# HPC Project repository 
This repository will be used for the source code of our High Performance Computing workgroup (exercises and final project). 

## Environment setup
This repository contains two folders:
- `project` contains the source code for the final project.
- `exercises` contains more subfolders for some tests and exercises.

### How to write and compile an exercise
The `exercises` folder contains:
- `makefile`, a n automation script used for automatically compiling programs and creating the `run.sh` script.
- More `*exercise*` folders with the source code of the exercises (the makefile compiles automatically the `main.cpp` file).

To add new source code to compile first create a dedicated folder in `exercises` and his respective `main.cpp` file. 
The structure should be similar to that:
- `project/`
- `exercises/`
  - `hello world/`
    - `main.cpp`
  - `example1/`
    - `lib.h`
    - `lib.cpp`
    - `main.cpp`
  - `makefile`

To compile your program enter in the `exercise` filder and run the makefile specifying the name of the folder that you have created.
```
cd exercises
make PRG=*folder*
```
The makefile will automatically compile the source file and the PBS script `run.sh` for launching the program on the cluster. 
- `exercises/`
  - `log/`
  - `out/`
    - `hello_world`
  - `hello world/`
    - `main.cpp`
  - `makefile`

Then, to run the program:
```
qsub run.sh
```
The logs will be placed in the new folder `log/` created by the makefile. 
When compiling the programs, the makefile accepts other optional variables:
- *CHUNKS*, number of chunks to use for the program.
- *PROCESSES*, number of processes per chunk.
- *RAM*, ram to allocate for the execution of the program.
- *RUN*, the name of the generated PBS script.
- *PLACE*, the placing technique for the processes.
For example:
```
make PRG=hello_world CHUNKS=10 PROCESSES=32 RAM=10gb
```

Remember to rerun `make` everytime that you want to run a different program on the cluster (the `run.sh` script has to be recompiled).
  
