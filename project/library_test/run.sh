#!/bin/bash
#PBS -l select=1:ncpus=4:mem=2gb
#PBS -l walltime=0:01:00
#PBS -l place=pack
#PBS -q short_cpuQ
#PBS -e library_test/log/library_test.err
#PBS -o library_test/log/library_test.out
cd ${PBS_O_WORKDIR}
module load mpich-3.2
mpirun.actual -n 4 library_test/out/library_test
