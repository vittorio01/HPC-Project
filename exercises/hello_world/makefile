CMP = mpicc
PRG = hello_world

CHUNKS = 1
PROCESSES = 4
RAM=2gb


compile: $(COMPILER) run.sh src/
	mkdir -p src/ out/ log/
	$(CMP) -g -Wall -o out/$(PRG) src/$(PRG).c

run.sh:
	echo '#!/bin/bash' >> $@
	echo '#PBS -l select=$(CHUNKS):ncpus=$(PROCESSES):mem=$(RAM)' >> $@
	echo '#PBS -l walltime=0:05:00' >> $@
	echo '#PBS -q short_cpuQ' >> $@
	echo '#PBS -e log/err.err' >> $@
	echo '#PBS -o log/out.out' >> $@
	echo 'cd $${PBS_O_WORKDIR}' >> $@
	echo 'module load mpich-3.2' >> $@
	echo 'mpirun.actual -n $(PROCESSES) out/$(PRG)' >> $@
	chmod +x $@
	
clear: 
	rm -rf out/ log/ run.sh
