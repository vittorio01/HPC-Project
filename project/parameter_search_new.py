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
import itertools
import re
import time
import os

def parse_benchmark_output(output_str):
    """Parses the standard output from the benchmark tool."""
    # Expected format: BENCHMARK_DATA, Fitness, Time, Bats, Iterations, Dimensions
    if not output_str: return None
    match = re.search(r"BENCHMARK_DATA,\s*([0-9eE\-\.]+),\s*([0-9eE\-\.]+),\s*(\d+),\s*(\d+),\s*(\d+)", output_str)
    if match:
        return {
            "fitness": float(match.group(1)),
            "time": float(match.group(2)),
            "bats": int(match.group(3)),
            "iterations": int(match.group(4)),
            "dim": int(match.group(5))
        }
    return None

def create_pbs_script(job_name, cmd_args, log_dir):
    """Creates a PBS script for cluster submission."""
    # Assuming standard project structure
    pbs_content = f"""#!/bin/bash
#PBS -l select=1:ncpus=1:mem=2gb
#PBS -l walltime=00:05:00
#PBS -q shortCPUQ
#PBS -o {log_dir}/{job_name}.out
#PBS -e {log_dir}/{job_name}.err

cd {os.getcwd()}
module load OpenMPI/4.1.5-GCC-12.3.0
module load GSL/2.7-GCC-12.3.0

{cmd_args}
"""
    script_path = Path(log_dir) / f"{job_name}.sh"
    with open(script_path, "w") as f:
        f.write(pbs_content)
    return script_path

