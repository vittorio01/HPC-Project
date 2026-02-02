/* Parallel MPI Version of the BAT Algorithm v0.2 */

#include <data.h>
#include <tools.h>
#include <benchmark.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <sys/time.h>
#include <time.h>

#define M_PI 3.14159265358979323846

// -- Bat Algorithm Parameters --
#define FMAX            2.0
#define FMIN            0.0
#define PULSE           0.05
#define LOUDNESS        1.0
#define GAMMA           0.9
#define ALPHA           0.9
#define VECTOR_DIM      2
#define POS_RADIUS      100
#define BATS            100000
#define ITERATIONS      100
#define N_LAUNCHES      1

// -- Bound check helper -- (This may also be avoided?)
static void checkBounds(Vector* pos, double bound) {
    if (pos == NULL) return;
    for (unsigned int i = 0; i < pos->d; i++) {
        if (pos->data[i] > bound) {
            pos->data[i] = bound;
        } else if (pos->data[i] < -bound) {
            pos->data[i] = -bound;
        }
    }
}

/* ------ MPI implementation of bat algorithm ------*/
/*
 * The algorithm is based on different steps:
 * 1) initialization of the following resources: 
 *      - one matrix for keeping track of the positions (batPos)
 *      - one matrix for keeping track of the velocities (batVel)
 *      - vectors for batPulse, batLoudness, batFitness for each bat
 *      - avgLoudness is used when evaluating the average fitness
 *      - best index, best fitness are used for keeping track of the best results
 * 2) generation of initial values (batPos, batVel, batFitness)
 * 3) Algorithm iterations:
 *      1) estimation of avgLoudness and bestFitness using MPI reductions
 *      2) generation of new frequency, position and velocity (global and local search)
 *      3) new fitness evaluation 
 * 4) deallocation of structures
 */
