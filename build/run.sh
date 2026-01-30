#!/bin/bash
#PBS -l select=1:ncpus=1:mem=2gb
#PBS -l walltime=0:01:00
#PBS -l place=pack
#PBS -q shortCPUQ
#PBS -e build/log/bat_algorithm_MPI.err
#PBS -o build/log/bat_algorithm_MPI.out
cd ${PBS_O_WORKDIR}
module load OpenMPI/4.1.5-GCC-12.3.0
module load GSL/2.7-GCC-12.3.0
mpiexec -n 6 build/out/bat_algorithm_MPI --bats 1000 --iterations 50  --dim 2  --alpha 0.8  --gamma 0.1  --loudness 1.0  --fmin 0.0  --fmax 1.0  --radius 100.0 --function rosenbrock
