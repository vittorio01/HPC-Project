#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#include "tools.h"   // <-- brings batAlgorithmParameters / batAlgorithmResults + includes data.h

/* Sphere objective: f(x) = sum x_i^2 */
static double objective(const double *x, unsigned int dim)
{
    double s = 0.0;
    for (unsigned int i = 0; i < dim; ++i)
        s += x[i] * x[i];
    return s;
}

static double rand_double(void)
{
    return (double)rand() / (double)RAND_MAX;
}

/* Main CPU version required by issue #15:
 * - reads everything from parameters
 * - writes everything into results
 */
static void batAlgorithmCPU(batAlgorithmParameters *parameters, batAlgorithmResults *results)
{
    const unsigned int bats = parameters->bats;
    const unsigned int dim  = parameters->vectorDim;
    const unsigned int iters = parameters->iterations;

    Matrix *x = NULL;   // positions  [bats x dim]
    Matrix *v = NULL;   // velocities [bats x dim]
    Vector *f = NULL;   // fitness    [bats]
    Vector *A = NULL;   // loudness   [bats]
    Vector *r = NULL;   // pulse rate [bats]

    Vector *best = NULL; // best position [dim]
    double best_f = 1.0e300;
    unsigned int best_idx = 0;

    /* allocate dynamic structures (from data.h library) */
    initMatrix(&x, bats, dim);
    initMatrix(&v, bats, dim);
    initVector(&f, bats);
    initVector(&A, bats);
    initVector(&r, bats);
    initVector(&best, dim);

    initMatrixData(v, 0.0);

    /* ---- Initialization (spawn around initPos with radius) ---- */
    for (unsigned int i = 0; i < bats; ++i) {
        for (unsigned int d = 0; d < dim; ++d) {
            double u = 2.0 * rand_double() - 1.0; // [-1,1]
            x->data[i][d] = parameters->initPos->data[d] + u * parameters->initPosRadius;
        }

        f->data[i] = objective(x->data[i], dim);

        A->data[i] = parameters->initLoudness;
        r->data[i] = parameters->initPulse;

        if (f->data[i] < best_f) {
            best_f = f->data[i];
            best_idx = i;
            for (unsigned int d = 0; d < dim; ++d)
                best->data[d] = x->data[i][d];
        }
    }

    /* ---- Main loop ---- */
    for (unsigned int t = 1; t <= iters; ++t) {

        /* average loudness */
        double A_mean = 0.0;
        for (unsigned int i = 0; i < bats; ++i)
            A_mean += A->data[i];
        A_mean /= (double)bats;

        for (unsigned int i = 0; i < bats; ++i) {

            /* frequency and global move */
            double freq = parameters->fMin + (parameters->fMax - parameters->fMin) * rand_double();

            for (unsigned int d = 0; d < dim; ++d) {
                v->data[i][d] += (x->data[i][d] - best->data[d]) * freq;
                x->data[i][d] += v->data[i][d];
            }

            /* local random walk with probability (rand > r[i]) */
            if (rand_double() > r->data[i]) {
                double eps = 2.0 * rand_double() - 1.0;  // [-1,1]
                for (unsigned int d = 0; d < dim; ++d) {
                    x->data[i][d] = best->data[d] + eps * A_mean;
                }
            }

            /* evaluate */
            double f_new = objective(x->data[i], dim);

            /* stochastic acceptance using loudness */
            if (f_new <= f->data[i] && rand_double() < A->data[i]) {

                f->data[i] = f_new;

                /* update loudness and pulse rate (from parameters->alpha/gamma) */
                A->data[i] *= parameters->alpha;
                r->data[i] = parameters->initPulse * (1.0 - exp(-parameters->gamma * (double)t));

                /* update global best */
                if (f_new < best_f) {
                    best_f = f_new;
                    best_idx = i;
                    for (unsigned int d = 0; d < dim; ++d)
                        best->data[d] = x->data[i][d];
                }
            }
        }
    }

    /* ---- Save into results structure (required by tools.h) ---- */
    results->bestFitness = best_f;
    results->bestIndex   = best_idx;
    copyVector(best, results->bestPos);

    /* free local dynamic allocations */
    destroyMatrix(&x);
    destroyMatrix(&v);
    destroyVector(&f);
    destroyVector(&A);
    destroyVector(&r);
    destroyVector(&best);
}

int main(void)
{
    struct timeval start, end;

    srand((unsigned)time(NULL));

    /* Allocate + init the official IO structs */
    batAlgorithmParameters *parameters = NULL;
    batAlgorithmResults *results = NULL;

    initParameters(&parameters, 2);
    initResults(&results, 2);

    /* Override defaults to match your old constants */
    parameters->bats = 40;
    parameters->iterations = 5000;

    parameters->fMin = 0.0;
    parameters->fMax = 2.0;

    parameters->alpha = 0.9;
    parameters->gamma = 0.9;

    parameters->initLoudness = 1.0;
    parameters->initPulse    = 0.5;

    /* old code used random [-5,5]; we emulate that via initPos + radius */
    for (unsigned int d = 0; d < parameters->vectorDim; ++d)
        parameters->initPos->data[d] = 0.0;
    parameters->initPosRadius = 5.0;

    /* Run + time */
    gettimeofday(&start, NULL);
    batAlgorithmCPU(parameters, results);
    gettimeofday(&end, NULL);

    double elapsed = (double)(end.tv_sec - start.tv_sec) +
                     (double)(end.tv_usec - start.tv_usec) / 1.0e6;

    printf("Execution time: %.6f sec\n\n", elapsed);

    /* Print using the dedicated library function (required) */
    printResults(results);

    /* cleanup (required) */
    destroyResults(&results);
    destroyParameters(&parameters);

    return 0;
}
