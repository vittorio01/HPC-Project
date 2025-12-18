CMP = mpicc
PRG = hello_world
RUN = $(PRG)/run.sh

CHUNKS = 1
PROCESSES = 1
RAM=2gb
PLACE=pack
WALLTIME=0:01:00


compile: libs-compile $(PRG)/main.c	
	@echo "--- Generating PBS script ---"
	rm -f $(RUN)
	echo '#!/bin/bash' >> $(RUN)
	echo '#PBS -l select=$(CHUNKS):ncpus=$(PROCESSES):mem=$(RAM)' >> $(RUN)
	echo '#PBS -l walltime=$(WALLTIME)' >> $(RUN)
	echo '#PBS -l place=$(PLACE)' >> $(RUN)
	echo '#PBS -q short_HPC4DS' >> $(RUN)
	echo '#PBS -e $(PRG)/log/$(PRG).err' >> $(RUN)
	echo '#PBS -o $(PRG)/log/$(PRG).out' >> $(RUN)
	echo 'cd $${PBS_O_WORKDIR}' >> $(RUN)
	echo 'module load mpich-3.2' >> $(RUN)
	echo 'mpirun.actual -n $(PROCESSES) $(PRG)/out/$(PRG)' >> $(RUN)
	chmod +x $(RUN)
	mkdir -p  $(PRG)/out/ $(PRG)/log/
	@echo "--- Compiling MPI program ---"
	# module load mpich-3.2
	$(CMP) -std=gnu11 -g -Wall -fopenmp -o $(PRG)/out/$(PRG) $(PRG)/main.c -lm -I $(LIBS) $(LIBS)/*.o
	@echo "--- Build complete ---"

run: compile
	@echo "--- Submitting job to cluster ---"
	qsub $(RUN)
	@echo "--- Job submitted ---"
	
clean: libs-clean 
	@echo "--- Cleaning up build files ---"
	rm -rf $(PRG)/out/ $(PRG)/log $(RUN)

include libraries/libs.mk
