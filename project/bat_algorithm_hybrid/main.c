#include <data.h>
#include <tools.h>
#include <benchmark.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <sys/time.h>
#include <time.h>


#define M_PI 3.14159265358979323846

#define FMAX            2.0
#define FMIN            0
#define PULSE           0.5
#define LOUDNESS        1.0
#define GAMMA           0.9
#define ALPHA           0.9
#define VECTORDIM       2
#define INITPOSRADIUS   500

#define BATS            1000
#define ITERATIONS      500
#define NLAUNCHS        10

typedef enum {false,true} bool;

/* ------ openMP implemetation of bat algorithm ------*/
/*
 * The algorithm is based on differen steps:
 * 1) initialization of the following resources: 
 *      - one matrix for keeping track of the positions (batPos)
 *      - avgLoudness is used as accumulator variable when evaluating the average fitness
 *      - best index, best fitness are used for keeping track of the best results obtained during iterations
 * 2) initialization of the threads and their local variables batVel,batPulse,batLoudness and batFitness used during the iterations. 
 * 3) generation of initial values (batPos and batFitness)
 * 4) Algorithm iterations:
 *      1) estimation of avgLoudness and bestFitness using openMP barriers and critical sections
 *      2) generation of new frequency, position and velocity (local and global case)
 *      3) new fitness evaluation 
 * 5) deallocation of local and shared structures
 */
void batAlgorithmOMP(batAlgorithmParameters* parameters, batAlgorithmResults* results, ObjectiveFn f) {
    if (parameters==NULL || f==NULL || results==NULL) return;
    
    //initialization of shared parameters used during the algorithm

    Matrix* batPos=NULL;
    initMatrix(&batPos,parameters->bats,parameters->vectorDim);
    if (batPos==NULL) return;
    
    double avgLoudness=0;    
    double bestFitness=1.0e30f;
    unsigned int bestIndex=0;
    bool initialized=true;

    //generation of threads
    #pragma omp parallel num_threads(parameters->bats) 
    {
        unsigned int thread=omp_get_thread_num();

        //Initialization of local parameters
        Vector* batVel=NULL;
        double batPulse=parameters->initPulse;
        double batLoudness=parameters->initLoudness;
        double batFitness;
        
        //Generation of local seed for random numbers
        ugSeed* randomSeed=NULL; 
        generateSeed(&randomSeed,thread);

        initVector(&batVel,parameters->vectorDim);
        
        //initialization of vectorrs used during loop 
        Vector* tmpPos=NULL;

        initVector(&tmpPos,parameters->vectorDim);
        //checking if the initialization of all batVel vectors are done for all threads
        #pragma omp critical 
        {
            if (initialized==true) {
                if (batVel==NULL || tmpPos==NULL) {
                    initialized=false;
                }
            }
        } 
        #pragma omp barrier 
        //If the initialization is done correctly, the algorithm can start
        if (initialized==true) {

            //generation of the initial parameters (positions,velocities and fitness)
            initVectorData(batVel,0);
            for (unsigned int i=0;i<parameters->vectorDim;i++) {
                tmpPos->data[i]=randomUniformRadius(parameters->initPos->data[i],parameters->initPosRadius,randomSeed);
            }
            
            copyToMatrix(tmpPos,batPos,thread);
            batFitness=objective_eval(f,tmpPos);

            //Algorithm iterations
            for (unsigned int t=0;t<parameters->iterations;t++) {
                                
                //Average Loudness and bestFitness estimation (to be further optimized because still slow)

                if (thread==0) {
                    //only thread 0 sets the avgLoudness as 0
                    avgLoudness=0;
                }
                #pragma omp barrier

                #pragma omp critical 
                {
                    //Threds performs the reduction in a sequential way
                    avgLoudness+=batLoudness; 
                    if (batFitness<bestFitness) {
                        bestFitness=batFitness;
                        bestIndex=thread;
                    }
                    
                }
                #pragma omp barrier
                if (thread==0) {
                    //thread 0 performs the loudness calculation
                    avgLoudness=(double)(avgLoudness/((double)parameters->bats));
                }
                #pragma omp barrier

                //newFreq, batVel and batPos update
                double newFreq = (parameters->fMin) + ((parameters->fMax)-(parameters->fMin) * randomUniform(0,1,randomSeed));
                for (unsigned int i=0;i<parameters->vectorDim;i++) {
                    double bestPos=batPos->data[bestIndex][i];
                    
                    batVel->data[i]=batVel->data[i] + ((batPos->data[thread][i]-bestPos)*newFreq);
                    tmpPos->data[i]=batPos->data[thread][i]+batVel->data[i];
                }

                //Checks if the bat should randmly perform a local search
                if (randomUniform(0,parameters->initPulse,randomSeed)>batPulse) {
                    //The local search is based on the average loudness 
                    for (unsigned int i=0;i<parameters->vectorDim;i++) {
                        tmpPos->data[i]=batPos->data[thread][i]+(randomUniform(-1,1,randomSeed)*avgLoudness);
                    }    
                } 
                
                //checks new fitness 
                batFitness=objective_eval(f,tmpPos);

                if (randomUniform(0,parameters->initLoudness,randomSeed)>batLoudness && batFitness<bestFitness) {
                    //If the new position gives a better fitness value, loudness and pulse are updated. 
                    batLoudness=(parameters->alpha)*batLoudness;
                    batPulse=(parameters->initPulse)*(1.0-exp((-(parameters->gamma)*(double)(t))));
                }
                copyToMatrix(tmpPos,batPos,thread);

            }
            
            destroyVector(&tmpPos);
            destroyVector(&batVel);
            destroySeed(&randomSeed);
        }
        //results are saved before the end of the function
        results->bestFitness=bestFitness;
        results->bestIndex=bestIndex;
        copyToVector(batPos,results->bestPos,bestIndex);
    }
    destroyMatrix(&batPos);
}

