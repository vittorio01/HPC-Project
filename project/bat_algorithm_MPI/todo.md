## Next Implementation Ideas
- Maybe you can use a genetic algorithm to see which set of starting inital configurations parameters is the best?
- Be careful to when you use random numbers. In multithreaded apps, if you initialize the random number within the same second, then you have effectively created the same number.
- There is the need of new functions to better handle vectors and initializations.
- Before uploading to cluster, check that there are no memory issues. Check if everything is freed, and if the algorithm is correct
- Write test cases before implementing MPI
- Need to add more benchmarking functions
