#!/bin/bash
#PBS -l select=1:ncpus=4:mem=2gb
#PBS -l walltime=0:05:00
#PBS -q short_cpuQ
#PBS -e log/err.err
#PBS -o log/out.out
cd ${PBS_O_WORKDIR}
module load mpich-3.2
mpirun.actual -n 4 out/hello_world
