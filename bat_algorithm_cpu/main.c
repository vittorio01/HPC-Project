#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include "tools.h"
#include "benchmark.h"

/* Create a temporary Vector view over a row in Matrix (no allocation). */
static inline Vector makeVectorView(double *row, unsigned int dim)
{
    Vector v;
    v.data = row;
    v.d    = (int)dim;   // tools.c / benchmark.c use Vector->d
    return v;
}

/* CPU Bat Algorithm */
static void batAlgorithmCPU(batAlgorithmParameters *parameters,
                            batAlgorithmResults *results,
                            ObjectiveFn function,
                            ugSeed *seed)
{
    const unsigned int bats  = parameters->bats;
    const unsigned int dim   = parameters->vectorDim;
    const unsigned int iters = parameters->iterations;

    Matrix *x = NULL;     // positions  [bats x dim]
    Matrix *v = NULL;     // velocities [bats x dim]
    Vector *fit = NULL;   // fitness    [bats]
    Vector *A = NULL;     // loudness   [bats]
    Vector *r = NULL;     // pulse rate [bats]

    Vector *best = NULL;  // best position [dim]
    double best_f = 1.0e300;
    unsigned int best_idx = 0;

    initMatrix(&x, bats, dim);
    initMatrix(&v, bats, dim);
    initVector(&fit, bats);
    initVector(&A, bats);
    initVector(&r, bats);
    initVector(&best, dim);

    initMatrixData(v, 0.0);

    /* ---- Initialization ---- */
    for (unsigned int i = 0; i < bats; ++i) {
        for (unsigned int d = 0; d < dim; ++d) {
            x->data[i][d] = randomUniformRadius(parameters->initPos->data[d],
                                                parameters->initPosRadius,
                                                seed);
        }

        Vector posView = makeVectorView(x->data[i], dim);
        fit->data[i] = objective_eval(function, &posView);

        A->data[i] = parameters->initLoudness;
        r->data[i] = parameters->initPulse;

        if (fit->data[i] < best_f) {
            best_f = fit->data[i];
            best_idx = i;
            memcpy(best->data, x->data[i], dim * sizeof(double));
        }
    }

    /* ---- Main loop ---- */
    for (unsigned int t = 1; t <= iters; ++t) {

        double A_mean = 0.0;
        for (unsigned int i = 0; i < bats; ++i) A_mean += A->data[i];
        A_mean /= (double)bats;

        for (unsigned int i = 0; i < bats; ++i) {

            /* frequency and global move */
            double freq = parameters->fMin +
                          (parameters->fMax - parameters->fMin) *
                          randomUniform(0.0, 1.0, seed);

            for (unsigned int d = 0; d < dim; ++d) {
                v->data[i][d] += (x->data[i][d] - best->data[d]) * freq;
                x->data[i][d] += v->data[i][d];
            }

            /* local random walk with probability (rand > r[i]) */
            if (randomUniform(0.0, 1.0, seed) > r->data[i]) {
                double eps = randomUniform(-1.0, 1.0, seed);
                for (unsigned int d = 0; d < dim; ++d) {
                    x->data[i][d] = best->data[d] + eps * A_mean;
                }
            }

            /* evaluate */
            Vector posView = makeVectorView(x->data[i], dim);
            double f_new = objective_eval(function, &posView);

            /* stochastic acceptance using loudness */
            if (f_new <= fit->data[i] && randomUniform(0.0, 1.0, seed) < A->data[i]) {

                fit->data[i] = f_new;

                /* update loudness and pulse rate */
                A->data[i] *= parameters->alpha;
                r->data[i] = parameters->initPulse *
                             (1.0 - exp(-parameters->gamma * (double)t));

                /* update global best */
                if (f_new < best_f) {
                    best_f = f_new;
                    best_idx = i;
                    memcpy(best->data, x->data[i], dim * sizeof(double));
                }
            }
        }
    }

    /* ---- Save results ---- */
    results->bestFitness = best_f;
    results->bestIndex   = best_idx;
    copyVector(best, results->bestPos);

    destroyMatrix(&x);
    destroyMatrix(&v);
    destroyVector(&fit);
    destroyVector(&A);
    destroyVector(&r);
    destroyVector(&best);
}

int main(int argc, char** argv)
{
    struct timeval start, end;

    /* Parse --dim early BEFORE initParameters(), because tools.c does not reallocate initPos on --dim */
    int dim = 2; // default
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dim") == 0 && i + 1 < argc) {
            dim = atoi(argv[++i]);
            break;
        }
    }

    batAlgorithmParameters *parameters = NULL;
    batAlgorithmResults *results = NULL;

    initParameters(&parameters, (unsigned int)dim);
    initResults(&results, (unsigned int)dim);

    /* Keep ONLY initPos initialization to [0,...,0] */
    for (unsigned int d = 0; d < parameters->vectorDim; ++d)
        parameters->initPos->data[d] = 0.0;

    /* Parse CLI (overrides defaults) + choose objective function */
    ObjectiveFn function = sphere; // safe default if user doesn't provide --function
    parseArguments(argc, argv, parameters, &function);

    /* RNG seed via tools.h (replaces srand(time(NULL))) */
    ugSeed *seed = NULL;
    generateSeed(&seed, 0);

    /* Run multiple times and compute average execution time */
    const int RUNS = 10;
    double total = 0.0;

    /* Keep best result over runs */
    batAlgorithmResults *bestResults = NULL;
    initResults(&bestResults, (unsigned int)dim);
    bestResults->bestFitness = 1.0e300;

    for (int run = 0; run < RUNS; ++run) {
        gettimeofday(&start, NULL);
        batAlgorithmCPU(parameters, results, function, seed);
        gettimeofday(&end, NULL);

        double elapsed = (double)(end.tv_sec - start.tv_sec) +
                         (double)(end.tv_usec - start.tv_usec) / 1.0e6;

        total += elapsed;

        if (results->bestFitness < bestResults->bestFitness) {
            bestResults->bestFitness = results->bestFitness;
            bestResults->bestIndex   = results->bestIndex;
            copyVector(results->bestPos, bestResults->bestPos);
        }
    }

    double avg = total / (double)RUNS;
    printf("Average execution time over %d runs: %.6f sec\n\n", RUNS, avg);

    /* Required prints */
    printResults(bestResults);
    printBenchmarkData(bestResults, parameters, avg);

    /* cleanup */
    destroySeed(&seed);
    destroyResults(&bestResults);
    destroyResults(&results);
    destroyParameters(&parameters);

    return 0;
}
