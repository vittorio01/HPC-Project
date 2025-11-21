#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define N_BATS 20          // number of bats
#define N_DIM  2           // problem dimension
#define N_ITER 1000        // number of iterations
#define F_MIN 0.0          // min frequency
#define F_MAX 2.0          // max frequency

// Objective function (example: sphere)
double objective(double *x) {
    double sum = 0.0;
    for (int i = 0; i < N_DIM; i++)
        sum += x[i] * x[i];
    return sum;
}

double rand_double() {
    return (double)rand() / RAND_MAX;
}

void bat_algorithm() {
    double bats[N_BATS][N_DIM];
    double velocity[N_BATS][N_DIM] = {0};
    double fitness[N_BATS];
    double best[N_DIM];
    double best_fitness = 1e9;

    // Initialize bats
    for (int i = 0; i < N_BATS; i++) {
        for (int j = 0; j < N_DIM; j++)
            bats[i][j] = rand_double() * 10 - 5; // random between [-5,5]
        fitness[i] = objective(bats[i]);
        if (fitness[i] < best_fitness) {
            best_fitness = fitness[i];
            for (int j = 0; j < N_DIM; j++)
                best[j] = bats[i][j];
        }
    }

    // Main loop
    for (int t = 0; t < N_ITER; t++) {
        for (int i = 0; i < N_BATS; i++) {
            double freq = F_MIN + (F_MAX - F_MIN) * rand_double();
            for (int j = 0; j < N_DIM; j++) {
                velocity[i][j] += (bats[i][j] - best[j]) * freq;
                bats[i][j] += velocity[i][j];
            }

            double new_fitness = objective(bats[i]);
            if (new_fitness < fitness[i]) {
                fitness[i] = new_fitness;
                if (new_fitness < best_fitness) {
                    best_fitness = new_fitness;
                    for (int j = 0; j < N_DIM; j++)
                        best[j] = bats[i][j];
                }
            }
        }
    }

    printf("Best fitness: %f\n", best_fitness);
}

int main() {
    struct timeval start, end;
    double total_time = 0.0;

    srand(0); // for reproducibility

    for (int round = 0; round < 10; round++) {
        gettimeofday(&start, NULL);
        bat_algorithm();
        gettimeofday(&end, NULL);

        double elapsed = (end.tv_sec - start.tv_sec) +
                         (end.tv_usec - start.tv_usec) / 1e6;
        printf("Round %d execution time: %.6f sec\n", round + 1, elapsed);
        total_time += elapsed;
    }

    printf("Average execution time: %.6f sec\n", total_time / 10.0);
    return 0;
}
