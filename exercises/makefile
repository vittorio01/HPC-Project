CMP = mpicc
PRG = hello_world
RUN = run.sh

CHUNKS = 1
PROCESSES = 4
RAM=2gb


compile: $(COMPILER) $(PRG)/main.c	
	rm -f $(RUN)
	echo '#!/bin/bash' >> $(RUN)
	echo '#PBS -l select=$(CHUNKS):ncpus=$(PROCESSES):mem=$(RAM)' >> $(RUN)
	echo '#PBS -l walltime=0:05:00' >> $(RUN)
	echo '#PBS -q short_cpuQ' >> $(RUN)
	echo '#PBS -e log/$(PRG).err' >> $(RUN)
	echo '#PBS -o log/$(PRG).out' >> $(RUN)
	echo 'cd $${PBS_O_WORKDIR}' >> $(RUN)
	echo 'module load mpich-3.2' >> $(RUN)
	echo 'mpirun.actual -n $(PROCESSES) out/$(PRG)' >> $(RUN)
	chmod +x $(RUN)
	mkdir -p  out/ log/
	$(CMP) -g -Wall -o out/$(PRG) $(PRG)/main.c

run.sh:
	
clear: 
	rm -rf out/ log/ run.sh
