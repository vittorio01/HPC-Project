# Parameter Search Tutorial

This tutorial explains how to use and customize `parameter_search_new.py` for tuning the Bat Algorithm implementations across CPU, MPI, and HYBRID platforms.

## Quick Start

### Cluster Setup (First Time Only)

Before using the parameter search script on the cluster, load required modules and configure Python:

```bash
# Load necessary modules
module load Python/3.11.3-GCCcore-12.3.0
module load OpenMPI/4.1.5-GCC-12.3.0
module load GSL/2.7-GCC-12.3.0

# Upgrade pip
python -m pip install --upgrade pip

# Install required Python libraries
pip install numpy
```

### Build Implementations

Use just one of the 3 below:
```bash
# From project directory
make PRG=bat_algorithm_cpu compile    # For CPU version or
make PRG=bat_algorithm_MPI compile    # For MPI version or
make PRG=bat_algorithm_hybrid compile # For HYBRID version
```

### Basic Usage

```bash
# CPU with LOW granularity (quick test)
python parameter_search_new.py -I CPU -G LOW -O ACCURACY

# MPI with MEDIUM granularity (balanced search)
python parameter_search_new.py -I MPI -G MEDIUM -O ACCURACY

# HYBRID with HIGH granularity (comprehensive, ~5 min)
python parameter_search_new.py -I HYBRID -G HIGH -O ACCURACY
```

### Command-Line Arguments

| Flag | Options | Default | Description |
|------|---------|---------|-------------|
| `-I` / `--implementation` | CPU, MPI, HYBRID | CPU | Which algorithm implementation to test |
| `-G` / `--granularity` | LOW, MEDIUM, HIGH | MEDIUM | Search granularity (affects number of tests) |
| `-O` / `--objective` | ACCURACY, TIME | ACCURACY | Optimize for fitness or execution time |
| `--max_exec_time` | float (seconds) | 60 | Max execution time per test |
| `--accuracy_level` | float | 0.5 | Minimum acceptable fitness (when using TIME objective) |

## Granularity Levels Explained

### LOW Granularity (~25 tests, <1 sec)
**Sweeps:** Iterations, Radius
**Best for:** Quick validation, understanding basic parameter sensitivity

Example output:
```
[1/25] Best fitness: 6.581871e-04
[2/25] Best fitness: 6.581871e-04
...
[25/25] Best fitness: 2.56574e-07
```

### MEDIUM Granularity (~512 tests, ~4 min)
**Sweeps:** Dimension, Alpha, Gamma, Radius, Bats, Iterations
**Best for:** Practical tuning with reasonable time investment

Parameters:
- Dimension: 2 steps (2→10)
- Alpha: 4 steps (0.8→0.99)
- Gamma: 4 steps (0.8→0.99)
- Radius: 4 steps (1→5000)
- Bats: 2 steps (100→100k)
- Iterations: 2 steps (10→2000)

### HIGH Granularity (~256 tests, <5 min)
**Sweeps:** Bats, Radius, Dimension, Alpha, Gamma, Iterations
**Best for:** Final optimization with more parameter variety

Parameters same as MEDIUM but with selected 4-step sweeps.

## Customizing the Search

### Adding/Removing Parameters from Search

Edit the `search_space` dictionary in your desired granularity section:

```python
# Current MEDIUM setup
search_space = {
    "Dimension": np.linspace(DIM_MIN, DIM_MAX, 2).astype(int),
    "Alpha":    np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),
    "Gamma":    np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),
    "Radius":   np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX, num_steps),
    "Bats":     np.linspace(BATS_MIN, BATS_MAX, 2).astype(int),
    "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, 2).astype(int),
}
```

**To remove Bats:**
```python
search_space = {
    "Dimension": np.linspace(DIM_MIN, DIM_MAX, 2).astype(int),
    "Alpha":    np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),
    "Gamma":    np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),
    "Radius":   np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX, num_steps),
    "Iterations": np.linspace(ITERATIONS_MIN, ITERATIONS_MAX, 2).astype(int),
}
```

**To add Pulse:**
```python
search_space = {
    "Dimension": np.linspace(DIM_MIN, DIM_MAX, 2).astype(int),
    "Alpha":    np.linspace(ALPHA_MIN, ALPHA_MAX, num_steps),
    "Gamma":    np.linspace(GAMMA_MIN, GAMMA_MAX, num_steps),
    "Pulse":    np.linspace(PULSE_MIN, PULSE_MAX, 3).astype(int),  # Add 3 steps
    "Radius":   np.linspace(POS_RADIUS_MIN, POS_RADIUS_MAX, num_steps),
}
```

### Changing Parameter Bounds

At the top of `main()`, modify the bounds:

```python
# Current bounds
ALPHA_MIN = 0.8
ALPHA_MAX = 0.99
DIM_MIN = 2
DIM_MAX = 10
BATS_MIN = 100
BATS_MAX = 100000
ITERATIONS_MIN = 10
ITERATIONS_MAX = 2000
```

**Example: Search higher dimensions**
```python
DIM_MIN = 5
DIM_MAX = 20
```

