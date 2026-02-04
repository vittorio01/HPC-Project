PRG = bat_algorithm_cpu

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
MODULE_GSL=GSL/2.7-GCC-12.3.0

BATS = 1000
ITERATIONS = 50 
DIM = 2 
ALPHA = 0.8 
GAMMA = 0.895 
PULSE = 0.1 
LOUDNESS = 1.0 
FMIN = 0.0 
FMAX = 1.0 
RADIUS = 100.0
FUNCTION = sphere

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
	echo 'module load $(MODULE_GSL)' >> $(RUN)
	echo 'export OMP_NUM_THREADS=2' >> $(RUN)
	echo 'mpiexec --map-by core --bind-to core -n $(PROCESSES) $(OUT_FOLDER)/$(PRG) --bats $(BATS) --iterations $(ITERATIONS) --dim $(DIM) --alpha $(ALPHA) --gamma $(GAMMA) --pulse $(PULSE) --loudness $(LOUDNESS) --fmin $(FMIN) --fmax $(FMAX) --radius $(RADIUS) --function $(FUNCTION)' >> $(RUN)
	chmod +x $(RUN)
	mkdir -p  $(LOG_FOLDER) $(OUT_FOLDER) 
	@echo "--- Compiling MPI program ---"
	# module load mpich-3.2
	mpicc -std=$(STD) -g -Werror -fopenmp -o $(OUT_FOLDER)/$(PRG) $(PRG)/main.c -lm -I$(LIBS_OUT) -I$(LOCAL_INC_FOLDER) $(shell find ./$(LOCAL_SRC_FOLDER) -name "*.c") $(LIBS_OUT)/*.o $(shell gsl-config --cflags --libs)   
	@echo "--- Build complete ---"

run: compile
	@echo "--- Submitting job to cluster ---"
	qsub $(RUN)
	@echo "--- Job submitted ---"

run-local: compile 
	@echo "--- Executing program in local ---"
	mpiexec --map-by core --bind-to core -n $(PROCESSES) $(OUT_FOLDER)/$(PRG) --bats $(BATS) --iterations $(ITERATIONS) --dim $(DIM) --alpha $(ALPHA) --gamma $(GAMMA) --pulse $(PULSE) --loudness $(LOUDNESS) --fmin $(FMIN) --fmax $(FMAX) --radius $(RADIUS) --function $(FUNCTION)
	@echo "--- End of execution ---"

clean: libs-clean 
	@echo "--- Cleaning up build files ---"
	rm -rf $(BUILD_FOLDER)

include lib/lib.mk
