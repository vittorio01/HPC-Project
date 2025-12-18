PRG = hello_world

STD = gnu11
BUILD_FOLDER = build
OUT_FOLDER = $(BUILD_FOLDER)/out
LOG_FOLDER = $(BUILD_FOLDER)/log
RUN = $(BUILD_FOLDER)/run.sh
LOCAL_INC_FOLDER = $(PRG)/include 
LOCAL_SRC_FOLDER = $(PRG)/src

CHUNKS = 1
PROCESSES = 1
CPUS = 1
RAM=2gb
PLACE=pack
WALLTIME=0:01:00
QUEUE=shortCPUQ
MODULE=OpenMPI/4.1.5-GCC-12.3.0

compile: libs-compile $(PRG)/main.c	
	@echo "--- Generating PBS script ---"
	rm -f $(RUN)
	echo '#!/bin/bash' >> $(RUN)
	echo '#PBS -l select=$(CHUNKS):ncpus=$(CPUS):mem=$(RAM)' >> $(RUN)
	echo '#PBS -l walltime=$(WALLTIME)' >> $(RUN)
	echo '#PBS -l place=$(PLACE)' >> $(RUN)
	echo '#PBS -q $(QUEUE)' >> $(RUN)
	echo '#PBS -e $(LOG_FOLDER)/$(PRG).err' >> $(RUN)
	echo '#PBS -o $(LOG_FOLDER)/$(PRG).out' >> $(RUN)
	echo 'cd $${PBS_O_WORKDIR}' >> $(RUN)
	echo 'module load $(MODULE)' >> $(RUN)
	echo 'mpiexec -n $(PROCESSES) $(OUT_FOLDER)/$(PRG)' >> $(RUN)
	chmod +x $(RUN)
	mkdir -p  $(LOG_FOLDER) $(OUT_FOLDER) 
	@echo "--- Compiling MPI program ---"
	# module load mpich-3.2
	mpicc -std=$(STD) -g -Wall -fopenmp -o $(OUT_FOLDER)/$(PRG) $(PRG)/main.c -lm -I$(LIBS_OUT) -I$(LOCAL_INC_FOLDER) $(shell find ./$(LOCAL_SRC_FOLDER) -name "*.c") $(LIBS_OUT)/*.o
	@echo "--- Build complete ---"

run: compile
	@echo "--- Submitting job to cluster ---"
	qsub $(RUN)
	@echo "--- Job submitted ---"
	
clean: libs-clean 
	@echo "--- Cleaning up build files ---"
	rm -rf $(BUILD_FOLDER)

include lib/lib.mk
