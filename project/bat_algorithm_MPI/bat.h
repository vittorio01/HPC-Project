/* 
 * Implementation of bat localization for the MPI algorithm
*/

#ifndef __BAT_H__
#define __BAT_H__

#include <stdlib.h>
#include <time.h>
#include "data.h"

typedef struct Bat{
    Vector* x;          //Current Position vector
    Vector* v;          //Current Velocity vector
    double freq;         //current Frequency
    double a;            //Current Loudness
    double r;            //Current Pulse Rate
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
void batInit(Bat* bat, Vector* initX, Vector* initV, double initFreq, double initA, double initR);

/*
* batRandom() spawns a bat at a random position in the function definition space
* and with a random velocity.
* bat -> pointer to bat struct
* xBound -> max random X coordinate 
* yBound -> max random Y coordinate 
* vBound -> max random velocity
*/
void batRandom(Bat* bat, double xBound, double yBound, double vBound);

/* the bat prints some useful info*/
void batEcho();

/* batDestroy() destroy the bat structure pointed by batInstance()*/
void batDestroy();

#endif 

