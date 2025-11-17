CMP = mpicc
PRG = hello_world
RUN = $(PRG)/run.sh

CHUNKS = 1
PROCESSES = 1
RAM=2gb
PLACE=pack

# Library paths
LIB_DIR = ../lib
LIB_INCLUDE = $(LIB_DIR)/data/include
LIB_OBJECTS = $(LIB_DIR)/data/data.o

.PHONY: compile run clean libs

libs:
	@echo "--- Building libraries ---"
	$(MAKE) -C $(LIB_DIR)
	@echo "--- Libraries built ---"

compile: libs $(PRG)/main.c	
	@echo "--- Generating PBS script ---"
	rm -f $(RUN)
	echo '#!/bin/bash' >> $(RUN)
	echo '#PBS -l select=$(CHUNKS):ncpus=$(PROCESSES):mem=$(RAM)' >> $(RUN)
	echo '#PBS -l walltime=0:01:00' >> $(RUN)
	echo '#PBS -l place=$(PLACE)' >> $(RUN)
	echo '#PBS -q short_cpuQ' >> $(RUN)
	echo '#PBS -e $(PRG)/log/$(PRG).err' >> $(RUN)
	echo '#PBS -o $(PRG)/log/$(PRG).out' >> $(RUN)
	echo 'cd $${PBS_O_WORKDIR}' >> $(RUN)
	echo 'module load mpich-3.2' >> $(RUN)
	echo 'mpirun.actual -n $(PROCESSES) $(PRG)/out/$(PRG)' >> $(RUN)
	chmod +x $(RUN)
	mkdir -p  $(PRG)/out/ $(PRG)/log/
	@echo "--- Compiling MPI program ---"
	# module load mpich-3.2
	$(CMP) -std=c11 -g -Wall -o $(PRG)/out/$(PRG) $(PRG)/main.c -lm -I $(LIB_INCLUDE) $(LIB_OBJECTS)
	@echo "--- Build complete ---"

run: compile
	@echo "--- Submitting job to cluster ---"
	rm -f $(PRG)/log/*
	qsub $(RUN)
	@echo "--- Job submitted ---"
	
clean:
	@echo "--- Cleaning up build files ---"
	rm -rf $(PRG)/out/ $(PRG)/log $(RUN)
	@echo "--- Cleaning libraries ---"
	$(MAKE) -C $(LIB_DIR) clean
