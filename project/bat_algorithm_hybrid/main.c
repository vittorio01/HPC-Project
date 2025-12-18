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

#define FMAX            2
#define FMIN            0
#define PULSE           0.5
#define LOUDNESS        0.1
#define GAMMA           0.9
#define ALPHA           0.9
#define VECTORDIM       2
#define INITPOSRADIUS   1000

#define OMPTHREADS      1000
#define ITERATIONS      100
#define NLAUNCHS        10

typedef enum {false,true} bool;


double objectiveFunction(Vector* pos) {
    double s=0;
    for (unsigned int i=0;i<pos->d;i++) {
        s+=pow(pos->data[i],2);
    }
    return s;
}

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

double randDoubleRadius(double pos, double radius, unsigned int* seed) {
    return (pos-radius) + (2*radius) * ((double) rand_r(seed) / RAND_MAX);
}
double randDouble(double min, double max, unsigned int* seed) {
    return (min)+(max-min) * ((double)rand_r(seed)/RAND_MAX);
}


unsigned int generateSeed(unsigned int thread) {
    return clock()+(thread*1000u);
}
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
        //---  openMP thead ---
    #pragma omp parallel num_threads(parameters->bats) 
    {
        unsigned int thread=omp_get_thread_num();

        //Initialization of parameters
        Vector* batVel=NULL;
        double batPulse=parameters->initPulse;
        double batLoudness=parameters->initLoudness;
        double batFitness;

        unsigned int randomSeed=generateSeed(thread);

        initVector(&batVel,parameters->vectorDim);
        
        //initialization of vectorrs used during loop 
        Vector* tmpPos=NULL;

        initVector(&tmpPos,parameters->vectorDim);
        //checking if the initialization is done for all threads
        #pragma omp critical 
        {
            if (initialized==true) {
                if (batVel==NULL || tmpPos==NULL) {
                    initialized=false;
                }
            }
        } 
        #pragma omp barrier 
        if (initialized==true) {

            //initialization of positions/velocities and fitness
            initVectorData(batVel,0);
            for (unsigned int i=0;i<parameters->vectorDim;i++) {
                tmpPos->data[i]=randDoubleRadius(parameters->initPos->data[i],parameters->initPosRadius,&randomSeed);
            }
            
            copyToMatrix(tmpPos,batPos,thread);
            batFitness=objFunction(tmpPos);
            for (unsigned int t=0;t<parameters->iterations;t++) {
                                
                //Average Loudness calculation
                if (thread==0) {
                    avgLoudness=0;
                }
                #pragma omp barrier

                #pragma omp critical 
                {
                    avgLoudness+=batLoudness;
                    if (batFitness<bestFitness) {
                        bestFitness=batFitness;
                        bestIndex=thread;
                    }
                    
                }
                #pragma omp barrier
                if (thread==0) {
                    avgLoudness=(double)(avgLoudness/((double)parameters->bats));
                }
                #pragma omp barrier
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
                
                batFitness=objFunction(tmpPos);

                if (randDouble(0,parameters->initLoudness,&randomSeed)>batLoudness && batFitness<bestFitness) {
                    //If the new position gives a better fitness value, position,velocity, loudness and pulse are updated. 
                    batLoudness=(parameters->alpha)*batLoudness;
                    batPulse=(parameters->initPulse)*(1.0-exp((-(parameters->gamma)*(double)(t))));
                }
                copyToMatrix(tmpPos,batPos,thread);

            }
            
            destroyVector(&tmpPos);
        }
        results->bestFitness=bestFitness;
        results->bestIndex=bestIndex;
        copyToVector(batPos,results->bestPos,bestIndex);
    }
    destroyMatrix(&batPos);
}


void divideRegion(Vector* pos, double radius, Vector* newPos, double* newRadius, unsigned int proc, unsigned int rank) {
    double phi=(sqrt(5.0)+1.0)/2.0; 
    
    *newRadius=radius*sqrt((rank+0.5)/proc);
    double theta = 2.0 * M_PI * rank / phi;
    newPos->data[0]=pos->data[0] + (*newRadius) * cos(theta);
    newPos->data[1]=pos->data[1] + (*newRadius) * sin(theta);
}

void batAlgorithmMPI3D(batAlgorithmParameters* parameters, batAlgorithmResults* results, double (*objFunction)(Vector*),unsigned int mpiId, unsigned int mpiProc, void* mpiBuffer, unsigned int bufferDim) {
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
    
    batAlgorithmOMP(localParameters,results,objectiveFunction);
    
    struct {
        double fitness;
        int id;
    } local, global;

    local.fitness = results->bestFitness;
    local.id = mpiId;
    MPI_Allreduce(&local,&global,1,MPI_DOUBLE_INT,MPI_MINLOC,MPI_COMM_WORLD);

    if (mpiId==0) {
        if (global.id!=mpiId) {
            int pos=0;
            MPI_Recv(mpiBuffer,bufferDim,MPI_PACKED,global.id,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Unpack(mpiBuffer, 3, &pos,results->bestPos->data,2,MPI_DOUBLE,MPI_COMM_WORLD);
            MPI_Unpack(mpiBuffer, 1, &pos,&(results->bestFitness),1,MPI_DOUBLE,MPI_COMM_WORLD);
            MPI_Unpack(mpiBuffer, 1, &pos,&(results->bestIndex),1,MPI_UNSIGNED,MPI_COMM_WORLD);
        } 
    } else {
        if (global.id==mpiId) {
            int pos=0;
            MPI_Pack(results->bestPos->data,2,MPI_DOUBLE,mpiBuffer,bufferDim,&pos,MPI_COMM_WORLD);
            MPI_Pack(&(results->bestFitness),1,MPI_DOUBLE,mpiBuffer,bufferDim,&pos,MPI_COMM_WORLD);
            MPI_Pack(&(results->bestIndex),1,MPI_UNSIGNED,mpiBuffer,bufferDim,&pos,MPI_COMM_WORLD);
            MPI_Send(mpiBuffer,bufferDim,MPI_PACKED,0,0,MPI_COMM_WORLD);
        }
    }
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
        printf("Bat algorithm launch with MPI processes=%d and openMP processes =%d\n",mpiProc,50);
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
    double elapsedTime=0;
    for (unsigned int i=0;i<NLAUNCHS;i++) {
        start=MPI_Wtime();

        batAlgorithmMPI3D(parameters,results,objectiveFunction,mpiId,mpiProc,mpiBuffer,resultsMPIDim);
        
        end=MPI_Wtime();
        totalTime+=end-start;
        if (mpiId==0) {
            printf("Iteration %d took %f\n",i,elapsedTime);
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

