/* 
 * Implementation of bat localization for the MPI algorithm
*/

#ifndef __BAT_H__
#define __BAT_H__

#include <stdlib.h>
#include "data.h"

struct Bat {
    Vector* v;          //CurrentVelocity vector
    Vector* x;          //Current Position vector
    float freq;         //current frequency
    float A;            //Current loudnes
    float r;            //Current range
    unsigned int step;  //Current algorithm step
}

BAT* batInstance=NULL;

/*
 * batInit() initialize the batInstance structure. Requires:
 * initV -> vector pointer of the initial velocity
 * initX -> vector pointer of the initial position
 * initFreq -> initial frequency Fmin
 * initA -> initial loudness A0
 * initR -> initial range
*/
void batInit(Vector* initV, Vector* initX, float initFreq, float initA, float initR);
void batEcho();

/* batDestroy() destroy the bat structure pointed by batInstance()*/
void batDestroy();

#endif 
