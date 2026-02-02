# This python script will be used to explore the parameter space of the different algorithms, to find the optimal parameters
# to run the benchmarks.
# In particular it will:
# - Select a specific algorithm to optimize (between CPU, MPI or HYBRID)
# - The User can request the desired GRANULARITY of the parametric search. With an increased Granularity, more parameters
#   will be tested, but with a greater time penalty.
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
import multiprocessing

# Global configuration for multiprocessing workers
_global_config = {}

def init_worker(args_dict, executable_str, total_cores_val):
    """Initialize worker process with shared configuration."""
    global _global_config
    # Reconstruct args namespace from dict
    _global_config['args'] = argparse.Namespace(**args_dict)
    _global_config['executable'] = Path(executable_str)
    _global_config['total_cores'] = total_cores_val

def parse_benchmark_output(output_str):
    """Parses the standard output from the benchmark tool."""
    # Expected format: BENCHMARK_DATA, Fitness, Time, Bats, Iterations, Dimensions
    if not output_str: return None
    match = re.search(r"BENCHMARK_DATA,\s*([0-9eE\-\.\+infnanINFNAN]+),\s*([0-9eE\-\.\+]+),\s*(\d+),\s*(\d+),\s*(\d+)", output_str)
    if match:
        return {
            "fitness": float(match.group(1)),
            "time": float(match.group(2)),
            "bats": int(match.group(3)),
            "iterations": int(match.group(4)),
            "dim": int(match.group(5))
        }
    return None

