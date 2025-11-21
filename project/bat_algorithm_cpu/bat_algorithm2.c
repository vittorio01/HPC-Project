#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

/*
 * Simple sequential implementation of the Bat Algorithm.
 * - Runs on a single CPU core (no MPI / OpenMP).
 * - Uses a Sphere objective function:
 *
 *      f(x) = sum_{i=1}^{N_DIM} x_i^2
 *
 * - Measures the execution time of each run with gettimeofday().
 * - Repeats the algorithm 10 times and prints the average time.
 */

#define N_BATS  20      /* number of bats                */
#define N_DIM   2       /* problem dimension             */
#define N_ITER  1000    /* number of iterations          */
#define F_MIN   0.0f    /* minimum frequency             */
#define F_MAX   2.0f    /* maximum frequency             */

/* Sphere objective function:
 * f(x) = sum_{i=1}^{N_DIM} x_i^2
 * We use float to be consistent with the provided data library.
 */
static float objective(const float *x)
{
    float sum = 0.0f;
    for (int i = 0; i < N_DIM; ++i)
        sum += x[i] * x[i];
    return sum;
}

/* Random number in [0, 1] as float */
static float rand_float(void)
{
    return (float)rand() / (float)RAND_MAX;
}

/* Run one instance of the Bat Algorithm and return the best fitness found. */
static float bat_algorithm(void)
{
    /* Positions and velocities of bats */
    float bats[N_BATS][N_DIM];
    float velocity[N_BATS][N_DIM] = {0.0f};

    float fitness[N_BATS];
    float best[N_DIM];
    float best_fitness = 1.0e30f; /* effectively "infinity" for our purposes */

    /* --- Initialization of bats --- */
    for (int i = 0; i < N_BATS; ++i) {
        for (int j = 0; j < N_DIM; ++j) {
            /* random position in [-5, 5] */
            bats[i][j] = rand_float() * 10.0f - 5.0f;
        }
        fitness[i] = objective(bats[i]);

        if (fitness[i] < best_fitness) {
            best_fitness = fitness[i];
            for (int j = 0; j < N_DIM; ++j)
                best[j] = bats[i][j];
        }
    }

    /* --- Main optimization loop --- */
    for (int t = 0; t < N_ITER; ++t) {
        for (int i = 0; i < N_BATS; ++i) {
            /* Frequency for this bat */
            float freq = F_MIN + (F_MAX - F_MIN) * rand_float();

            /* Velocity and position update */
            for (int j = 0; j < N_DIM; ++j) {
                velocity[i][j] += (bats[i][j] - best[j]) * freq;
                bats[i][j] += velocity[i][j];
            }

            /* Evaluate new solution */
            float new_fitness = objective(bats[i]);
            if (new_fitness < fitness[i]) {
                fitness[i] = new_fitness;

                /* Update global best */
                if (new_fitness < best_fitness) {
                    best_fitness = new_fitness;
                    for (int j = 0; j < N_DIM; ++j)
                        best[j] = bats[i][j];
                }
            }
        }
    }

    return best_fitness;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct timeval start, end;
    double total_time = 0.0;
    const int runs = 10;

    /* Seed RNG once */
    srand((unsigned int)time(NULL));

    for (int round = 0; round < runs; ++round) {
        gettimeofday(&start, NULL);
        float best_fitness = bat_algorithm();
        gettimeofday(&end, NULL);

        double elapsed = (double)(end.tv_sec - start.tv_sec) +
                         (double)(end.tv_usec - start.tv_usec) / 1.0e6;

        printf("Run %2d: best fitness = %.6f, time = %.6f sec\n",
               round + 1, best_fitness, elapsed);

        total_time += elapsed;
    }

    printf("Average execution time over %d runs: %.6f sec\n",
           runs, total_time / (double)runs);

    return 0;
}
