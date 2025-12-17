#!/bin/bash
#PBS -l select=10:ncpus=10:mem=2gb
#PBS -l walltime=0:01:00
#PBS -l place=pack
#PBS -q short_cpuQ
#PBS -e bat_algorithm_hybrid/log/bat_algorithm_hybrid.err
#PBS -o bat_algorithm_hybrid/log/bat_algorithm_hybrid.out
cd ${PBS_O_WORKDIR}
module load mpich-3.2
mpirun.actual -n 10 bat_algorithm_hybrid/out/bat_algorithm_hybrid