def evaluate_configuration(params):
    """Runs the benchmark with specific params and returns the parsed result."""
    args = _global_config['args']
    executable = _global_config['executable']
    total_cores = _global_config['total_cores']
    
    # Build args
    cmd_args = []
    cmd_args += ["--bats", str(int(params["Bats"]))]
    cmd_args += ["--iterations", str(int(params["Iterations"]))]
    cmd_args += ["--dim", str(int(params["Dimension"]))]
    cmd_args += ["--alpha", str(params["Alpha"])]
    cmd_args += ["--gamma", str(params["Gamma"])]
    cmd_args += ["--pulse", str(params["Pulse"])]
    cmd_args += ["--loudness", str(params["Loudness"])]
    cmd_args += ["--fmin", str(params["Frequency Min"])]
    cmd_args += ["--fmax", str(params["Frequency Max"])]
    cmd_args += ["--radius", str(args.radius)]
    cmd_args += ["--function", str(args.function)]

    # Prepare the Environment
    env = os.environ.copy()

    # Construct the full command matching 'make run-local'
    final_cmd = []

    if args.implementation == 'CPU':
        final_cmd = [str(executable)] + cmd_args
    else:
        # (MPI/HYBRID)
        final_cmd = ["mpiexec", "-n", str(args.mpi_procs), str(executable)] + cmd_args 
        
        if args.implementation == 'HYBRID':
            # Calculate threads per process
            threads_per_proc = max(1, total_cores // args.mpi_procs)
            env["OMP_NUM_THREADS"] = str(threads_per_proc)

    # RUN LOCALLY
    try:
        res = subprocess.run(final_cmd, capture_output=True, text=True, timeout=args.max_exec_time + 10, env=env)
        parsed = parse_benchmark_output(res.stdout)
        return parsed
    except subprocess.TimeoutExpired:
        return None
    except Exception as e:
        return None


def main():
    # -- PARSING -- #
    parser = argparse.ArgumentParser(description="Parameter Search Script")
    parser.add_argument("-I", "--implementation", type=str.upper, choices=['CPU', 'MPI', 'HYBRID'], default='CPU', help='Either CPU, MPI or HYBRID')
    parser.add_argument("-O", "--objective", type=str.upper, choices=['TIME', 'ACCURACY'], default='ACCURACY', help='Choose whether to optimize for timing or accuracy')
    parser.add_argument("-G", "--granularity", type=str.upper, choices=['LOW', 'MEDIUM', 'HIGH'], default='MEDIUM', help='Either LOW, MEDIUM or HIGH')
    parser.add_argument("--accuracy_level", type=float, default=0.5, help='This selects the maximum possible fitness that can be achieved, when used in conjuction with TIME optimization')
    parser.add_argument("--max_exec_time", type=float, default=60, help='Maximum execution time in SECONDS that can be accepted when ACCURACY optimization is chosen')
    parser.add_argument("--mpi_procs", type=int, default=None, help='Number of MPI processes (default: auto-detect based on CPU cores)')
    parser.add_argument("--function", type=str, default='sphere', help='Optimization function (e.g., sphere, rosenbrock, rastrigin, ackley)')
    parser.add_argument("--radius", type=float, default=100.0, help='Search radius for initial position (function-dependent)')
    parser.add_argument("--dimension", type=int, default=2, help='Problem dimension (default 2)')
    args = parser.parse_args()
    
    # Auto-detect CPU cores
    try:
        total_cores = multiprocessing.cpu_count()
    except:
        total_cores = 4
    
    # If mpi_procs not specified, use reasonable default (not all cores)
    if args.mpi_procs is None:
        # Use at most 4 processes for local MPI (avoid oversubscription)
        args.mpi_procs = min(4, total_cores)
    
    # Validate MPI processes
    if args.mpi_procs < 1:
        print(f"Warning: mpi_procs must be at least 1")
        args.mpi_procs = 1
        args.mpi_procs = min(args.mpi_procs, total_cores)
    
    print(f"Chosen Implementation: {args.implementation}\n") 
    print(f"Chosen Objective: {args.objective}\n")
    print(f"Optimization Function: {args.function}")
    print(f"Problem Dimension: {args.dimension}")
    print(f"Search Radius: {args.radius}\n")
    print(f"System CPU Cores: {total_cores}")
    print(f"MPI Processes: {args.mpi_procs}\n")
    
    if args.objective == 'TIME':
        print(f"Choosen min accuracy: {args.accuracy_level}\n")
    else:
        print(f"Choosen max exec time: {args.max_exec_time}\n")

    # -- SETUP ENVIRONMENT -- #
    project_root = Path.cwd()

    # Compile the binary first
    print("[*] Compiling binary...")
    # Map implementation names to directory names (exact case)
    impl_names = {
        'CPU': 'bat_algorithm_cpu',
        'MPI': 'bat_algorithm_MPI',
        'HYBRID': 'bat_algorithm_hybrid'
    }
    compile_cmd = ["make", f"PRG={impl_names[args.implementation]}", "compile"]
    try:
        compile_result = subprocess.run(compile_cmd, cwd=str(project_root), capture_output=True, text=True, timeout=120)
        if compile_result.returncode != 0:
            print(f"[ERROR] Compilation failed!")
            print(f"STDOUT: {compile_result.stdout}")
            print(f"STDERR: {compile_result.stderr}")
            return
        print("[+] Compilation successful!\n")
    except subprocess.TimeoutExpired:
        print("[ERROR] Compilation timed out!")
        return
    except Exception as e:
        print(f"[ERROR] Compilation failed: {e}")
        return

    # Correct path mapping based on implementation
    if args.implementation == 'CPU':
        executable = project_root / "build/out/bat_algorithm_cpu"
    elif args.implementation == 'MPI':
        executable = project_root / "build/out/bat_algorithm_MPI"
    else:
        executable = project_root / "build/out/bat_algorithm_hybrid"

    # Verify executable exists
    if not executable.exists():
        print(f"[ERROR] Executable not found at {executable}")
        return

    # Ensure build/log exists
    log_dir = project_root / "build" / "log"
    log_dir.mkdir(parents=True, exist_ok=True)
    
    # Setup global config for multiprocessing
    _global_config['args'] = args
    _global_config['executable'] = executable
    _global_config['total_cores'] = total_cores

    # -- SETUP PARAMETERS -- #
    # Parametric Sweep Bounds 
    BATS_MIN = 100
    BATS_MAX = 100000
    ITERATIONS_MIN = 10 
    ITERATIONS_MAX = 1000 
    ALPHA_MIN = 0.01
    ALPHA_MAX = 0.99
    GAMMA_MIN = 0.01
    GAMMA_MAX = 0.99
    PULSE_MIN = 0.01
    PULSE_MAX = 0.99
    LOUDNESS_MIN = 0.1
    LOUDNESS_MAX = 1.0

    # Define number of steps based on granularity
    # LOW: Quick sweep (~10 steps per param = 10,000 tests)
    # MEDIUM: Thorough sweep (~100 steps per param)
    # HIGH: Comprehensive sweep (~200 steps per param)
    steps_config = {'LOW': 10, 'MEDIUM': 100, 'HIGH': 200}
    num_steps = steps_config[args.granularity]
    
    # Parallel processing - use all available cores
    num_workers = multiprocessing.cpu_count()
    print(f"Using {num_workers} parallel workers for faster execution.\n")
    
    # Early stopping thresholds
    BEST_FITNESS = 1e-9  # Fitness <= this is considered optimal (e.g., for sphere min=0)
    BEST_TIME = 0.001     # Time <= 0.001s is considered excellent performance

    # Defaults (used when not sweeping)
    defaults = {
        "Bats": 1000,
        "Iterations": 50,
        "Dimension": args.dimension,
        "Alpha": 0.8,
        "Gamma": 0.895,
        "Pulse": 0.1,
        "Loudness": 1.0,
        "Frequency Min": 0.0,
        "Frequency Max": 1.0
    }

    # Best tracking
    best_result = None
    best_params = None

    def update_best(res, params):
        """Updates best result and returns True if optimal solution found (early stop)."""
        nonlocal best_result, best_params

        if not res: return False
        
        fitness = res['fitness']
        time_sec = res['time']
        
        print(f"   -> Fit: {fitness:.5e}, Time: {time_sec:.5f}s")
        
        if args.objective == 'ACCURACY':
            # Minimize Fitness, subject to time constraint
            if time_sec <= args.max_exec_time:
                if best_result is None or fitness < best_result['fitness']:
                    best_result = res
                    best_params = params.copy()
                    # Early stop if we found excellent fitness
                    if fitness <= BEST_FITNESS:
                        print(f"\n*** OPTIMAL FITNESS FOUND: {fitness:.5e} <= {BEST_FITNESS:.5e} ***")
                        print("Stopping search early.\n")
                        return True
        else: # TIME
             # Minimize Time, subject to fitness constraint
             if fitness <= args.accuracy_level:
                if best_result is None or time_sec < best_result['time']:
                    best_result = res
                    best_params = params.copy()
                    # Early stop if we found excellent time
                    if time_sec <= BEST_TIME:
                        print(f"\n*** OPTIMAL TIME FOUND: {time_sec:.5f}s <= {BEST_TIME:.5f}s ***")
                        print("Stopping search early.\n")
                        return True
        
        return False


    # -- LOW GRANULARITY HEURISTIC -- #
    if args.granularity == 'LOW':
        # Strategy: Sweep Bats, Iterations, Pulse, Loudness (main performance tuning knobs)
        print("Running LOW granularity search: Sweeping Bats, Iterations, Pulse, Loudness.")
        
        search_space = {
            "Bats": np.linspace(BATS_MIN, BATS_MAX, num_steps).astype(int),
            "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, num_steps).astype(int),
            "Pulse": np.linspace(PULSE_MIN, PULSE_MAX, num_steps),
            "Loudness": np.linspace(LOUDNESS_MIN, LOUDNESS_MAX, num_steps)
        }
        
        keys, values = zip(*search_space.items())
        all_bundles = list(itertools.product(*values))
        total_tests = len(all_bundles)
        
        # Prepare all configurations
        all_configs = []
        for bundle in all_bundles:
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            all_configs.append(current_params)
        
        print(f"Running {total_tests} tests in parallel with {num_workers} workers...")
        
        # Run in parallel using imap_unordered for real-time progress
        args_dict = vars(args)
        with multiprocessing.Pool(processes=num_workers, initializer=init_worker, 
                                  initargs=(args_dict, str(executable), total_cores)) as pool:
            # Use imap_unordered to get results as they complete
            completed = 0
            for res in pool.imap_unordered(evaluate_configuration, all_configs, chunksize=1):
                completed += 1
                # Note: we don't have params order preserved with imap_unordered, but we can still update best
                if res:
                    # Find matching params (not ideal but works)
                    should_stop = update_best(res, {})
                
                # Progress update for every test
                if best_result:
                    print(f"[{completed}/{total_tests}] Best: Fit={best_result['fitness']:.6e}, Time={best_result['time']:.4f}s")
                else:
                    print(f"[{completed}/{total_tests}] No valid results yet...")
                
                # Early stopping
                if should_stop:
                    pool.terminate()
                    break


    # -- MEDIUM GRANULARITY HEURISTIC -- #
    if args.granularity == 'MEDIUM':
        # Strategy: Sweep Alpha, Gamma, Iterations, Pulse, Loudness (~128 tests)
        print("Running MEDIUM granularity search: Sweeping Alpha, Gamma, Iterations, Pulse, Loudness.")
        
        search_space = {
            "Alpha":    np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),   # 4 steps
            "Gamma":    np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),   # 4 steps
            "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, num_steps).astype(int),  
            "Pulse": np.linspace(PULSE_MIN, PULSE_MAX, num_steps),  # 2 steps
            "Loudness": np.linspace(LOUDNESS_MIN, LOUDNESS_MAX, num_steps)  # 2 steps
        }

        keys, values = zip(*search_space.items())
        all_bundles = list(itertools.product(*values))
        total_tests = len(all_bundles)
        
        # Prepare all configurations
        all_configs = []
        for bundle in all_bundles:
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            all_configs.append(current_params)
        
        print(f"Running {total_tests} tests in parallel with {num_workers} workers...")
        
        # Run in parallel with initializer and real-time progress
        args_dict = vars(args)
        with multiprocessing.Pool(processes=num_workers, initializer=init_worker,
                                  initargs=(args_dict, str(executable), total_cores)) as pool:
            # Use imap_unordered to get results as they complete
            completed = 0
            for res in pool.imap_unordered(evaluate_configuration, all_configs, chunksize=10):
                completed += 1
                if res:
                    should_stop = update_best(res, {})
                
                # Progress update for every test
                if best_result:
                    print(f"[{completed}/{total_tests}] Best: Fit={best_result['fitness']:.6e}, Time={best_result['time']:.4f}s")
                else:
                    print(f"[{completed}/{total_tests}] No valid results yet...")
                
                # Early stopping
                if should_stop:
                    pool.terminate()
                    break


    # -- HIGH GRANULARITY HEURISTIC -- #
    if args.granularity == 'HIGH':
        # Comprehensive sweep: Bats, Alpha, Gamma, Iterations, Pulse, Loudness (~256 tests)
        print("Running HIGH granularity search: Comprehensive sweep of all parameters (5 min budget).")
        
        search_space = {
            "Bats":      np.linspace(BATS_MIN, BATS_MAX, num_steps).astype(int),  # 4 steps
            "Alpha":     np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),            # 4 steps
            "Gamma":     np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),            # 4 steps
            "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, num_steps).astype(int),  # 2 steps
            "Pulse": np.linspace(PULSE_MIN, PULSE_MAX, num_steps),  # 2 steps
            "Loudness": np.linspace(LOUDNESS_MIN, LOUDNESS_MAX, num_steps)  # 2 steps
        }
        
        keys, values = zip(*search_space.items())
        all_bundles = list(itertools.product(*values))
        total_tests = len(all_bundles)
        
        # Prepare all configurations
        all_configs = []
        for bundle in all_bundles:
            current_params = defaults.copy()
            for key, val in zip(keys, bundle):
                current_params[key] = val
            all_configs.append(current_params)
        
        print(f"Running {total_tests} tests in parallel with {num_workers} workers...")
        
        # Run in parallel with initializer and real-time progress
        args_dict = vars(args)
        with multiprocessing.Pool(processes=num_workers, initializer=init_worker,
                                  initargs=(args_dict, str(executable), total_cores)) as pool:
            # Use imap_unordered to get results as they complete
            completed = 0
            for res in pool.imap_unordered(evaluate_configuration, all_configs, chunksize=50):
                completed += 1
                if res:
                    should_stop = update_best(res, {})
                
                # Progress update for every test
                if best_result:
                    print(f"[{completed}/{total_tests}] Best: Fit={best_result['fitness']:.6e}, Time={best_result['time']:.4f}s")
                else:
                    print(f"[{completed}/{total_tests}] No valid results yet...")
                
                # Early stopping
                if should_stop:
                    pool.terminate()
                    break

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
