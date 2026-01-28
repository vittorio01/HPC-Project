# This python script will be used to explore the parameter space of the different algorithms, to find the optimal parameters
# to run the benchmarks.
# In particular it will:
# - Select a specific algorithm to optimize (between CPU, MPI or HYBRID)
# - The User can request the desired GRANULARITY of the parametric search. With an increased Granularity, more parameters
#   will be tested, but with a greater time penalty.
# - For each set of desired parameters, a job will be sent running on the cluster. From the job the data will then be collected
#   with the obtained fitness value and execution time.
# - An OBJECTIVE for the parametrization should also be chosen, between TIME and ACCURACY. If TIME is chosen, the fastest
#   algorithm will be selected, up to a certain ACCURACY_LEVEL desired. If ACCURACY is chosen then the algorithm with highest
#   accuracy will be selected, up to a certain MAX_EXEC_TIME
# - At the end of the algorithm the best found parameters will be returned.

import argparse
import subprocess
from pathlib import Path
import numpy as np

if __name__ == "__main__":
    # -- PARSING -- #
    parser = argparse.ArgumentParser(description="Parameter Search Script")
    parser.add_argument("-I", "--implementation", type=str.upper, choices=['CPU', 'MPI', 'HYBRID'], default='CPU', help='Either CPU, MPI or HYBRID')
    parser.add_argument("-O", "--objective", type=str.upper, choices=['TIME', 'ACCURACY'], default='ACCURACY', help='Choose whether to optimize for timing or accuracy')
    parser.add_argument("-G", "--granularity", type=str.upper, choices=['LOW', 'MEDIUM', 'HIGH'], default='MEDIUM', help='Either LOW, MEDIUM or HIGH')
    parser.add_argument("--accuracy_level", type=float, default=0.5, help='This selects the maximum possible fitness that can be achieved, when used in conjuction with TIME optimization')
    parser.add_argument("--max_exec_time", type=float, default=60, help='Maximum execution time in SECONDS that can be accepted when ACCURACY optimization is chosen')
    args = parser.parse_args()

    print(f"Chosen Implementation: {args.implementation}\n") 
    print(f"Chosen Objective: {args.objective}\n")
    if args.objective == 'TIME':
        print(f"Choosen min accuracy: {args.accuracy_level}\n")
    else:
        print(f"Choosen max exec time: {args.max_exec_time}\n")

    # -- ARGS CHECKS -- #

    # -- SETUP ENVIRONMENT -- #
    project_root = Path.cwd()
    main_file = project_root / f"bat_algorithm_{args.implementation.lower}" / "main.c"

    # -- SETUP PARAMETERS -- #
    F_MIN = 0
    F_MAX = 100
    BATS_MIN = 100
    BATS_MAX = 100000000
    ALPHA_MIN = 0
    ALPHA_MAX = 1
    GAMMA_MIN = 0
    GAMMA_MAX = 1
    PULSE_MIN = 0
    PULSE_MAX = 1
    LOUDNESS_MIN = 0
    LOUDNESS_MAX = 1
    POS_RADIUS_MIN = 1
    POS_RADIUS_MAX = 10000

    # helper to generate steps based on granularity level
    steps = {'LOW': 100, 'MEDIUM': 1000, 'HIGH': 10000}
    num_steps = steps[args.granularity]

    search_space = {
        "Frequency":    np.linspace(F_MIN, F_MAX, num_steps),
        "Bats":         np.linspace(BATS_MIN, BATS_MAX, num_steps),
        "Alpha":        np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),
        "Gamma":        np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),
        "Pulse":        np.linspace(PULSE_MIN, PULSE_MAX, num_steps),
        "Loudness":     np.linspace(LOUDNESS_MIN, LOUDNESS_MAX, num_steps),
        "Radius":       np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX),    
    }

    # -- LOW GRANULARITY HEURISTIC -- #
    if args.granularity == 'LOW':
        pass


    # -- MEDIUM GRANULARITY HEURISTIC -- #
    if args.granularity == 'MEDIUM':
        if args.implementation == 'CPU':
            pass

        if args.implementation == 'MPI':
            # Start sweeping the optimization space
            #    - Write pbs with new parameters
            #    - Run jobs on cluster
            #    - Verify that the job has run and read the results (how?)
            #    - proceed to next iteration
        
            # Return best parameter listing
            pass

        if args.implementation == 'HYBRID':
            pass


    # -- HIGH GRANULARITY HEURISTIC -- #
    if args.granularity == 'HIGH':
        pass

    # -- REPORT OBTAINED BEST PARAMETERS -- #