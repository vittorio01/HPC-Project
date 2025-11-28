#include "bat.h"
#include <time.h>

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
    initVectorRandom(pos, -posBound, posBound);
    Vector * v = malloc(sizeof(Vector));

    // Initialize bat
    batInit(bat, pos, v, initF, initA, initR);
}

void batCheckPos(Bat *bat, double posBound) {
   if (bat == NULL || bat->pos == NULL) {
       return;
   }
   // checking X position (positive)
   if (bat->pos->data[0] > posBound) {
       bat->pos->data[0] = posBound;
   }
   // checking X position (negative)
   if (bat->pos->data[0] < -posBound) {
       bat->pos->data[0] = -posBound;
   }
   // checking Y position (positive)
   if (bat->pos->data[1] > posBound) {
       bat->pos->data[1] = posBound;
   }
   // checking Y position (negative)
   if (bat->pos->data[1] < -posBound) {
       bat->pos->data[1] = -posBound;
   }
}

void batEcho();

void batDestroy();
