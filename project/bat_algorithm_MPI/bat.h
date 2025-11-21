/* 
 * Implementation of bat localization for the MPI algorithm
*/

#ifndef __BAT_H__
#define __BAT_H__

#include <stdlib.h>
#include "data.h"

typedef struct Bat{
    Vector* x;          //Current Position vector
    Vector* v;          //Current Velocity vector
    float freq;         //current Frequency
    float a;            //Current Loudness
    float r;            //Current Pulse Rate
    // unsigned int step;  //Current algorithm step
} Bat;


/*
 * batInit() initialize the batInstance structure. Requires:
 * bat -> pointer to Bat Structure
 * initV -> vector pointer of the initial velocity
 * initX -> vector pointer of the initial position
 * initFreq -> initial frequency Fmin
 * initA -> initial loudness A0
 * initR -> initial range
*/
void batInit(Bat* bat, Vector* initX, Vector* initV, float initFreq, float initA, float initR);

/* the bat prints some useful info*/
void batEcho();

/* batDestroy() destroy the bat structure pointed by batInstance()*/
void batDestroy();

#endif 
