#!/bin/bash
#PBS -l select=1:ncpus=50:mem=2gb
#PBS -l walltime=0:01:00
#PBS -l place=pack
#PBS -q shortCPUQ
#PBS -e bat_algorithm_hybrid/log/bat_algorithm_hybrid.err
#PBS -o bat_algorithm_hybrid/log/bat_algorithm_hybrid.out
cd ${PBS_O_WORKDIR}
module load OpenMPI/4.1.5-GCC-12.3.0
mpiexec -n 50 bat_algorithm_hybrid/out/bat_algorithm_hybrid
