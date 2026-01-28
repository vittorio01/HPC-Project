#include <data.h>
#include <tools.h>
#include <benchmark.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <sys/time.h>
#include <time.h>
#include "utils.h"

#define M_PI 3.14159265358979323846

#define FMAX            2.0
#define FMIN            0
#define PULSE           0.5
#define LOUDNESS        1.0
#define GAMMA           0.9
#define ALPHA           0.9
#define VECTORDIM       2
#define INITPOSRADIUS   500

#define BATS                        10000
#define ITERATIONS                  50
#define NLAUNCHS                    10

#define BATS_PER_THREAD             100

/* ------ openMP implemetation of bat algorithm ------*/
/*
 * The set of bats assigned to an MPI process is divided again in a number of openMP threads relative to the target average number of bats per thread (BATS_PER_THREAD). Each thread performs the following steps:
 * 1) initialization of the following resources: 
 *      - two matrices for keeping track of the positions (batPos) and the velocities. 
 *      - Some vectors for maintaining other variables like current pulse, current loudness and current fitness for each bat.
 *      - avgLoudness is used as accumulator variable when evaluating the average fitness
 *      - best index, best fitness are used for keeping track of the best results obtained during iterations
 * 2) initialization of the vectors and matrices with their respective values (random initial position for batpos). and evaluation of the initial best position.
 * 3) Algorithm iterations:
 *      1) estimation of avgLoudness
 *      2) generation of new frequency, position and velocity (local and global case) for each bat. 
 *      3) new best fitness evaluation 
 * At the end of the algorithm, the threads are synchronized using a barrier and the results are evaluated and the best values are saved in the result structure. 
 * 5) deallocation of the dynamic resouces.
 */

