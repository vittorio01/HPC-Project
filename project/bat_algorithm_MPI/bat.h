/*
 * Implementation of bat localization for the MPI algorithm
*/

#ifndef __BAT_H__
#define __BAT_H__

#include <stdlib.h>
#include <time.h>
#include "data.h"

typedef struct Bat{
    Vector* pos;        // Current Position vector
    Vector* v;          // Current Velocity vector
    double freq;        // Current Frequency
    double a;           // Current Loudness
    double r;           // Current Pulse Rate
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
* posBound -> max random X,Y coordinates (a square)
* vBound -> max random velocity
*/
void batRandom(Bat* bat, double posBound, double vBound, double initF, double initA, double initR);

/**
 * baCheckPos checks if the current bat position is within bounds, if not it changes the bat position to the max bound
 * bat -> pointer to Bat structure
 * posBound -> value of the max allowed X, Y bounds (it's a square)
 */
void batCheckPos(Bat * bat, double posBound);

/* the bat prints some useful info*/
void batEcho();

/* batDestroy() destroy the bat structure pointed by batInstance()*/
void batDestroy();

#endif
