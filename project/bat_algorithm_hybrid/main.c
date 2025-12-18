#include <data.h>
#include <tools.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <sys/time.h>
#include <time.h>

#define M_PI 3.14159265358979323846

#define FMAX            100
#define FMIN            0
#define PULSE           0.9
#define LOUDNESS        100
#define GAMMA           0.9
#define ALPHA           0.9
#define VECTORDIM       2
#define INITPOSRADIUS   1000

#define OMPTHREADS      200
#define ITERATIONS      200
#define NLAUNCHS        10

typedef enum {false,true} bool;

/* ------ objective functions ------*/

/* Hypersphere (minimum on [0,...,0] */
double objectiveFunction(Vector* pos) {
    if (pos==NULL) return -1;
    double s=0;
    for (unsigned int i=0;i<pos->d;i++) {
        s+=pow(pos->data[i],2);
    }
    return s;
}

/* Rosenbrock Nd (minimum on [1,...,1]) */
/*
double objectiveFunction(Vector* pos) {
    if (pos==NULL) return -1;
    
    double s=0;
    for (unsigned int i=0;i<(pos->d)-1;i++) {
        s+=(double)pow((1.0-pow(pos->data[i],2)),2);
        s+=(double)(100.0)*pow(((pos->data[i+1])-pow(pos->data[i],2)),2);
    }

    return s;
}*/

/* ----- functions for random number generationi in openMP ----- */
double randDoubleRadius(double pos, double radius, unsigned int* seed) {
    return (pos-radius) + (2*radius) * ((double) rand_r(seed) / RAND_MAX);
}
double randDouble(double min, double max, unsigned int* seed) {
    return (min)+(max-min) * ((double)rand_r(seed)/RAND_MAX);
}

unsigned int generateSeed(unsigned int thread) {
    return clock()+(thread*1000u);
}


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
void batAlgorithmOMP(batAlgorithmParameters* parameters, batAlgorithmResults* results, double (*objFunction)(Vector*)) {
    if (parameters==NULL || objFunction==NULL || results==NULL) return;
    
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
        unsigned int randomSeed=generateSeed(thread);

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
                tmpPos->data[i]=randDoubleRadius(parameters->initPos->data[i],parameters->initPosRadius,&randomSeed);
            }
            
            copyToMatrix(tmpPos,batPos,thread);
            batFitness=objFunction(tmpPos);

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
                double newFreq = (parameters->fMin) + ((parameters->fMax)-(parameters->fMin) * randDouble(0,1,&randomSeed));
                for (unsigned int i=0;i<parameters->vectorDim;i++) {
                    double bestPos=batPos->data[bestIndex][i];
                    
                    batVel->data[i]=batVel->data[i] + ((batPos->data[thread][i]-bestPos)*newFreq);
                    tmpPos->data[i]=batPos->data[thread][i]+batVel->data[i];
                }

                //Checks if the bat should randmly perform a local search
                if (randDouble(0,parameters->initPulse,&randomSeed)>batPulse) {
                    //The local search is based on the average loudness 
                    for (unsigned int i=0;i<parameters->vectorDim;i++) {
                        tmpPos->data[i]=batPos->data[thread][i]+(randDouble(-1,1,&randomSeed)*avgLoudness);
                    }    
                } 
                
                //checks new fitness 
                batFitness=objFunction(tmpPos);

                if (randDouble(0,parameters->initLoudness,&randomSeed)>batLoudness && batFitness<bestFitness) {
                    //If the new position gives a better fitness value, loudness and pulse are updated. 
                    batLoudness=(parameters->alpha)*batLoudness;
                    batPulse=(parameters->initPulse)*(1.0-exp((-(parameters->gamma)*(double)(t))));
                }
                copyToMatrix(tmpPos,batPos,thread);

            }
            
            destroyVector(&tmpPos);
        }
        //results are saved before the end of the function
        results->bestFitness=bestFitness;
        results->bestIndex=bestIndex;
        copyToVector(batPos,results->bestPos,bestIndex);
    }
    destroyMatrix(&batPos);
}

/* ------ MPI section ------ */
/* The openMP algorithm is launched on more MPI processes that spawns the bats in different positions based on the divideRegion function.
 * At the end of the executions, the returned value from openMP are then compared to find the best global fitness, which is considered as the best result obtained.

 * The region division is based on the fibonacci spiral. In general we consider a region specified with center P and radius R and whe divide in smaller subregions specified with localP and localR:
 * - The radius r that denotes the evolution of the fibonacci spiral is obtained using the formula "radius * sqrt((rank+0.5)/proc)"
 * - The new position is obtained by travelling around the center in a spiral movement denoted by the angle theta (product of the golden angle and the rank) and the radius r
 *
 * The localCenter is given by the projections sin and cos of the radius r respect the position theta shifted by the initial position.
 * The resulting localRadius is given by the formula "(radius)/sqrt(proc)" (much smaller than the original)
*/

void divideRegion(Vector* pos, double radius, Vector* newPos, double* newRadius, unsigned int proc, unsigned int rank) {
    double phi=(sqrt(5.0)+1.0)/2.0; 
    double theta = 2.0 * M_PI * (1.0 - (1.0 / phi))*rank;
    double r = radius * sqrt ((rank + 0.5)/((double)proc));

    newPos->data[0]=pos->data[0] + r * cos(theta);
    newPos->data[1]=pos->data[1] + r * sin(theta);
    *newRadius= (radius) / sqrt(proc);
}

/*
 * Each MPI process generates its local position using the divideRegion function and then executes the openMP bat algorithm. 
 * After the execution, the best fitness its relative process are reduces using a MINLOC broadcast. 
 * The best results are then sent to process 0, which return the function with valid values in the result structure.
 */

void batAlgorithmMPI3D(batAlgorithmParameters* parameters, batAlgorithmResults* results, double (*objFunction)(Vector*),unsigned int mpiId, unsigned int mpiProc, void* mpiBuffer, unsigned int bufferDim) {
    
    //Generation of local parameters
    batAlgorithmParameters* localParameters=NULL;
    initParameters(&localParameters,2);
    
    localParameters->fMin=parameters->fMin;
    localParameters->fMax=parameters->fMax;
    localParameters->initPulse=parameters->initPulse;
    localParameters->initLoudness=parameters->initLoudness;
    localParameters->gamma=parameters->gamma;
    localParameters->alpha=parameters->alpha;
    localParameters->vectorDim=parameters->vectorDim;
    localParameters->bats=parameters->bats;
    localParameters->iterations=parameters->iterations;

    divideRegion(parameters->initPos,parameters->initPosRadius,localParameters->initPos,&(localParameters->initPosRadius),mpiProc,mpiId);
    
    //Execution of the openMP algorithm
    batAlgorithmOMP(localParameters,results,objectiveFunction);
    
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

    //The local parameters structure is then destroyed and the process 0 returns the best results in the result structure. 
    destroyParameters(&localParameters);
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
    parameters->bats=OMPTHREADS;
    parameters->iterations=ITERATIONS;
   


    if (mpiId==0) {
        printf("Bat algorithm launch with MPI processes=%d and openMP processes=%d\n",mpiProc,50);
        printParameters(parameters);
    }

    batAlgorithmResults* results=NULL;
    initResults(&results,2);
    
    unsigned int resultsMPIDim=0;
    int dim;
    MPI_Pack_size(VECTORDIM,MPI_DOUBLE,MPI_COMM_WORLD,&dim);
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

        batAlgorithmMPI3D(parameters,results,objectiveFunction,mpiId,mpiProc,mpiBuffer,resultsMPIDim);
        
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

