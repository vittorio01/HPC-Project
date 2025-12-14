#!/bin/bash
#PBS -l select=1:ncpus=1:mem=2gb
#PBS -l walltime=0:01:00
#PBS -l place=pack
#PBS -q short_cpuQ
#PBS -e bat_algorithm_MPI/log/bat_algorithm_MPI.err
#PBS -o bat_algorithm_MPI/log/bat_algorithm_MPI.out
cd ${PBS_O_WORKDIR}
module load mpich-3.2
mpirun.actual -n 1 bat_algorithm_MPI/out/bat_algorithm_MPI