**Example: More aggressive Bats sweep**
```python
BATS_MIN = 50
BATS_MAX = 500000
```

### Adjusting Steps Per Parameter

The `num_steps` variable controls how many discrete values each parameter takes:

```python
# Current config
steps_config = {'LOW': 5, 'MEDIUM': 4, 'HIGH': 4}
num_steps = steps_config[args.granularity]
```

**Example: Increase MEDIUM granularity to 6 steps each**
```python
steps_config = {'LOW': 5, 'MEDIUM': 6, 'HIGH': 4}
```

⚠️ **Warning:** Increasing steps exponentially increases total tests!
- MEDIUM with 5 steps per param: 5^6 = 15,625 tests (too many!)
- MEDIUM with 4 steps per param: 4^6 = 4,096 tests (still many)
- MEDIUM with 2 steps per param: 2^6 = 64 tests (reasonable)

### Calculating Total Tests

For each granularity, total tests = ∏(steps per parameter)

**MEDIUM example:**
- Dimension: 2 steps
- Alpha: 4 steps
- Gamma: 4 steps
- Radius: 4 steps
- Bats: 2 steps
- Iterations: 2 steps
- **Total: 2 × 4 × 4 × 4 × 2 × 2 = 512 tests**

## Understanding Output

```
Running MEDIUM granularity search: Sweeping Dimension, Alpha, Gamma, Radius, Bats, Iterations.
   -> Fit: 0.00007, Time: 0.00016s
[1/512] Best fitness: 7.144028e-05
   -> Fit: 0.00007, Time: 0.00401s
[2/512] Best fitness: 6.839854e-05
```

- `[1/512]` = Progress (test 1 of 512)
- `Best fitness: X.XXXXXXe-XX` = Best fitness found so far
- `-> Fit: X` = Fitness of current test
- `Time: X.XXXXs` = Execution time for current test

## Final Report

```
========================================
BEST CONFIGURATION FOUND:
  Bats: 1000
  Iterations: 132
  Dimension: 2
  Alpha: 0.8
  Gamma: 0.895
  Pulse: 0.1
  Loudness: 1.0
  Frequency Min: 0.0
  Frequency Max: 1.0
  Radius: 1.0
--------------------
Fitness: 2.56574e-07
Time:    0.012672s
========================================
```

## Tips & Tricks

### 1. Start with LOW, then MEDIUM, then HIGH
```bash
python parameter_search_new.py -I CPU -G LOW
python parameter_search_new.py -I CPU -G MEDIUM
python parameter_search_new.py -I CPU -G HIGH
```

### 2. Focus search on promising regions
If LOW finds good fitness with `Radius=1`, narrow HIGH's radius bounds:
```python
POS_RADIUS_MIN = 0.5  # Instead of 1
POS_RADIUS_MAX = 100  # Instead of 5000
```

### 3. Fix parameters you're confident about
If you know `Alpha=0.9` is good, don't sweep it:
```python
# Remove from search_space, it uses defaults
defaults["Alpha"] = 0.9
```

### 4. Optimize for execution time instead
```bash
python parameter_search_new.py -I CPU -G MEDIUM -O TIME --accuracy_level 0.5
```

### 5. Compare implementations
```bash
python parameter_search_new.py -I CPU -G MEDIUM
python parameter_search_new.py -I MPI -G MEDIUM
python parameter_search_new.py -I HYBRID -G MEDIUM
```

### 6. Increase timeout for slow configs
```bash
python parameter_search_new.py -I CPU -G MEDIUM --max_exec_time 120
```

## Common Issues

### Issue: Search times out
**Solution:** Reduce steps per parameter or remove parameters from sweep
```python
steps_config = {'LOW': 3, 'MEDIUM': 3, 'HIGH': 2}
```

### Issue: Fitness values seem off
**Solution:** Check that dimension is being parsed correctly
- Verify `--dim` parameter is being passed to C executable
- Check compilation: `make PRG=bat_algorithm_cpu compile`

### Issue: Best configuration hasn't changed after 100+ tests
**Solution:** Bounds might be too narrow or defaults already optimal
- Widen bounds in problem-prone parameters
- Try different objective (TIME instead of ACCURACY)

## Advanced: Writing Custom Sweeps

For highly customized searches, edit the granularity section directly:

```python
if args.granularity == 'CUSTOM':
    print("Running CUSTOM granularity search...")
    
    search_space = {
        "Bats": [100, 500, 1000, 5000],  # Manual values
        "Radius": [1, 10, 50, 100, 500],
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
        
        if best_result:
            print(f"[{test_num}/{total_tests}] Best fitness: {best_result['fitness']:.6e}")
```

Then add to argument parser:
```python
parser.add_argument("-G", "--granularity", type=str.upper, 
                    choices=['LOW', 'MEDIUM', 'HIGH', 'CUSTOM'], 
                    default='MEDIUM')
```

## Performance Benchmarks

On CPU (single core):
- LOW: ~0.5 seconds
- MEDIUM: ~2-4 minutes (512 tests)
- HIGH: ~5-10 minutes (256 tests, varies by parameters)

On MPI/HYBRID: Submitted as PBS jobs to cluster, check job status with `qstat`.
