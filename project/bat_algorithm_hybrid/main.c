#include <data.h>
#include <tools.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <sys/time.h>
#include <time.h>

#define NLAUNCHS    10

typedef enum {false,true} bool;

double objectiveFunction(Vector* pos) {
    if (pos==NULL) return -1;
    double s=0;
    for (unsigned int i=0;i<pos->d;i++) {
        s+=pow(pos->data[i],2);
    }
    return s;
}

double randDoubleRadius(double pos, double radius) {
    return (pos-radius) + (2*radius) * ((double) rand() / RAND_MAX);
}
double randDouble(double min, double max) {
    return (min)+(max-min) * ((double)rand()/RAND_MAX);
}

void batAlgorithmOMP(batAlgorithmParameters* parameters, batAlgorithmResults* results, double (*objFunction)(Vector*)) {
    if (parameters==NULL || objFunction==NULL || results==NULL) return;
    
    srand((unsigned int)time(NULL));
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
        initVector(&batVel,parameters->vectorDim);
        
        //initialization of vectors used during loop 
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
                batPos->data[thread][i]=randDoubleRadius(parameters->initPos->data[i],parameters->initPosRadius);
            }
            copyToVector(batPos,tmpPos,thread);
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
                    avgLoudness=avgLoudness/parameters->bats;
                }
                #pragma omp barrier
                double newFreq = (parameters->fMin) + ((parameters->fMax)-(parameters->fMin) * randDouble(0,1));
                for (unsigned int i=0;i<parameters->vectorDim;i++) {
                    double bestPos=batPos->data[bestIndex][i];
                    
                    batVel->data[i]=batVel->data[i] + ((batPos->data[thread][i]-bestPos)*newFreq);
                    tmpPos->data[i]=batPos->data[thread][i]+batVel->data[i];
                }

                //Checks if the bat should randmly perform a local search
                if (randDouble(0,1)>batPulse) {
                    //The local search is based on the average loudness 
                    for (unsigned int i=0;i<parameters->vectorDim;i++) {
                        tmpPos->data[i]=batPos->data[thread][i]+(randDouble(0,1)*avgLoudness);
                    }    
                } 

                double newFitness=objFunction(tmpPos);
                if (randDouble(0,1)>batLoudness && newFitness<bestFitness) {
                    //If the new position gives a better fitness value, position,velocity, loudness and pulse are updated. 
                    copyToMatrix(tmpPos,batPos,thread);
                    batLoudness=(parameters->alpha)*batLoudness;
                    batPulse=(parameters->initPulse)*(1-exp((-(parameters->gamma)*t)));
                    batFitness = newFitness;
                }
            }
            
            destroyVector(&tmpPos);
        }
        results->bestFitness=bestFitness;
        results->bestIndex=bestIndex;
        copyToVector(batPos,results->bestPos,bestIndex);
    }
    destroyMatrix(&batPos);
}



int main(int argc, char** argv) {
    printf("Initializaing structures for launching the bat algorithm... ");
    batAlgorithmParameters* parameters=NULL;
    batAlgorithmResults* results=NULL;
    initParameters(&parameters,3);
    initResults(&results,3);
    if (parameters==NULL || results==NULL)  {
        printf("error\n");
        return 1;
    }
    printf("done\n");
    printf("Initializing initial parameters... ");
    Vector* initPosVector=NULL;
    Vector* initVelVector=NULL;
    initVector(&initPosVector,3);
    initVector(&initVelVector,3);
    if (initPosVector==NULL || initVelVector==NULL) {
        printf("error\n");
        return 1;
    }
    initVectorData(initPosVector,0);
    initVectorData(initVelVector,0);
    printf("done\n");
    parameters->fMin=0;
    parameters->fMax=2;
    parameters->initPulse=0.5;
    parameters->initLoudness=0.1;
    parameters->gamma=0.9;
    parameters->alpha=0.9;
    parameters->vectorDim=3;
    parameters->initPosRadius=5;
    parameters->initVelRadius=5;
    parameters->bats=2000;
    parameters->iterations=50;
    copyVector(initPosVector,parameters->initPos);
    copyVector(initVelVector,parameters->initVel);
    printParameters(parameters);
    printf("Starting algorithm...\n");

    struct timeval start,end;
    double totalTime=0;
    double elapsedTime=0;
    for (unsigned int i=0;i<NLAUNCHS;i++) {
        gettimeofday(&start,NULL);
        batAlgorithmOMP(parameters,results,objectiveFunction);
        gettimeofday(&end,NULL);
        elapsedTime=(double)(end.tv_sec-start.tv_sec)+(double)(end.tv_usec-start.tv_usec)/1.0e6;
        totalTime+=elapsedTime;
        printf("Iteration %d took %f\n",i,elapsedTime);
        printResults(results);
    }
    printf("Average execution time: %f\n",(double)(totalTime/(double )NLAUNCHS));

    printf("Destroying structures...\n");
    destroyVector(&initPosVector);
    destroyVector(&initVelVector);
    destroyParameters(&parameters);
    destroyResults(&results);
    return 0;
}
