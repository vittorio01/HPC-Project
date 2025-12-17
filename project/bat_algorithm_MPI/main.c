/* Parallel MPI Version of the BAT Algorithm v0.1 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#include "data.h"
#include "bat.h"
#include "bat.c"
#include "tools_1.h"
#include "tools_1.c"


// -- Bat Properties --
#define N_BATS 1000000
#define N_ITER 10  // Max Optimization Iterations
#define F_MIN 0.0
#define F_MAX 100.0
#define A_0 1.0     // Initial Loudness
#define R_0 0.05     // Initial Pulse Rate
#define ALPHA 0.9   // Loudness Cooling Factor
#define BETA_MIN 0.0
#define BETA_MAX 1.0
#define EPS_MIN -1.0 // Epsilon minimum value
#define EPS_MAX 1.0 // Epsilon max value
#define GAMMA 0.9   // Pulse Rate Cooling Factor
#define V_BOUND 5.0  // Max Initial Random Velocity

// -- Function Properties --
// space is gonna be from [-POS_BOUND, POS_BOUND] x [-POS_BOUND, POS_BOUND], so a square
#define POS_BOUND 100  // Max X, Y coordinates
#define DIM 2         // Problem Dimension

int main(int argc, char** argv) {

    // MPI
    int comm_sz; // Number of processes
    int my_rank; // Process individual rank 

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Different random seed for every process
    srand((unsigned int) time(NULL) + my_rank);

    // Compute local bat count per process
    int local_n_bats = N_BATS / comm_sz;
    int remainder = N_BATS % comm_sz;

    // Local data structures
    Bat **bat_array = malloc(local_n_bats * sizeof(Bat *));
    Vector *fitness = NULL;
    initVector(&fitness, local_n_bats);

    // Local best tracking
    int local_best_index = 0;
    double local_best_fitness;

    // Global best tracking (replicated on all processes)
    double global_best_fitness;
    Vector *global_best_pos = NULL;
    initVector(&global_best_pos, DIM);

    // Initialize local bats
    for (int i = 0; i < local_n_bats; i++) {
        // Spawn bats with random parameters
        bat_array[i] = malloc(sizeof(Bat));
        batRandom(bat_array[i], POS_BOUND, V_BOUND, 0);
        // evaluate initial fitness
        fitness->data[i] = evaluateFitness2D(bat_array[i]->pos);
    } 

    // find local best
    local_best_index = minOfVec(fitness);
    local_best_fitness = fitness->data[local_best_index];

    // Structure to hold fitness and rank for MPI_MINLOC
    struct {
        double fitness;
        int rank;
    } local_min, global_min;

    local_min.fitness = local_best_fitness;
    local_min.rank = my_rank;

    // Find global best *process* (not coordinates) across all processes
    MPI_Allreduce(&local_min, &global_min, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
    global_best_fitness = global_min.fitness;

    // Broadcast best position from the process that has it
    if (my_rank == global_min.rank) {
        copyVector(bat_array[local_best_index]->pos, global_best_pos);
    }
    MPI_Bcast(global_best_pos->data, DIM, MPI_DOUBLE, global_min.rank, MPI_COMM_WORLD);

    // Main optimization loop
    for (int iter = 0; iter < N_ITER; iter++) {
        // Compute local average loudness
        double local_sum_A = 0;
        for (int i = 0; i < local_n_bats; i++) {
            local_sum_A += bat_array[i]->a;
        }

        // Compute global average loudness
        double global_sum_A = 0;
        // this sums all the local loudnesses for every processes and store it in global_sum_A
        MPI_Allreduce(&local_sum_A, &global_sum_A, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        double avg_A = global_sum_A / N_BATS;

        // Update each local bat
        for (int i = 0; i < local_n_bats; i++) {
            // new candidate bat position
            Vector *cand = NULL;
            initVector(&cand, DIM);
            if (!cand) continue;
            copyVector(bat_array[i]->pos, cand);

            // GLOBAL SEARCH 
            double beta = random_uniform(BETA_MIN, BETA_MAX);
            bat_array[i]->freq = F_MIN + (F_MAX - F_MIN) * beta;

            // update velocity
            double delta_x = bat_array[i]->pos->data[0] - global_best_pos->data[0];
            double delta_y = bat_array[i]->pos->data[1] - global_best_pos->data[1];
            bat_array[i]->v->data[0] += delta_x * bat_array[i]->freq;
            bat_array[i]->v->data[1] += delta_y * bat_array[i]->freq;

            // Update candidate position
            cand->data[0] += bat_array[i]->v->data[0];
            cand->data[1] += bat_array[i]->v->data[1];

            // apply bound condition
            Bat temp_bat;
            temp_bat.pos = cand;
            batCheckPos(&temp_bat, POS_BOUND);
            copyVector(temp_bat.pos, cand); 

            // LOCAL SEARCH (Random Walk)
            // - Generate a random number between 0 and 1, if that is greater than bat pulse rate
            // - then perform local search
            double rand_local_search = random_uniform(0.0, 1.0);
            if (rand_local_search > bat_array[i]->r) {
                double epsilon = random_uniform(EPS_MIN, EPS_MAX);
                // you can use here either avg_A over all bats or bat[i]->A, 
                // experiment with both!
                cand->data[0] += epsilon * avg_A;
                cand->data[1] += epsilon * avg_A;

                // apply bound conditions
                temp_bat.pos = cand;
                batCheckPos(&temp_bat, POS_BOUND);
                copyVector(temp_bat.pos, cand);
            }

            // EVALUATION AND ACCEPTANCE
            // the new solutions (global + random walk) are evaluated
            double new_fitness = evaluateFitness2D(cand);
            double rand_acceptance = random_uniform(0.0, 1.0);

            // Accept the new solution only if it is better && rand_acceptance < bat->a
            if (new_fitness < fitness->data[i] && rand_acceptance < bat_array[i]->a) {
                // free old position before allocating new one
                destroyVector(&(bat_array[i]->pos));
                bat_array[i]->pos = cand;
                fitness->data[i] = new_fitness;
                // Update Loudness and Pulse Rate
                bat_array[i]->a = ALPHA * bat_array[i]->a;
                bat_array[i]->r = R_0 * (1.0 - exp(-GAMMA * (double)iter));
            } else {
                destroyVector(&cand);
            }
        }

        // Update local best
        local_best_index = minOfVec(fitness);
        local_best_fitness = fitness->data[local_best_index];
        local_min.fitness = local_best_fitness;
        local_min.rank = my_rank;

        // Find global best *process*
        MPI_Allreduce(&local_min, &global_min, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);
        global_best_fitness = global_min.fitness;

        // Broadcast global best position
        if (my_rank = global_min.rank) {
            copyVector(bat_array[local_best_index]->pos, global_best_pos);
        }
        MPI_Bcast(global_best_pos->data, DIM, MPI_DOUBLE, global_min.rank, MPI_COMM_WORLD);
    }

 // Print results only from rank 0
    if (my_rank == 0) {
        printf("\nBest Fitness: %f\n", global_best_fitness);
        printf("Best position: (%f, %f)\n", global_best_pos->data[0], global_best_pos->data[1]);
        printf("\nRosenbrock minima is at: (1, 1) with a value of 0\n");
        printf("Distance: (%f, %f)\n", 
               global_best_pos->data[0] - 1.0, 
               global_best_pos->data[1] - 1.0);
    }

    // Cleanup
    for (int i = 0; i < local_n_bats; i++) {
        if (bat_array[i] != NULL) {
            destroyVector(&(bat_array[i]->pos));
            destroyVector(&(bat_array[i]->v));
            free(bat_array[i]);
        }
    }
    free(bat_array);
    destroyVector(&fitness);
    destroyVector(&global_best_pos);

    MPI_Finalize();
    return 0;   // Print results only from rank 0
}