/* ------ MPI section ------ */

/*
 * Each MPI process generates its local position using the divideRegion function and then executes the openMP bat algorithm. 
 * After the execution, the best fitness its relative process are reduces using a MINLOC broadcast. 
 * The best results are then sent to process 0, which return the function with valid values in the result structure.
 */

void batAlgorithmMPI3D(batAlgorithmParameters* parameters, batAlgorithmResults* results, ObjectiveFn f,unsigned int mpiId, unsigned int mpiProc, void* mpiBuffer, unsigned int bufferDim) {
    
    //Execution of the openMP algorithm
    batAlgorithmOMP(parameters,results,f);
    
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
            int pos=0;
            MPI_Recv(mpiBuffer,bufferDim,MPI_PACKED,global.id,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Unpack(mpiBuffer, bufferDim, &pos,results->bestPos->data,parameters->vectorDim,MPI_DOUBLE,MPI_COMM_WORLD);
            MPI_Unpack(mpiBuffer, bufferDim, &pos,&(results->bestFitness),1,MPI_DOUBLE,MPI_COMM_WORLD);
            MPI_Unpack(mpiBuffer, bufferDim, &pos,&(results->bestIndex),1,MPI_UNSIGNED,MPI_COMM_WORLD);
        } 
    } else {
        if (global.id==mpiId) {

            //the best process sends its results by packing the values  
            int pos=0;
            MPI_Pack(results->bestPos->data,parameters->vectorDim,MPI_DOUBLE,mpiBuffer,bufferDim,&pos,MPI_COMM_WORLD);
            MPI_Pack(&(results->bestFitness),1,MPI_DOUBLE,mpiBuffer,bufferDim,&pos,MPI_COMM_WORLD);
            MPI_Pack(&(results->bestIndex),1,MPI_UNSIGNED,mpiBuffer,bufferDim,&pos,MPI_COMM_WORLD);
            MPI_Send(mpiBuffer,bufferDim,MPI_PACKED,0,0,MPI_COMM_WORLD);
        }
    }

}

int main(int argc, char** argv) {
    int mpiProc, mpiId;
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&mpiProc);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpiId);
  
    batAlgorithmParameters* parameters=NULL;
    initParameters(&parameters,2);
    for (unsigned int i=0;i<3;i++) {
        parameters->initPos->data[i]=30;
    }
    parameters->fMin=FMIN;
    parameters->fMax=FMAX;
    parameters->initPulse=PULSE;
    parameters->initLoudness=LOUDNESS;
    parameters->gamma=GAMMA;
    parameters->alpha=ALPHA;
    parameters->vectorDim=2;
    parameters->initPosRadius=INITPOSRADIUS;
    parameters->bats=BATS/mpiProc;
    parameters->iterations=ITERATIONS;
   


    if (mpiId==0) {
        printf("Bat algorithm launch with MPI processes=%d and openMP processes=%d\n",mpiProc,parameters->bats);
        printParameters(parameters);
    }

    batAlgorithmResults* results=NULL;
    initResults(&results,2);
    
    unsigned int resultsMPIDim=0;
    int dim;
    MPI_Pack_size(VECTOR_DIM,MPI_DOUBLE,MPI_COMM_WORLD,&dim);
    resultsMPIDim+=dim;
    MPI_Pack_size(1,MPI_DOUBLE,MPI_COMM_WORLD,&dim);
    resultsMPIDim+=dim;
    MPI_Pack_size(1,MPI_UNSIGNED,MPI_COMM_WORLD,&dim);
    resultsMPIDim+=dim;
    void* mpiBuffer=NULL;
    mpiBuffer=malloc(resultsMPIDim);

    double start,end;
    double totalTime=0;
    for (unsigned int i=0;i<NLAUNCHS;i++) {
        start=MPI_Wtime();

        batAlgorithmMPI3D(parameters,results,sphere,mpiId,mpiProc,mpiBuffer,resultsMPIDim);
        
        end=MPI_Wtime();
        totalTime+=end-start;
        if (mpiId==0) {
            printf("Iteration %d took %f\n",i,end-start);
            printResults(results);
        }       
    }
    if(mpiId==0) printf("Average execution time: %f\n",(double)(totalTime/(double )NLAUNCHS));

    destroyResults(&results);
    destroyParameters(&parameters);
    free(mpiBuffer);
    MPI_Finalize();
    return 0;
}

