#!/bin/bash
#PBS -l select=1:ncpus=1:mem=2gb
#PBS -l walltime=0:01:00
#PBS -l place=pack
#PBS -q short_cpuQ
#PBS -e build/log/bat_algorithm_MPI.err
#PBS -o build/log/bat_algorithm_MPI.out
cd ${PBS_O_WORKDIR}
module load mpich-3.2
mpiexec -n 1 build/out/bat_algorithm_MPI