def main():
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

    # -- SETUP ENVIRONMENT -- #
    project_root = Path.cwd()

    # Correct path mapping based on implementation
    if args.implementation == 'CPU':
        executable = project_root / "build/out/bat_algorithm_cpu"
    elif args.implementation == 'MPI':
        executable = project_root / "build/out/bat_algorithm_MPI"
    else:
        executable = project_root / "build/out/bat_algorithm_hybrid"

    # Ensure build/log exists
    log_dir = project_root / "build" / "log"
    log_dir.mkdir(parents=True, exist_ok=True)

    # -- SETUP PARAMETERS -- #
    F_MIN = 0.0
    F_MAX = 2.0
    BATS_MIN = 100
    BATS_MAX = 500
    
    # We fix reasonable bounds for the search
    ALPHA_MIN = 0.8
    ALPHA_MAX = 0.99
    GAMMA_MIN = 0.8
    GAMMA_MAX = 0.99
    PULSE_MIN = 0.1
    PULSE_MAX = 0.9
    LOUDNESS_MIN = 0.5
    LOUDNESS_MAX = 1.0
    
    # New parameters
    POS_RADIUS_MIN = 100
    POS_RADIUS_MAX = 1000
    DIM_MIN = 10
    DIM_MAX = 50

    # Define number of steps based on granularity
    # We reduce these significantly from the original 100/1000/10000 to make the search finite
    # LOW: very coarse (2-3 steps)
    # MEDIUM: moderate (3-4 steps)
    # HIGH: fine (5+ steps)
    steps_config = {'LOW': 2, 'MEDIUM': 3, 'HIGH': 5}
    num_steps = steps_config[args.granularity]

    # Defaults (used when not sweeping)
    defaults = {
        "Bats": 100,
        "Iterations": 500,
        "Dimension": 10,
        "Alpha": 0.9,
        "Gamma": 0.9,
        "Pulse": 0.5,
        "Loudness": 0.5,
        "Frequency Min": 0.0,
        "Frequency Max": 2.0,
        "Radius": 500.0
    }

    # Best tracking
    best_result = None
    best_params = None

    def evaluate_configuration(params):
        """Runs the benchmark with specific params and returns the parsed result."""
        # Build args
        cmd = [str(executable)]
        cmd += ["--bats", str(int(params["Bats"]))]
        cmd += ["--iterations", str(int(params["Iterations"]))]
        cmd += ["--dim", str(int(params["Dimension"]))]
        cmd += ["--alpha", str(params["Alpha"])]
        cmd += ["--gamma", str(params["Gamma"])]
        cmd += ["--pulse", str(params["Pulse"])]
        cmd += ["--loudness", str(params["Loudness"])]
        cmd += ["--fmin", str(params["Frequency Min"])]
        cmd += ["--fmax", str(params["Frequency Max"])]
        cmd += ["--radius", str(params["Radius"])]

        if args.implementation == 'CPU':
            # Run locally
            try:
                res = subprocess.run(cmd, capture_output=True, text=True, timeout=args.max_exec_time + 10)
                return parse_benchmark_output(res.stdout)
            except subprocess.TimeoutExpired:
                print("   [Timeout]")
                return None
        else:
            # Run on cluster (MPI/HYBRID)
            # Create a unique job name
            job_id = f"job_{int(time.time()*1000)}"
            cmd_str = "mpiexec -n 1 " + " ".join(cmd) # Placeholder for actual mpi args
            pbs_file = create_pbs_script(job_id, cmd_str, log_dir)
            
            # Submit
            try:
                # Try qsub
                sub = subprocess.run(["qsub", str(pbs_file)], capture_output=True, text=True)
                if sub.returncode != 0:
                    # Fallback to local execution if qsub fails (e.g. running on non-login node)
                    print(f" [Warn] qsub failed, running locally: {' '.join(cmd)}")
                    res = subprocess.run(["mpiexec", "-n", "1"] + cmd, capture_output=True, text=True)
                    return parse_benchmark_output(res.stdout)
                
                # If submitted, wait for output file
                out_file = log_dir / f"{job_id}.out"
                max_wait = args.max_exec_time + 30
                start_wait = time.time()
                while time.time() - start_wait < max_wait:
                    if out_file.exists():
                        time.sleep(1) # Flush
                        content = out_file.read_text()
                        return parse_benchmark_output(content)
                    time.sleep(1)
            except FileNotFoundError:
                 # Fallback
                res = subprocess.run(["mpiexec", "-n", "1"] + cmd, capture_output=True, text=True)
                return parse_benchmark_output(res.stdout)
            
            return None


    def update_best(res, params):
        nonlocal best_result, best_params

        if not res: return
        
        fitness = res['fitness']
        time_sec = res['time']
        
        print(f"   -> Fit: {fitness:.5f}, Time: {time_sec:.5f}s")
        
        if args.objective == 'ACCURACY':
            # Minimize Fitness, subject to time constraint
            if time_sec <= args.max_exec_time:
                if best_result is None or fitness < best_result['fitness']:
                    best_result = res
                    best_params = params.copy()
        else: # TIME
             # Minimize Time, subject to fitness constraint
             if fitness <= args.accuracy_level:
                if best_result is None or time_sec < best_result['time']:
                    best_result = res
                    best_params = params.copy()


    # -- LOW GRANULARITY HEURISTIC -- #
    if args.granularity == 'LOW':
        # Strategy: Fix problem size (Dim, Radius, Bats), Sweep algorithm main heuristics (Pulse, Loudness)
        # We assume Alpha/Gamma are less critical or standard (0.9)
        print("Running LOW granularity search: Sweeping Pulse and Loudness only.")
        
        search_space = {
            "Pulse":    np.linspace(PULSE_MIN, PULSE_MAX, num_steps),
            "Loudness": np.linspace(LOUDNESS_MIN, LOUDNESS_MAX, num_steps)
        }
        
        keys, values = zip(*search_space.items())
        total = 0
        for bundle in itertools.product(*values):
            total += 1
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            
            print(f"Test {total}: {bundle}")
            res = evaluate_configuration(current_params)
            update_best(res, current_params)


    # -- MEDIUM GRANULARITY HEURISTIC -- #
    if args.granularity == 'MEDIUM':
        # Strategy: Sweep Alpha, Gamma, Pulse, Loudness
        # Fix Bats and Dimensions to defaults
        print("Running MEDIUM granularity search: Sweeping Alpha, Gamma, Pulse, Loudness.")
        
        search_space = {
            "Alpha":    np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),
            "Gamma":    np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),
            "Pulse":    np.linspace(PULSE_MIN, PULSE_MAX, num_steps),
            "Loudness": np.linspace(LOUDNESS_MIN, LOUDNESS_MAX, num_steps),
        }

        keys, values = zip(*search_space.items())
        
        if args.implementation == 'CPU':
            for bundle in itertools.product(*values):
                current_params = defaults.copy()
                for key, val in zip(keys, bundle):
                    current_params[key] = val
                
                print(f"CPU Test: {bundle}")
                res = evaluate_configuration(current_params)
                update_best(res, current_params)

        if args.implementation == 'MPI':
            # Start sweeping the optimization space
            print("Starting MPI sweep...")
            for bundle in itertools.product(*values):
                current_params = defaults.copy()
                for key, val in zip(keys, bundle):
                    current_params[key] = val
                
                # Run jobs on cluster (handled by evaluate_configuration)
                print(f"MPI Test: {bundle}")
                res = evaluate_configuration(current_params)
                update_best(res, current_params)
        
        if args.implementation == 'HYBRID':
             for bundle in itertools.product(*values):
                current_params = defaults.copy()
                for key, val in zip(keys, bundle):
                    current_params[key] = val
                res = evaluate_configuration(current_params)
                update_best(res, current_params)


    # -- HIGH GRANULARITY HEURISTIC -- #
    if args.granularity == 'HIGH':
        # Sweep Everything including Bats and Frequency Max (approx)
        print("Running HIGH granularity search: Full sweep.")
        
        search_space = {
            "Bats":      np.linspace(BATS_MIN, BATS_MAX, steps_config['LOW']).astype(int), # Keep bats steps low
            "Alpha":     np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),
            "Gamma":     np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),
            "Pulse":     np.linspace(PULSE_MIN, PULSE_MAX, num_steps),
            "Loudness":  np.linspace(LOUDNESS_MIN, LOUDNESS_MAX, num_steps),
            "Radius":    np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX, steps_config['LOW'])
        }
        
        keys, values = zip(*search_space.items())
        for bundle in itertools.product(*values):
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            
            print(f"High Param Test: {bundle}")
            res = evaluate_configuration(current_params)
            update_best(res, current_params)

    # -- REPORT OBTAINED BEST PARAMETERS -- #
    print("\n" + "="*40)
    if best_result:
        print("BEST CONFIGURATION FOUND:")
        for k, v in best_params.items():
            print(f"  {k}: {v}")
        print("-" * 20)
        print(f"Fitness: {best_result['fitness']}")
        print(f"Time:    {best_result['time']}s")
    else:
        print("No suitable configuration found.")
    print("="*40)

if __name__ == "__main__":
    main()
