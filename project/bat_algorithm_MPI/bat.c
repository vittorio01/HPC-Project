#include "bat.h"

void batInit(Bat* bat, Vector* initX, Vector* initV, float initF, float initA, float initR) {
    bat->x = initX;
    bat->v = initV;
    bat->freq = initF;
    bat->a = initA;
    bat->r = initR;
}

void batEcho();

/* batDestroy() destroy the bat structure pointed by batInstance()*/
void batDestroy() {

}


