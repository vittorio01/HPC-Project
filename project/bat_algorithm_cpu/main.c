#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#define N_BATS  40          
#define N_DIM   2
#define N_ITER  5000        // more iterations for better accuracy
#define F_MIN   0.0f
#define F_MAX   2.0f

/* Bat Algorithm parameters  */
#define ALPHA   0.9f        // loudness decay
#define GAMMA   0.9f        // pulse rate increase
#define A0      1.0f        // initial loudness
#define R0      0.5f        // initial pulse rate

/* Sphere objective: f(x) = sum x_i^2 */
static float objective(const float *x)
{
    float s = 0.0f;
    for (int i = 0; i < N_DIM; ++i)
        s += x[i] * x[i];
    return s;
}

static float rand_float(void)
{
    return (float)rand() / (float)RAND_MAX;
}

/* Runs one instance of the Bat Algorithm.
 * best_out: array of size N_DIM where we store the best position.
 * Returns: best fitness.
 */
static float bat_algorithm(float best_out[N_DIM])
{
    float x[N_BATS][N_DIM];         // positions
    float v[N_BATS][N_DIM] = {0};   // velocities
    float f[N_BATS];                // fitness

    float A[N_BATS];                // loudness
    float r[N_BATS];                // pulse rate

    float best[N_DIM];              // global best position
    float best_f = 1.0e30f;         // global best fitness

    /* ---- Initialization ---- */
    for (int i = 0; i < N_BATS; ++i) {
        for (int d = 0; d < N_DIM; ++d) {
            x[i][d] = rand_float() * 10.0f - 5.0f;   // [-5, 5]
        }
        f[i] = objective(x[i]);

        A[i] = A0;
        r[i] = R0;

        if (f[i] < best_f) {
            best_f = f[i];
            for (int d = 0; d < N_DIM; ++d)
                best[d] = x[i][d];
        }
    }

    /* ---- Main loop ---- */
    for (int t = 1; t <= N_ITER; ++t) {

        /* average loudness A^t (needed for local random walk) */
        float A_mean = 0.0f;
        for (int i = 0; i < N_BATS; ++i)
            A_mean += A[i];
        A_mean /= (float)N_BATS;

        for (int i = 0; i < N_BATS; ++i) {

            /* Frequency and global move */
            float freq = F_MIN + (F_MAX - F_MIN) * rand_float();

            for (int d = 0; d < N_DIM; ++d) {
                v[i][d] += (x[i][d] - best[d]) * freq;
                x[i][d] += v[i][d];
            }

            /* Local random walk around the best solution
                   with probability (rand > r[i]) */
            if (rand_float() > r[i]) {
                float eps = 2.0f * rand_float() - 1.0f;  // [-1,1]
                for (int d = 0; d < N_DIM; ++d) {
                    x[i][d] = best[d] + eps * A_mean;
                }
            }

            /* Evaluate new solution */
            float f_new = objective(x[i]);

            /* Stochastic acceptance using loudness */
            if (f_new <= f[i] && rand_float() < A[i]) {

                f[i] = f_new;

                /* Update loudness and pulse rate */
                A[i] *= ALPHA;
                r[i] = R0 * (1.0f - expf(-GAMMA * (float)t));

                /* Update global best if needed */
                if (f_new < best_f) {
                    best_f = f_new;
                    for (int d = 0; d < N_DIM; ++d)
                        best[d] = x[i][d];
                }
            }
        }
    }

    /* copy best position out */
    if (best_out) {
        for (int d = 0; d < N_DIM; ++d)
            best_out[d] = best[d];
    }

    return best_f;
}

int main(void)
{
    struct timeval start, end;
    double total_time = 0.0;
    const int runs = 10;

    float global_best[N_DIM];
    float global_best_f = 1.0e30f;

    srand((unsigned)time(NULL));

    for (int r = 0; r < runs; ++r) {
        float best_run_pos[N_DIM];

        gettimeofday(&start, NULL);
        float best_run_f = bat_algorithm(best_run_pos);
        gettimeofday(&end, NULL);

        double elapsed = (double)(end.tv_sec - start.tv_sec) +
                         (double)(end.tv_usec - start.tv_usec) / 1.0e6;

        printf("Run %2d: best fitness = %.6f, time = %.6f sec\n",
               r + 1, best_run_f, elapsed);

        total_time += elapsed;

        if (best_run_f < global_best_f) {
            global_best_f = best_run_f;
            for (int d = 0; d < N_DIM; ++d)
                global_best[d] = best_run_pos[d];
        }
    }

    printf("Average execution time over %d runs: %.6f sec\n",
           runs, total_time / runs);

    /* ---- Final solution and correctness check ---- */
    printf("\n=== Final best solution over all runs ===\n");
    printf("Best fitness found: %.8f\n", global_best_f);
    printf("Best position: [");
    for (int d = 0; d < N_DIM; ++d)
        printf("%s%.8f", d == 0 ? "" : ", ", global_best[d]);
    printf("]\n");

    /* For Sphere: optimum at (0,...,0), f(x*) = 0 */
    float dist2 = 0.0f;
    for (int d = 0; d < N_DIM; ++d)
        dist2 += global_best[d] * global_best[d];
    float dist = sqrtf(dist2);

    printf("True optimum (Sphere): f(x*) = 0 at (0,...,0)\n");
    printf("Fitness error: %.8e\n", global_best_f);
    printf("Distance of best point to optimum: %.8e\n", dist);

    const float TOL = 1e-3f;
    if (global_best_f < TOL)
        printf("=> Solution is CLOSE to the true optimum (within tolerance %.1e).\n", TOL);
    else
        printf("=> Solution is NOT very close yet (try different params / more iterations).\n", TOL);

    return 0;
}
