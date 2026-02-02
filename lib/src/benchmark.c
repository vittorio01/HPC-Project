#include "benchmark.h"
#include "tools.h"
// Rosenbrock N-D
// sum_{i=0..d-2} [(1-x_i)^2 + 100(x_{i+1}-x_i^2)^2]
double rosenbrock(const Vector *pos) {
    if (!pos || !pos->data) return NAN;
    if (pos->d < 2) return 0.0;

    double sum = 0.0;
    for (int i = 0; i < pos->d - 1; i++) {
        const double xi = pos->data[i];
        const double xj = pos->data[i+1];
        const double a = 1.0 - xi;
        const double b = xj - (xi * xi);
        sum += (a * a) + 100.0 * (b * b);
    }
    return sum;
}

// Sphere N-D
double sphere(const Vector *pos) {
    if (!pos || !pos->data) return NAN;

    double sum = 0.0;
    for (int i = 0; i < pos->d; i++) {
        const double x = pos->data[i];
        sum += x * x;
    }
    return sum;
} 

double ackley(const Vector *pos) {
    double sumSq=0.0;
    double sumCos=0.0;
    for (unsigned int i=0;i<pos->d;i++) {
        sumSq += pos->data[i]*pos->data[i];
        sumCos += cos(ACKLEY_C * pos->data[i]);
    }
    double term1= -ACKLEY_A * exp(-ACKLEY_B * sqrt(sumSq/pos->d));
    double term2= -exp(sumCos/pos->d);
    return term1+term2 + ACKLEY_A + exp(1.0);
}

double grienwank(const Vector *pos) {
    double sumSq = 0.0;
    double prodCos = 1.0;
    for (unsigned int i=0;i<pos->d; i++) {
        sumSq += pos->data[i]*pos->data[i];
        prodCos *= cos((pos->data[i])/sqrt((double)(i+1)));
    }
    return 1.0 + sumSq /4000.0 - prodCos;
}

double levy(const Vector *pos) {
    double result=0.0;

    for (unsigned int i=0;i<pos->d;i++) {
        double wi=1.0 + (pos->data[i] - 1.0) / 4.0; 

        if (i==0) {
            result += pow(sin(M_PI*wi),2);
        }
        if (i<(pos->d-1)) {
            double wiNext=1.0+(pos->data[i+1]-1.0)/4.0;
            result += pow((wi-1.0),2) * (1.0 + 10.0 * pow(sin(M_PI * wiNext),2));
        } else {
            result += pow((wi-1.0),2)*(1.0 +pow(sin(2.0 * M_PI * wi),2));
        }
    }
    return result;
}


/**
 * Optional wrapping function that corrects the result based on whether
 * Maximization or Minimazation is chosen (through OPTIMIZATION_MODE)
 */
double objective_eval(ObjectiveFn f, const Vector *pos) {
    if (!f) return NAN;
    const double raw_val = f(pos);

    #if OPTIMIZATION_MODE == 1
        return -raw_val;    // Maximize
    #else
        return raw_val;     // Minimize
    #endif
}


void printBenchmarkData(batAlgorithmResults* results, batAlgorithmParameters* parameters, double elapsed) {
    if (results == NULL || parameters == NULL) return;
    /*
        Format: TAG, Fitness, Time, Bats, Iterations, Dimensions, Processes
    */
    printf("BENCHMARK_DATA, %.12f, %.12f, %u, %u, %u\n",
        results->bestFitness,
        elapsed,
        parameters->bats,
        parameters->iterations,
        parameters->vectorDim);
}
