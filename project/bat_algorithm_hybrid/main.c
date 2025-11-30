#include <data.h>
#include <tools.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define VECTOR_DIM  2
#define N_BATS      20
#define N_ITER      1000
#define F_MIN       0
#define F_MAX       2

#define GAMMA       0.9
#define ALPHA       0.9

double objectiveFunction(double x, double y) {
    return x+y;
}

void batAlgorithmOMP(Vector* initPos, Vector* initVel,double initFreq, double initPulse, double initLoudness,double randPosRadius, double randVelRadius,unsigned int bats, double (*objFunction)(double,double)) {
    if (initVel==NULL || initPos==NULL) return;
    
    //initialization of shared parameters used during the algorithm
    Matrix* batPos=NULL;
    Vector* batFitnesses=NULL;
    
    initMatrix(&batPos,bats,initPos->d);
    initVector(&batFitness,bats);
    if (bat==NULL || batPos==NULL || batFitness==NULL) return;
    
    double avgLoudness;    
    
    bool initialized=true;
    //---  openMP thead ---

    #pragma omp parallel num_threads(bats) {
        unsigned int thread=omp_get_thread_num();

        //Initialization of parameters
        Vector* batVel=NULL;
        double batFreq=initFreq;
        double batPulse=initPulse;
        double batLoudness=initLoudness;
        initVector(&batVel,vectorDim);
        
        //initialization of vectors used during loop 
        Vector* tmpPos=NULL;
        Vector* tmpVel=NULL;

        initVector(&tmpPos,bats->batsNumber);
        initVector(&tmpVel,bats->batsNumber);
        
        //checkinfg if the initialization is done for all threads
        #pragma omp critical {
            if (initialized==true) {
                if (batVel==NULL || tmpVel==NULL || tmpPos==NULL) {
                    initialized=false;
                }
            }
        } 
        #pragma omp barrier 
        if (initialized==false) return;

        //initialization of positions/velocities and fitness
        srand((unsigned int)time(NULL));

        for (unsigned int i=0;i<VECTOR_DIM;i++) {
            batVel->data[i]=randDoubleRadius(initVel->data[i],randVelRadius);
            batPos->data[thread][i]=randDoubleRadius(initPos->data[i],randPosRadius);
        }
        batFitness->data[i]=objFunction(batPos->data[thread][0],batPos->data[thread][1]); 

        //evaluation of the best fitness
        unsigned int bestIndex=maxOfVec(bats->batFitness);
        
        for (unsigned int t=0;t<N_ITER;t++) {
            //Average Loudness calculation
            if (thread==0) avgLoudness=0;
            #pragma omp barrier

            #pragma omp critical {
                avgLoudness+=batLoudness;
            }
            #pragma omp barrier
                
            //Checks if the bat should randmly perform a loca search
            if (randDouble(0,1)>batPulse) {
                //The local search is based on the average loudness 
                for (unsigned int i=0;i<VECTOR_DIM;i++) {
                    tmpPos->data[i]=batPos->data[thread][i]+(randDouble(0,1)*avgLoudness);
                }    
            } 

            else {
                //The global search is based on the last position and the last velocity
                double newFreq = F_MIN + (F_MAX-F_MIN) * randDouble(0,1);
                for (unsigned int i=0;i<bats->vectorDim;i++) {
                    double bestPos=batPos[bestIndex][i];
                    
                    tmpVel->data[i]=batVel->data[i] + ((batPos->data[thread][i]-bestPos)*batFrequency);
                    tmpPos->data[j]=batPos->data[thread][i]+tmpVel->data[i];
                }
            }
            double newFitness=objFunction(tmpPos->data[0],tmpPos->data[i]);
            double bestFitness=bats->batFitness->data[bestFitnessIndex];

            if (randDouble(0,1)>batLoudness && newFitness>bestFitness) {
                //If the new position gives a better fitness value, position,velocity, loudness and pulse are updated. 
                copyToMatrix(tmpPos,batPos,i);
                copyToMatrix(tmpVel,batVel,i);
                batLoudness=ALPHA*batLoudness;
                batPulse=batPulse*(1-exp((-GAMMA)*t));
                batFitness->data[thread]=newFitness;
            }
            //evaluation of the new bestFitness and bestIndex
            bestIndex=maxOfVec(batFitness);
        }
        
        destroyVector(&tmpVel);
        destroyVector(&tmpPos);
    }
}



int main(int argc, char** argv) {
    ompBats* bats=NULL;
    Vector* testPosVector=NULL;
    Vector* testVelVector=NULL;
    initVector(&testPosVector,2);
    initVector(&testVelVector,2);
        
    
    destroyVector(&testPosVector);
    destroyVector(&testVelVector);
    return 0;
}
