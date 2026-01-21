## TODO
- Refactor the code to be possible to use more dimensions and see if u can implement more
    stuff from Vittorio's lib
- consider if to remove the Bat structs
- find the deadlock
- try to run in the cluster MPI processes
- Create a lib to plot bats position at start and end
- Change from snake_case to camelCase


## Next Implementation Ideas
- Create a script to find the best parameters.
- We need to consider using another Rand like drand48() or something else
    cause apparently the normal rand is not suitable for simulations


## Log

- Moved my random_uniform() in shared tools.h . random_uniform is still not uniform so it needs probably updating or
    better alternative.

