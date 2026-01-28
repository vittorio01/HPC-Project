## TODO
- Create a lib to plot bats position at start and end. You can use any library in C which saves a figure in any format which
    can then be opened on our host laptops. 
- Change from snake_case to camelCase
- Create a script to find the best parameters.


## Next Implementation Ideas


## Log

- Created a new parseArguments() function in tools.c which gives the opportunity to pass 
    parameters from the command line and makefiles. This function only overrides the default 
    when the arguments are provided, otherwise the code uses the previous ones declared in 
    the file by you. This is useful since now I have a common interface for my python 
    script to give parameters and launch jobs on the cluster. Your main files have already been
    updated to use this function. 

- Created a new printBenchmarkData() function in benchmark.h, which at the end
    of every algorithm, prints a line in CSV format useful for external scripts
    to read execution time and fitness achieved;

- Implemented a parameter_search() script which finds the best parameters based on the desired implementation
    (MPI, CPU or HYBRID) and on objective (ACCURACY, TIME). There are also many other options which can and should
    be set. You can view all options by doing `python sweep_parameters.py -h` or `--help`. If you have any doubts just
    tell me. Also I did some tests and for now the results makes sense, but there may be some errors, so let me know.
