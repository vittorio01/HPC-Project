#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

//#include <tools.h>
#include <data.h>
#include <math.h>
#include <stdlib.h>

// incomplete definitions for resolving circular dependency with tools.h
typedef struct batAlgorithmResults batAlgorithmResults;
typedef struct batAlgorithmParameters batAlgorithmParameters;


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

/* Wrapper*/
double objective_eval(ObjectiveFn f, const Vector *pos);

/* printBenchmarkData() prints a CSV-formatted line for valid parsing by scripts:
 * results    -> pointer to results structure
 * parameters -> pointer to parameters structure
 * elapsed    -> execution time in seconds
 */
void printBenchmarkData(batAlgorithmResults* results, batAlgorithmParameters* parameters, double elapsed);

#endif


