#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

#define START_SIZE  1
#define STEPS       30

int main(int arcg,char** argv) {
    int np;
	int pid;
    char* sendBuffer;
    char* receiveBuffer;
    //Initialization of MPI 
    MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    //PErforms the test for STEPS times
    for (unsigned int i=0;i<STEPS;i++) {
        //defining packet size
        unsigned int stepSize=START_SIZE*pow(2,i+1);

        //Allocating send and receive buffers
        sendBuffer=(char*) malloc (stepSize*sizeof(char));
        receiveBuffer=(char*) malloc (stepSize*sizeof(char));

	    if (pid==0) {
            //Process 0 sends a packet and waits a response 
            
            //Initialization of time variables and send buffer
            double startTime,endTime;
            for (unsigned int j=0;j<stepSize;j++) {
                sendBuffer[j]='A';  //The buffer isi initializaed with As
            }
            
            //Taking start time
            startTime=MPI_Wtime();
            MPI_Send(sendBuffer,stepSize,MPI_CHAR,1,0,MPI_COMM_WORLD);
            MPI_Recv(receiveBuffer,stepSize,MPI_CHAR,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		    
            //taking end time
            endTime=MPI_Wtime();

            //final checks and speed calculations of the RTT
            bool check=true;
            float runtime=endTime-startTime;                                    //Time spent to sends and receive packet
            float speed=((float)(stepSize)/(float)(runtime*pow(1024,2)*2));     //Bandwidth in Mb/s 
            for (unsigned int j=0;j<stepSize;j++) {
                check=check & (sendBuffer[j]=receiveBuffer[j]);
            }

            //Printing results
            printf("Step %d (%d bytes) \t",i,stepSize);
            if (check) {
                printf("ok \t");
            }
            else {
                printf("error! \t");
            }
            printf("(%fs, %f Mb/s)\n",runtime,speed);
        }
 	    else {
            //Process 1 receives the message and forwards it to process 0
            MPI_Recv(receiveBuffer,stepSize,MPI_CHAR,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Send(receiveBuffer,stepSize,MPI_CHAR,0,0,MPI_COMM_WORLD);
	    }

        //Deallocating buffers for next iteration
        free(sendBuffer);
        free(receiveBuffer);

    }
 	//End of program
	MPI_Finalize();
    return 0;	
}
