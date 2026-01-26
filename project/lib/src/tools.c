#include "tools.h"

void initParameters(batAlgorithmParameters** parameters, unsigned int vectorDim) {
    (*parameters)=malloc(sizeof(batAlgorithmParameters));
    if ((*parameters) == NULL) return;

    initVector(&((*parameters)->initPos),vectorDim);
    if ((*parameters)->initPos==NULL) return;
    initVectorData((*parameters)->initPos,DEFAULT_POS);
    (*parameters)->fMin=DEFAULT_FMIN;
    (*parameters)->fMax=DEFAULT_FMAX;
    (*parameters)->initPulse=DEFAULT_PULSE;
    (*parameters)->initLoudness=DEFAULT_LOUDNESS;
    (*parameters)->gamma=DEFAULT_GAMMA;
    (*parameters)->alpha=DEFAULT_ALPHA;
    (*parameters)->vectorDim=vectorDim;
    (*parameters)->initPosRadius=DEFAULT_POS_RADIUS;
    (*parameters)->bats=DEFAULT_BATS_NUMBER;
    (*parameters)->iterations=DEFAULT_ITERATIONS;
}
void destroyParameters(batAlgorithmParameters** parameters) {
    if ((*parameters)==NULL) return;
    destroyVector(&((*parameters)->initPos));
    free((*parameters));
    (*parameters)=NULL;
}

void printParameters(batAlgorithmParameters* parameters) {
    if (parameters==NULL) return;
    printf("* Bat algorithm parameters * \n");
    printf("Number of bats and iterations: %d, %d\n",parameters->bats,parameters->iterations);
    printf("Problem dimension: %d variables\n",parameters->vectorDim);
    printf("Initial position ");
    printVector(parameters->initPos,0,parameters->vectorDim);
    printf("Generation radius: %f\n",parameters->initPosRadius);
    printf("Frequency limits: [%f,%f]\n",parameters->fMin,parameters->fMax);
    printf("Initial loudness: %f\n",parameters->initLoudness);
    printf("Initial pulse: %f\n",parameters->initPulse);
    printf("Initial gamma and alpha: %f , %f\n",parameters->gamma,parameters->alpha);
    printf("\n");
}

void initResults(batAlgorithmResults** results, unsigned int vectorDim) {
    (*results)=malloc(sizeof(batAlgorithmResults));
    if ((*results)==NULL) return;
    initVector(&((*results)->bestPos),vectorDim);
}
void destroyResults(batAlgorithmResults** results) {
    if ((*results)==NULL) return;
    destroyVector(&((*results)->bestPos));
    free((*results));
    (*results)=NULL;
}

void printResults(batAlgorithmResults* results) {
    if (results==NULL) return;
    printf("* Bat algorithm results * \n");
    printf("Best position found ");
    printVector(results->bestPos,0,results->bestPos->d);
    printf("Best fitness value: %f\n",results->bestFitness);
    printf("Bat index: %d\n",results->bestIndex);
    printf("\n");
} 

double randomUniformRadius(double pos, double radius, ugSeed* r) {
    return pos + radius * (2.0 * gsl_rng_uniform(r) - 1.0);
}
double randomUniform(double min, double max, ugSeed* r) {
    return min + (max - min) * gsl_rng_uniform(r);
}

void generateSeed(ugSeed** r,unsigned int threadId) {
    (*r)=gsl_rng_alloc(gsl_rng_mt19937);
    unsigned long seed=(unsigned long)clock()+threadId*1000u;
    gsl_rng_set(*r, seed);
}

void destroySeed(ugSeed** r) {
    gsl_rng_free(*r);
    (*r)=NULL;
}



