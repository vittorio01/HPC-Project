#include "tools_1.h"

double rosenbrockStandard2D(double x, double y) {
    double a = 1.0 - x;
    double b = y - x * x;
    return (a * a) + 100 * (b*b);
}

double evaluateFitness2D(Vector *pos) {
    double x = pos->data[0];
    double y = pos->data[1];

    double raw_val = rosenbrockStandard2D(x, y);
    #if OPTIMIZATION_MODE == 1
        // Maximization: invert the sign.
        // The algorithm minimizes negative numbers, which means maximizing positives
        return -raw_val;
    #else
        // Minimization
        return raw_val;
    #endif
}


double random_uniform(double min, double max) {
    return min + ((double) rand() / (double) RAND_MAX)*(max-min);
}
