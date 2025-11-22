#include "bat.h"

void batInit(Bat* bat, Vector* initX, Vector* initV, double initF, double initA, double initR) {
    bat->x = initX;
    bat->v = initV;
    bat->freq = initF;
    bat->a = initA;
    bat->r = initR;
}

void batRandom(Bat* bat, double xBound, double yBound, double vBound) {
    
}

void batEcho();

/* batDestroy() destroy the bat structure pointed by batInstance()*/
void batDestroy() {

}