void batAlgorithmMPI(batAlgorithmParameters* parameters, batAlgorithmResults* results, ObjectiveFn f, unsigned int mpiId, unsigned int mpiProc, unsigned int totalBats) {

    if (parameters == NULL || f == NULL || results == NULL) return;
    
    unsigned int localBats = parameters->bats;
    unsigned int vectorDim = parameters->vectorDim;
    
    // Generation of local seed for random numbers
    ugSeed* randomSeed = NULL;
    generateSeed(&randomSeed, mpiId);
    
    // Initialization of bat data structures
    Matrix* batPos = NULL;
    Matrix* batVel = NULL;
    Vector* batPulse = NULL;
    Vector* batLoudness = NULL;
    Vector* batFitness = NULL;
    
    initMatrix(&batPos, localBats, vectorDim);
    initMatrix(&batVel, localBats, vectorDim);
    initVector(&batPulse, localBats);
    initVector(&batLoudness, localBats);
    initVector(&batFitness, localBats);

    // Check if allocation failed or not for ALL processes
    int localAllocFailed = (batPos == NULL || batVel == NULL || batPulse == NULL || 
        batLoudness == NULL || batFitness == NULL) ? 1 : 0;
    
    int globalAllocFailed = 0;

    // Check if any other process has failed
    MPI_Allreduce(&localAllocFailed, &globalAllocFailed, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    
    if (globalAllocFailed) {
        if (mpiId == 0) fprintf(stderr, "Error: Failed to allocate bat structures on one or more processes.\n");
        // Cleanup any partial allocation
        destroyMatrix(&batPos);
        destroyMatrix(&batVel);
        destroyVector(&batPulse);
        destroyVector(&batLoudness);
        destroyVector(&batFitness);
        destroySeed(&randomSeed);
        return;
    }
    
    // Temporary vectors for computations
    Vector* tmpPos = NULL;
    Vector* candPos = NULL;
    initVector(&tmpPos, vectorDim);
    initVector(&candPos, vectorDim);
    
    // Global best tracking (replicated on all processes)
    Vector* globalBestPos = NULL;
    initVector(&globalBestPos, vectorDim);
    double globalBestFitness = INFINITY;
    
    // Local best tracking
    int localBestIndex = 0;
    double localBestFitness = INFINITY;
    
    // Initialize local bats with random positions and velocities
    for (unsigned int i = 0; i < localBats; i++) {
        // Random position around initPos
        for (unsigned int d = 0; d < vectorDim; d++) {
            batPos->data[i][d] = randomUniformRadius(parameters->initPos->data[d], 
                                                      parameters->initPosRadius, randomSeed);
            batVel->data[i][d] = 0.0;
        }
        
        // Initialize pulse and loudness
        batPulse->data[i] = parameters->initPulse;
        batLoudness->data[i] = parameters->initLoudness;
        
        // Evaluate initial fitness
        copyToVector(batPos, tmpPos, i);
        batFitness->data[i] = objective_eval(f, tmpPos);
        
        // Track local best
        if (batFitness->data[i] < localBestFitness) {
            localBestFitness = batFitness->data[i];
            localBestIndex = i;
        }
    }
    
    // Structure for MPI_MINLOC reduction
    struct {
        double fitness;
        int rank;
    } localMin, globalMin;
    
    localMin.fitness = localBestFitness;
    localMin.rank = mpiId;
    
    // Find global best across all processes
    MPI_Allreduce(&localMin, &globalMin, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
    globalBestFitness = globalMin.fitness;
    
    // Broadcast best position from the process that has it
    if (mpiId == (unsigned int)globalMin.rank) {
        copyToVector(batPos, globalBestPos, localBestIndex);
    }
    MPI_Bcast(globalBestPos->data, vectorDim, MPI_DOUBLE, globalMin.rank, MPI_COMM_WORLD);
    
    // Main optimization loop
    for (unsigned int t = 0; t < parameters->iterations; t++) {
        
        // Compute local sum of loudness
        double localSumLoudness = 0.0;
        for (unsigned int i = 0; i < localBats; i++) {
            localSumLoudness += batLoudness->data[i];
        }
        
        // Compute global average loudness
        double globalSumLoudness = 0.0;
        MPI_Allreduce(&localSumLoudness, &globalSumLoudness, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        double avgLoudness = globalSumLoudness / (double)totalBats;
        
        // Update each local bat
        for (unsigned int i = 0; i < localBats; i++) {
            // Copy current position to candidate
            copyToVector(batPos, candPos, i);
            
            // GLOBAL SEARCH: Update frequency and velocity
            double newFreq = parameters->fMin + (parameters->fMax - parameters->fMin) * randomUniform(0, 1, randomSeed);
            
            for (unsigned int d = 0; d < vectorDim; d++) {
                double delta = batPos->data[i][d] - globalBestPos->data[d];
                batVel->data[i][d] += delta * newFreq;
                candPos->data[d] = batPos->data[i][d] + batVel->data[i][d];
            }
            
            // Apply boundary conditions
            checkBounds(candPos, parameters->initPosRadius);
            
            // LOCAL SEARCH (Random Walk)
            // If random > pulse rate, perform local search
            if (randomUniform(0, 1, randomSeed) > batPulse->data[i]) {
                for (unsigned int d = 0; d < vectorDim; d++) {
                    // The following assignment is slightly different from the original paper. In the paper
                    // the assignment would be something like candPos = bestBatPos + random * avgLoudness
                    // but this means a non trivial amount of bats at start (when pulse)
                    candPos->data[d] = batPos->data[i][d] + randomUniform(-1, 1, randomSeed) * avgLoudness;
                }
                // Apply boundary conditions
                checkBounds(candPos, parameters->initPosRadius);
            }
            
            // EVALUATION AND ACCEPTANCE
            double newFitness = objective_eval(f, candPos);
            
            // Accept the new solution if it's better AND random < loudness
            if (newFitness < batFitness->data[i] && randomUniform(0, 1, randomSeed) < batLoudness->data[i]) {
                // Update position
                copyToMatrix(candPos, batPos, i);
                batFitness->data[i] = newFitness;
                
                // Update loudness and pulse rate
                batLoudness->data[i] = parameters->alpha * batLoudness->data[i];
                batPulse->data[i] = parameters->initPulse * (1.0 - exp(-(parameters->gamma) * (double)t));
            }
        }
        
        // Update local best
        localBestFitness = INFINITY;
        for (unsigned int i = 0; i < localBats; i++) {
            if (batFitness->data[i] < localBestFitness) {
                localBestFitness = batFitness->data[i];
                localBestIndex = i;
            }
        }
        
        localMin.fitness = localBestFitness;
        localMin.rank = mpiId;
        
        // Find global best across all processes
        MPI_Allreduce(&localMin, &globalMin, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
        globalBestFitness = globalMin.fitness;
        
        // Broadcast global best position
        if (mpiId == (unsigned int)globalMin.rank) {
            copyToVector(batPos, globalBestPos, localBestIndex);
        }
        MPI_Bcast(globalBestPos->data, vectorDim, MPI_DOUBLE, globalMin.rank, MPI_COMM_WORLD);
    }
    
    // Store results
    results->bestFitness = globalBestFitness;
    results->bestIndex = (unsigned int)globalMin.rank * localBats + localBestIndex;
    copyVector(globalBestPos, results->bestPos);
    
    // Cleanup
    destroyMatrix(&batPos);
    destroyMatrix(&batVel);
    destroyVector(&batPulse);
    destroyVector(&batLoudness);
    destroyVector(&batFitness);
    destroyVector(&tmpPos);
    destroyVector(&candPos);
    destroyVector(&globalBestPos);
    destroySeed(&randomSeed);
}

int main(int argc, char** argv) {
    // mpiProc and mpiId represent number of spawned processes, and rank of spawned process
    int mpiProc, mpiId;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiProc);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiId);
    
    // Parse CLI arguments early to get dimension before allocation
    int dim = VECTOR_DIM; // default
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dim") == 0 && i + 1 < argc) {
            dim = atoi(argv[++i]);
            break;
        }
    }
    
    // Initialize parameters using library structures with proper dimension
    batAlgorithmParameters* parameters = NULL;
    initParameters(&parameters, dim);
    
    // Set initial position
    for (unsigned int i = 0; i < dim; i++) {
        parameters->initPos->data[i] = 0.0;
    }
    
    // Parse CLI Arguments 
    ObjectiveFn function=NULL; 
    parseArguments(argc, argv, parameters,&function);

    // Calculate actual total bats (accounts for integer division)
    unsigned int actualTotalBats = parameters->bats * mpiProc;
    
    if (mpiId == 0) {
        printf("Bat algorithm launch with MPI processes=%d, bats per process=%d\n", mpiProc, parameters->bats);
        printParameters(parameters);
    }
    
    // Initialize results structure
    batAlgorithmResults* results = NULL;
    initResults(&results, VECTOR_DIM);
    
    double start, end;
    double totalTime = 0;
    
    for (unsigned int i = 0; i < N_LAUNCHES; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        
        batAlgorithmMPI(parameters, results, function, mpiId, mpiProc, actualTotalBats);
        
        MPI_Barrier(MPI_COMM_WORLD);
        end = MPI_Wtime();
        totalTime += end - start;
        
        if (mpiId == 0) {
            printf("Iteration %d took %f s\n", i, end - start);
            printResults(results);
            
            // Print distance from known minimum for Rosenbrock
            printf("Rosenbrock minima is at: (1, ..., 1) with a value of 0\n");
            printf("Distance (component-wise): [");
            for (unsigned int d = 0; d < results->bestPos->d; d++) {
                printf("%f", results->bestPos->data[d] - 1.0);
                if (d < results->bestPos->d - 1) printf(", ");
            }
            printf("]\n\n");
        }
    }
    
    if (mpiId == 0) {
        double avgTime = (double) (totalTime / (double) N_LAUNCHES);
        printf("Average execution time: %f s\n", avgTime);

        /* Print machine readable output for parameter search*/
        unsigned int originalBats = parameters->bats;
        parameters->bats = actualTotalBats;
        printBenchmarkData(results, parameters, avgTime);
        parameters->bats = originalBats;
    }
    
    destroyResults(&results);
    destroyParameters(&parameters);
    MPI_Finalize();
    return 0;
}