void batAlgorithmOMP(batAlgorithmParameters* parameters, batAlgorithmResults* results, ObjectiveFn f, unsigned int mpiId, unsigned int batsToProcess) {
    if (parameters==NULL || f==NULL || results==NULL) return;
    
    //This variable is used in order to verify that the dynamic allocation works for each threads in the process. 
    bool runtimeError=false;

    //Variables for computing the best results between threads. 
    int bestThread=-1;
    double bestThreadFitness=1.0e300;
    
    //The number of threads to launch is obtained by dividing the number of bats to process by the number of target bats per thread. 
    //The result is then adjusted in case the divisio generates a remaindes (ceiling division).
    unsigned int threadsNum=intRequiredThreads(batsToProcess,BATS_PER_THREAD);
    #pragma omp parallel num_threads(threadsNum) 
    {
        unsigned int threadId=omp_get_thread_num();

        //The number of actual bats to process per thread is obtained by performing the ceiling division on the number of threads atually launched. 
        unsigned int batsPerThread=intCeil(batsToProcess,threadsNum,threadId); 
        
        //Inizialization of useful vectors and matrices.
        Matrix *batPos = NULL;
        Matrix *batVel = NULL; 
        Vector *batFitness = NULL; 
        Vector *batLoudness = NULL;     
        Vector *batPulse = NULL; 
        Vector *currentBatPos = NULL; 
        Vector *bestPos = NULL;
        double bestFitness = 1.0e300; 
        unsigned int bestIndex = 0;

        initMatrix(&batPos, parameters->bats, parameters->vectorDim); 
        initMatrix(&batVel, parameters->bats, parameters->vectorDim); 
        initVector(&batFitness, parameters->bats);
        initVector(&batLoudness, parameters->bats);
        initVector(&batPulse, parameters->bats);
        initVector(&bestPos, parameters->vectorDim);
        initVector(&currentBatPos, parameters->vectorDim);
        
        ugSeed* randomSeed=NULL; 
        generateSeed(&randomSeed,mpiId+threadId);
        
        //The threads verify if all their resouces are actually allocated in the heap by changing the value of runtimeError in a critical section. 
        bool error=false; 
        if (randomSeed==NULL || batPos==NULL || batVel==NULL || batFitness == NULL || batLoudness==NULL || batPulse==NULL || bestPos==NULL) error=true;
        #pragma omp critical 
        {
            if (error==true) {
                runtimeError=true;
            }
        }
        #pragma omp barrier

        //If there are not errors the execution can continue 
        if (runtimeError==false) {
            
            //The matrices and the vectors are initialized.
            for (unsigned int batId = 0;batId<batsPerThread; batId++) {

                //Initialization of the initial positions using random functions. 
                for (unsigned int i=0;i<parameters->vectorDim;i++) {
                    currentBatPos->data[i]=randomUniformRadius(parameters->initPos->data[i],parameters->initPosRadius,randomSeed);
                }
                copyToMatrix(currentBatPos,batPos,batId); 

                //Initialization of the initial fitness, pulse and loudness values
                batFitness->data[batId]=objective_eval(f,currentBatPos);
                batLoudness->data[batId]=parameters->initLoudness; 
                batPulse->data[batId]=parameters->initPulse; 
                
                //Evaluation of the initial best position, fitness and index 
                if (batFitness->data[batId]<bestFitness) {
                    bestFitness=batFitness->data[batId];
                    copyVector(currentBatPos,bestPos);
                    bestIndex=batId;
                }
            }
            
            //Algorithm section 
            for (unsigned int i=0;i<parameters->iterations; i++) {
                
                //Evaluation of the average loudness 
                double avgLoudness=0.0;
                for (unsigned int batId=0;batId<batsPerThread;batId++) {
                    avgLoudness+=batLoudness->data[batId];
                }
                avgLoudness=(double) (avgLoudness/ (double) batsPerThread);

                //Iteration steps for each bat 
                for (unsigned int batId=0;batId<batsPerThread; batId++) {
                    double batFreq= parameters->fMin + (parameters->fMax - parameters->fMin) * randomUniform(0,1,randomSeed);
                    
                    //Generation of new position and velocity in normal cases 
                    for (unsigned int d=0;d<parameters->vectorDim;d++) {
                        batVel->data[batId][d] += (batPos->data[batId][d] - bestPos->data[d]) * batFreq;
                        currentBatPos->data[d] = batPos->data[batId][d] += batVel->data[batId][d];

                    }

                    //setting an alternative position for local search case. 
                    if (randomUniform(0,1,randomSeed) > batPulse->data[batId]) {
                        //double eps = randomUniform(-1,1,randomSeed);
                        for (unsigned int d=0 ;d <parameters->vectorDim; d++) {

                            //The bat assume a random position around the best one 
                            double eps = randomUniform(-1,1,randomSeed);
                            currentBatPos->data[d] = bestPos->data[d] + eps * avgLoudness;
                        }
                    }
                    copyToMatrix(currentBatPos,batPos,batId); 

                    //Evaluation of the new itness 
                    double newFitness=objective_eval(f,currentBatPos);

                    //If the new fitness is better, the bat parameters are updated 
                    if (newFitness <= batFitness->data[batId] && randomUniform(0,1,randomSeed)<batLoudness->data[batId]) {
                        batFitness->data[batId]=newFitness; 
                        batLoudness->data[batId] *= parameters->alpha; 
                        batPulse->data[batId] = parameters->initPulse * (1.0 * exp(-parameters->gamma * (double)(i))); 

                        //If the new solution is better than the global, the best parameters are updated. 
                        if (newFitness<bestFitness) {
                            bestFitness=newFitness; 
                            bestIndex=batId; 
                            copyVector(currentBatPos,bestPos);
                        }
                    }
                }
            }
            //At the end of the execution the threads synchronizes each other in order to find the best results.
            //Here the thrads updates the bestThread and bestThreadFitness one by one to discover the thread with the best results. 
            #pragma omp critical 
            {

                if (bestThread<0 || bestFitness<bestThreadFitness) {
                    bestThread=threadId;
                    bestThreadFitness=bestFitness;
                }
            } 
            #pragma omp barrier 

            //The thread with the best solution updates the result structures with its values. 
            if (threadId==bestThread) {
               copyVector(bestPos,results->bestPos);
               results->bestFitness=bestFitness;
               results->bestIndex=bestIndex;
            }
        }
        
        //Each thread destroys its structures. 
        destroyMatrix(&batPos);
        destroyMatrix(&batVel);
        destroyVector(&batFitness);
        destroyVector(&batLoudness);
        destroyVector(&batPulse);
        destroyVector(&bestPos);
        destroyVector(&currentBatPos);
        destroySeed(&randomSeed);
    }
}


