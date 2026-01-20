#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include "data.h";

// Toggle this definition to switch modes
// 0 = Minimize
// 1 = Maximize
#define OPTIMIZATION_MODE 0

/**
 * Fitness function pointer type
 */
typedef double (*ObjectiveFn)(const Vector *pos);

/* Benchmark functions (dimension-agnostic)*/
double rosenbrock(const Vector *pos);
double sphere(const Vector *pos);

#endif


