#include "bat.h"
#include "tools.h"
#include <stdio.h>
#include <time.h>

void batInit(Bat* bat, Vector* initPos, Vector* initV, double initF, double initA, double initR) {
    if (bat == NULL) {
        return;
    } 
    bat->pos = initPos;
    bat->v = initV;
    bat->freq = initF;
    bat->a = initA;
    bat->r = initR;
}

void batRandom(Bat* bat, unsigned int dim, double posBound, double vBound, double initF) {
    // Generate a random position
    Vector * pos = NULL;
    initVector(&pos, dim);
    if (pos == NULL) {
        fprintf(stderr, "\nError: Failed to init position vector in batRandom()");
        exit(EXIT_FAILURE);        
    }
    initVectorRandom(pos, -posBound, posBound);

    // Generate random velocity
    Vector * v = NULL;
    initVector(&v, dim);
    if (v == NULL) {
        fprintf(stderr, "Error: Failed to init velocity vector in batRandom()");
        exit(EXIT_FAILURE);        
    }
    initVectorRandom(v, -vBound, vBound);

    // Generate random loudness
    double loudness = random_uniform(0.0, 1.0);
    // Generate random pulse rate
    double pulse_rate = random_uniform(0.0, 1.0); 
    // Initialize bat
    batInit(bat, pos, v, initF, loudness, pulse_rate);
}

void batCheckPos(Bat *bat, double posBound) {
   if (bat == NULL || bat->pos == NULL) {
       return;
   }

   for (unsigned int i = 0; i < bat->pos->d; i++) {
       if (bat->pos->data[i] > posBound) {
           bat->pos->data[i] = posBound;
       } else if (bat->pos->data[i] < -posBound) {
           bat->pos->data[i] = -posBound;
       }
   }
}

void batEcho();

void batDestroy();