/* ------ MPI section ------ */

/*
 * Each MPI process executes the openMP bat algorithm for their number of bats assigned with the ceiling division. 
 * After the execution, the best fitness its relative process are reduces using a MINLOC broadcast. 
 * The best results are then sent to process 0, which return the function with valid values in the result structure.
 */

void batAlgorithmMPI3D(batAlgorithmParameters* parameters, batAlgorithmResults* results, ObjectiveFn f,unsigned int mpiId, unsigned int mpiProc) {
    
    //The number of bats per OMP thread are obtained by ceiling.
    unsigned int localBats=intCeil(parameters->bats,mpiProc,mpiId);

    //Execution of the openMP algorithm
    batAlgorithmOMP(parameters,results,f, mpiId,localBats);

    //Estimation of the best fitness and its mpi process
    struct {
        double fitness;
        int id;
    } local, global;

    local.fitness = results->bestFitness;
    local.id = mpiId;
    MPI_Allreduce(&local,&global,1,MPI_DOUBLE_INT,MPI_MINLOC,MPI_COMM_WORLD);

    if (mpiId==0) {

        // The process 0 waits the best process to send its results and overwrite its result structure
        if (global.id!=mpiId) {
            MPI_Recv(results->bestPos->data,parameters->vectorDim,MPI_DOUBLE,global.id,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&(results->bestIndex),1,MPI_UNSIGNED,global.id,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            results->bestFitness=global.fitness; 
        } 

    } else {
        if (global.id==mpiId) {

            //the best process sends its results by packing the values  
            MPI_Send(results->bestPos->data,parameters->vectorDim,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
            MPI_Send(&(results->bestIndex),1,MPI_UNSIGNED,0,0,MPI_COMM_WORLD);
        }
    }

    //Final barrier to wait the end of the execution of all processes. 
    MPI_Barrier(MPI_COMM_WORLD);

}

int main(int argc, char** argv) {
    int mpiProc, mpiId;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&mpiProc);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpiId);
  
    // Parse CLI arguments early to get dimension before allocation
    int dim = 2; // default
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dim") == 0 && i + 1 < argc) {
            dim = atoi(argv[++i]);
            break;
        }
    }
    
    batAlgorithmParameters* parameters=NULL;
    initParameters(&parameters,dim);
    for (unsigned int i=0;i<dim;i++) {
        parameters->initPos->data[i]=30;
    }
    parameters->fMin=FMIN;
    parameters->fMax=FMAX;
    parameters->initPulse=PULSE;
    parameters->initLoudness=LOUDNESS;
    parameters->gamma=GAMMA;
    parameters->alpha=ALPHA;
    parameters->vectorDim=dim;
    parameters->initPosRadius=INITPOSRADIUS;
    parameters->bats=BATS;
    parameters->iterations=ITERATIONS;

    // Parse CLI arguments (if present)
    parseArguments(argc, argv, parameters);

    if (mpiId==0) {
        unsigned int batsPerProc=intCeil(parameters->bats,mpiProc,0);
        unsigned int threadsNum=intRequiredThreads(batsPerProc,BATS_PER_THREAD);

        printf("Bat algorithm launch with MPI processes=%d and openMP processes=%d\n",mpiProc,threadsNum);
        printParameters(parameters);
    }

    batAlgorithmResults* results=NULL;
    initResults(&results,2);
    
    double start,end;
    double totalTime=0;
    for (unsigned int i=0;i<NLAUNCHS;i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        
        start=MPI_Wtime();
        
        batAlgorithmMPI3D(parameters,results,rosenbrock,mpiId,mpiProc);
        
        end=MPI_Wtime();
        totalTime+=end-start;
        if (mpiId==0) {
            printf("Iteration %d took %f\n",i,end-start);
            printResults(results);
        }       
    }
    if(mpiId==0) {
        double avgTime = (double)(totalTime/(double)NLAUNCHS);
        printf("Average execution time: %f\n", avgTime);
        
        /* Print machine readable output */
        printBenchmarkData(results, parameters, avgTime);
    }
    destroyResults(&results);
    destroyParameters(&parameters);
    MPI_Finalize();
    return 0;
}

