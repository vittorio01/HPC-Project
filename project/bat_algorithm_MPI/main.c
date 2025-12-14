/* Parallel MPI Version of the BAT Algorithm v0.1 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "data.h"
#include "bat.h"
#include "bat.c"
#include "tools_1.h"
#include "tools_1.c"


// -- Bat Properties --
#define N_BATS 100
#define N_ITER 1000   // Max Optimization Iterations
#define F_MIN 0.0
#define F_MAX 100.0
#define A_0 1.0     // Initial Loudness
#define R_0 0.05     // Initial Pulse Rate
#define ALPHA 0.9   // Loudness Cooling Factor
#define BETA_MIN 0.0
#define BETA_MAX 1.0
#define GAMMA 0.9   // Pulse Rate Cooling Factor
#define V_BOUND 5.0  // Max Initial Random Velocity

// -- Function Properties --
// space is gonna be from [-POS_BOUND, POS_BOUND] x [-POS_BOUND, POS_BOUND], so a square
#define POS_BOUND 50  // Max X, Y coordinates
#define DIM 2         // Problem Dimension

int main(int argc, char** argv) {

    // Seed for random numbers
    srand((unsigned int) time(NULL));

    // Required data structures
    Bat ** bat_array = malloc(N_BATS * sizeof(Bat *));
    Vector * fitness = NULL;
    initVector(&fitness, N_BATS);

    int best_index = 0; // index of bat with best fitness
    double best_fitness; // fitness value of best bat

    // Create N Bats instances, each with random initial position, velocity, loudness and pulse rate
    for (int i = 0; i < N_BATS; i++) {
        // Spawn bats with random parameters
        bat_array[i] = malloc(sizeof(Bat));
        batRandom(bat_array[i], POS_BOUND, V_BOUND, 0);

        // evaluate initial fitness
        fitness->data[i] = evaluateFitness2D(bat_array[i]->pos);
    }

    // Evaluate initial global fitness, position with first best fitness is recorded
    best_index = minOfVec(fitness);
    best_fitness = fitness->data[best_index];

    // Perform N_ITER iteration of the optimization loop
    for (int i = 0; i < N_ITER; i++) {
        // Compute average loudness
        double sum_A = 0;
        double avg_A = 0;
        for (int j = 0; j < N_BATS; j++) {
            sum_A = sum_A + bat_array[j]->a;
        }
        avg_A = sum_A / N_BATS;

        // Iterate through bats
        for (int j = 0; j < N_BATS; j++) {
            // DEBUG
            printf("\nIteration %d, bat %d", i, j);

            // New candidate Bat position
            Vector *cand = NULL;
            initVector(&cand, DIM);
            if (!cand) continue;
            copyVector(bat_array[j]->pos, cand);

            // -- GLOBAL SEARCH --
            // Choose beta for each bat
            double beta = random_uniform(BETA_MIN, BETA_MAX);

            // frequency update
            bat_array[j]->freq = F_MIN + (F_MAX - F_MIN) * beta;

            // velocity update (for each component)
            // ex. V = V{j} + (X{j}- X{best_bat}) * f{j}
            double delta_x = bat_array[j]->pos->data[0] - bat_array[best_index]->pos->data[0];
            double delta_y = bat_array[j]->pos->data[1] - bat_array[best_index]->pos->data[1];
            bat_array[j]->v->data[0] += delta_x * bat_array[j]->freq;
            bat_array[j]->v->data[1] += delta_y * bat_array[j]->freq;

            // Updating candidate position (X_new = X + V)
            cand->data[0] += bat_array[j]->v->data[0];
            cand->data[1] += bat_array[j]->v->data[1];

            // Clamp candidate position within bounds
            Bat temp_bat;
            temp_bat.pos = cand;
            batCheckPos(&temp_bat, POS_BOUND);
            copyVector(temp_bat.pos, cand);

            // -- LOCAL SEARCH (RANDOM WALK) --
            // - Generate a random number between 0 and 1, if that is greater than bat pulse rate, then perform local search
            double rand_local_search = random_uniform(0.0, 1.0);

            // Local search is going to do a random local search for that bat
            if (rand_local_search > bat_array[j]->r) {
               double epsilon = random_uniform(-1.0 , 1.0);

               // you can use here either avg_A over all bats or bat[j]->A, experiment with both!
               cand->data[0] += epsilon * avg_A;
               cand->data[1] += epsilon * avg_A;

               // Clamp again after local search
               Bat temp_bat;
               temp_bat.pos = cand;
               batCheckPos(&temp_bat, POS_BOUND);
               copyVector(temp_bat.pos, cand);
            }
               
            // -- EVALUATION, ACCEPTANCE AND UPDATE OF A AND R --
            // Evaluate the new solutions after random walk
            double new_fitness = evaluateFitness2D(cand);
            // Accept the new solution only if it is better && rand_acceptance < bat->a
            double rand_acceptance = random_uniform(0.0, 1.0);
            if (new_fitness < fitness->data[j] && rand_acceptance< bat_array[j]->a) {
                // free old position before updating
                destroyVector(&(bat_array[j]->pos));
                // Update position
                bat_array[j]->pos = cand;
                // Update fitness
                fitness->data[j] = new_fitness;
                // Update loudness and Pulse Rate
                bat_array[j]->a = ALPHA * bat_array[j]->a;
                bat_array[j]->r = R_0 * (1.0 - exp(-GAMMA * (double)i)); 
            } else {
                // free rejected pos
                destroyVector(&cand);
            }

        }

        // Update Global Best
        best_index = minOfVec(fitness);
        best_fitness = fitness->data[best_index];
    }

    // Print final Result
    printf("\nBest Fitness: %f\n", best_fitness);
    printf("Best position: (%f, %f)\n",
        bat_array[best_index]->pos->data[0],
        bat_array[best_index]->pos->data[1]);
    // Print theoretical minima + distance from theory
    printf("\nRosenbrock minima is at: (1, 1). with a value of 0");
    printf("\nDistance between bat best position and function minima: (%f, %f)",
                bat_array[best_index]->pos->data[0] - 1.0,
                bat_array[best_index]->pos->data[1] - 1.0);

    // Free each bat and its internal structures
    for (int i = 0; i < N_BATS; i++) {
        if (bat_array[i] != NULL) {
            destroyVector(&(bat_array[i]->pos));
            destroyVector(&(bat_array[i]->v));
            free(bat_array[i]);
        }
    }

    free(bat_array);
    destroyVector(&fitness);


    return 0;
}
