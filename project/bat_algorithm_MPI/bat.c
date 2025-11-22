#include "bat.h"

void batInit(Bat* bat, Vector* initPos, Vector* initV, double initF, double initA, double initR) {
    bat->pos = initPos;
    bat->v = initV;
    bat->freq = initF;
    bat->a = initA;
    bat->r = initR;
}

void batRandom(Bat* bat, double posBound, double vBound, double initF, double initA, double initR) {
    // Generate a random position and velocity
    Vector * pos = malloc(sizeof(Vector));
    initVectorRandom(&pos, -posBound, posBound);
    Vector * v = malloc(sizeof(Vector));

    // Initialize bat
    batInit(bat, pos, v, initF, initA, initR);
}

void batEcho();

void batDestroy();


