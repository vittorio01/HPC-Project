#include <stdio.h>
#include <mpi.h>

int main() {

    int msg; // Buffer of Message to be sent
    int comm_sz; // Number of processes
    int my_rank; // My process rank

    MPI_Init(NULL, NULL); // Initialize a communicator
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Process i-th
    if (my_rank != 0) {
        
        // If process 1, 2, 3, ..., to comm_sz - 1
        if (my_rank != comm_sz - 1) {
            printf("\nProcess %d receive from process %d ", my_rank,
                my_rank-1);
            MPI_Recv(&msg, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, 
                MPI_STATUS_IGNORE);  
            printf("\nProcess %d sends to process %d ", my_rank, my_rank+1);
            MPI_Send(&msg, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD);
        }
        // if last process, take message and print
        else {
            MPI_Recv(&msg, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD,
                MPI_STATUS_IGNORE); 
            printf("\nThe message received from last process with"
                "rank %d is msg: %d", my_rank, msg);
        }
    }
    // Process 0
    else 
    {
       // Prepare the message
       msg = 5;
       // Send the message to process 1
       printf("\nSending the Message to process 1");
       MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD); 
    }

    MPI_Finalize();
    return 0;
}
