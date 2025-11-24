/*
*   Useful functions
*/
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <math.h>
#include "data.h"

// Toggle this definition to switch modes
// 0 = Minimize
// 1 = Maximize
#define OPTIMIZATION_MODE 0

/**
 * Standard Rosenbrock Function (2D)
 * f(x, y) = (1-x)^2 + 100(y - x^2)^2
 * Global Min = 0 at (1, 1)
 */
double rosenbrockStandard(double x, double y);

/**
 * Fitness wrapper
 */
double evaluateFitness2D(Vector *pos);

/**
 * Create a random double with uniform distribution between min and max
 */
double random_uniform(double min, double max);


#endif
