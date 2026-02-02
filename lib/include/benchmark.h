#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

//#include <tools.h>
#include <data.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI 
#define M_PI 3.14159265358979323846
#endif 

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

//rosenbrock -> global minimum at [1,...,1]
double rosenbrock(const Vector *pos);

//sphere -> global minimum at [0,...,0]
double sphere(const Vector *pos);

//ackley -> global minimum at [0,...,0], local around every x_i=k
#define ACKLEY_A 20.0 
#define ACKLEY_B 0.2
#define ACKLEY_C (2*M_PI)
double ackley(const Vector *pos);

//grienwank -> global minimum at [0,...,0], local around x_1=+-2*pi*sqrt(i) 
double grienwank(const Vector *pos);

//levy -> global minimum at [1,...,1], local around x_1= 1 +- 4 (for every dimention)
double levy(const Vector *pos);

/* Wrapper*/
double objective_eval(ObjectiveFn f, const Vector *pos);

/* printBenchmarkData() prints a CSV-formatted line for valid parsing by scripts:
 * results    -> pointer to results structure
 * parameters -> pointer to parameters structure
 * elapsed    -> execution time in seconds
 */
void printBenchmarkData(batAlgorithmResults* results, batAlgorithmParameters* parameters, double elapsed);

#endif


