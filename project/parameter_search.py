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

def create_pbs_script(job_name, cmd_args, log_dir, num_procs=1, is_hybrid=False):
    """Creates a PBS script for cluster submission."""
    # Assuming standard project structure
    num_threads = 1
    if is_hybrid:
        # For HYBRID: distribute CPUs between MPI and OpenMP
        # e.g., 4 MPI procs × 4 threads = 16 CPUs
        num_threads = max(1, 60 // num_procs)
    
    pbs_content = f"""#!/bin/bash
#PBS -l select=1:ncpus={num_procs}:mem=2gb
#PBS -l walltime=00:10:00
#PBS -q shortCPUQ
#PBS -o {log_dir}/{job_name}.out
#PBS -e {log_dir}/{job_name}.err

cd {os.getcwd()}
module load OpenMPI/4.1.5-GCC-12.3.0
module load GSL/2.7-GCC-12.3.0

export OMP_NUM_THREADS={num_threads}
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
    parser.add_argument("--mpi_procs", type=int, default=4, help='Number of MPI processes (default 4, max 60)')
    args = parser.parse_args()
    
    # Validate MPI processes
    if args.mpi_procs < 1 or args.mpi_procs > 60:
        print(f"Error: mpi_procs must be between 1 and 60, got {args.mpi_procs}")
        return
    
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
    # Bounds for heuristic parameters
    ALPHA_MIN = 0.8
    ALPHA_MAX = 0.99
    GAMMA_MIN = 0.8
    GAMMA_MAX = 0.99
    PULSE_MIN = 0.01
    PULSE_MAX = 0.9
    LOUDNESS_MIN = 0.1
    LOUDNESS_MAX = 1.0
    
    # Bounds for tuning knobs
    BATS_MIN = 100
    BATS_MAX = 1000000
    POS_RADIUS_MIN = 1
    POS_RADIUS_MAX = 5000
    ITERATIONS_MIN = 10 # Increased from 10 to avoid too-fast runs
    ITERATIONS_MAX = 500  # Reduced from 5000 to avoid timeouts
    DIM_MIN = 2
    DIM_MAX = 10

    # Define number of steps based on granularity
    # LOW: Quick sweep (~25 tests, <30 sec)
    # MEDIUM: Moderate sweep (~192 tests, ~90-120 sec)
    # HIGH: Comprehensive sweep (~256 tests, <300 sec, fits 5 min budget)
    steps_config = {'LOW': 5, 'MEDIUM': 4, 'HIGH': 4}
    num_steps = steps_config[args.granularity]

    # Defaults (used when not sweeping)
    defaults = {
        "Bats": 1000,
        "Iterations": 50,
        "Dimension": 2,
        "Alpha": 0.8,
        "Gamma": 0.895,
        "Pulse": 0.1,
        "Loudness": 1.0,
        "Frequency Min": 0.0,
        "Frequency Max": 1.0,
        "Radius": 100.0
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
            is_hybrid = args.implementation == 'HYBRID'
            
            # Build mpiexec command with proper process count
            mpi_cmd = f"mpiexec -n {args.mpi_procs} " + " ".join(cmd)
            pbs_file = create_pbs_script(job_id, mpi_cmd, log_dir, num_procs=args.mpi_procs, is_hybrid=is_hybrid)
            
            # Submit
            try:
                # Try qsub
                print(f"   [Submitting PBS job: {job_id} ({args.mpi_procs} procs)]", end=" ", flush=True)
                sub = subprocess.run(["qsub", str(pbs_file)], capture_output=True, text=True, timeout=10)
                if sub.returncode != 0:
                    # Fallback to local execution if qsub fails (e.g. running on non-login node)
                    print(f"[Warn] qsub failed, running locally with {args.mpi_procs} procs")
                    res = subprocess.run(["mpiexec", "-n", str(args.mpi_procs)] + cmd, capture_output=True, text=True, timeout=args.max_exec_time + 10)
                    return parse_benchmark_output(res.stdout)
                
                # If submitted, wait for output file
                out_file = log_dir / f"{job_id}.out"
                max_wait = args.max_exec_time + 60
                start_wait = time.time()
                elapsed = 0
                print(f"[waiting up to {max_wait}s]", flush=True)
                
                while elapsed < max_wait:
                    if out_file.exists():
                        time.sleep(0.5) # Flush
                        content = out_file.read_text()
                        if content.strip():  # Only parse non-empty output
                            return parse_benchmark_output(content)
                    elapsed = time.time() - start_wait
                    time.sleep(0.5)
                
                # Timeout waiting for output
                print(f"   [PBS output timeout after {max_wait}s]")
                return None
                
            except subprocess.TimeoutExpired as e:
                print(f"   [Error: {e}]")
                return None
            except FileNotFoundError as e:
                # Fallback to local mpiexec
                print(f"   [qsub not available, trying local mpiexec with {args.mpi_procs} procs]")
                try:
                    res = subprocess.run(["mpiexec", "-n", str(args.mpi_procs)] + cmd, capture_output=True, text=True, timeout=args.max_exec_time + 10)
                    return parse_benchmark_output(res.stdout)
                except subprocess.TimeoutExpired:
                    print("   [Timeout]")
                    return None
            except Exception as e:
                print(f"   [Error: {type(e).__name__}: {e}]")
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
        # Strategy: Sweep Iterations and Radius (main performance tuning knobs for low dim)
        # Fix heuristic params (Pulse, Loudness) to good defaults
        print("Running LOW granularity search: Sweeping Iterations and Radius.")
        
        search_space = {
            "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, num_steps).astype(int),
            "Radius":     np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX, num_steps)
        }
        
        keys, values = zip(*search_space.items())
        all_bundles = list(itertools.product(*values))
        total_tests = len(all_bundles)
        
        for test_num, bundle in enumerate(all_bundles, 1):
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            
            res = evaluate_configuration(current_params)
            update_best(res, current_params)
            
            # Progress update
            if best_result:
                print(f"[{test_num}/{total_tests}] Best fitness: {best_result['fitness']:.6e}")
            else:
                print(f"[{test_num}/{total_tests}] Running...")


    # -- MEDIUM GRANULARITY HEURISTIC -- #
    if args.granularity == 'MEDIUM':
        # Strategy: Sweep Dimension, Alpha, Gamma, Radius, Bats, Iterations (256 tests, ~4 min)
        # Fix: Pulse, Loudness at defaults
        print("Running MEDIUM granularity search: Sweeping Dimension, Alpha, Gamma, Radius, Bats, Iterations.")
        
        search_space = {
            "Dimension": np.linspace(DIM_MIN, DIM_MAX, 2).astype(int),  # 2 steps
            "Alpha":    np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),   # 4 steps
            "Gamma":    np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),   # 4 steps
            "Radius":   np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX, num_steps),  # 4 steps
            "Bats":     np.linspace(BATS_MIN, BATS_MAX, 2).astype(int),  # 2 steps
            "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, 2).astype(int),  # 2 steps
        }

        keys, values = zip(*search_space.items())
        all_bundles = list(itertools.product(*values))
        total_tests = len(all_bundles)
        
        for test_num, bundle in enumerate(all_bundles, 1):
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            
            res = evaluate_configuration(current_params)
            update_best(res, current_params)
            
            # Progress update
            if best_result:
                print(f"[{test_num}/{total_tests}] Best fitness: {best_result['fitness']:.6e}")
            else:
                print(f"[{test_num}/{total_tests}] Running...")


    # -- HIGH GRANULARITY HEURISTIC -- #
    if args.granularity == 'HIGH':
        # Comprehensive sweep for 5-min budget: Bats, Radius, Dimension, Alpha, Gamma, Iterations (256 tests)
        print("Running HIGH granularity search: Comprehensive sweep of all critical parameters (5 min budget).")
        
        search_space = {
            "Bats":      np.linspace(BATS_MIN, BATS_MAX, num_steps).astype(int),  # 4 steps
            "Radius":    np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX, num_steps),  # 4 steps
            "Dimension": np.linspace(DIM_MIN, DIM_MAX, 2).astype(int),             # 2 steps
            "Alpha":     np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),            # 4 steps
            "Gamma":     np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),            # 4 steps
            "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, 2).astype(int),  # 2 steps
        }
        
        keys, values = zip(*search_space.items())
        all_bundles = list(itertools.product(*values))
        total_tests = len(all_bundles)
        
        for test_num, bundle in enumerate(all_bundles, 1):
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            
            res = evaluate_configuration(current_params)
            update_best(res, current_params)
            
            # Progress update
            if best_result:
                print(f"[{test_num}/{total_tests}] Best fitness: {best_result['fitness']:.6e}")
            else:
                print(f"[{test_num}/{total_tests}] Running...")

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
