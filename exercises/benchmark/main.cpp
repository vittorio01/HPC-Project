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
    MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    for (unsigned int i=0;i<STEPS;i++) {
        unsigned int stepSize=START_SIZE*pow(2,i+1);
        sendBuffer=(char*) malloc (stepSize*sizeof(char));
        receiveBuffer=(char*) malloc (stepSize*sizeof(char));

	    if (pid==0) {
            double startTime,endTime;
            for (unsigned int j=0;j<stepSize;j++) {
                sendBuffer[j]='A';
            }
            startTime=MPI_Wtime();
            MPI_Send(sendBuffer,stepSize,MPI_CHAR,1,0,MPI_COMM_WORLD);
            MPI_Recv(receiveBuffer,stepSize,MPI_CHAR,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		    endTime=MPI_Wtime();
            bool check=true;
            float runtime=endTime-startTime;
            float speed=((float)(stepSize)/(float)(runtime*pow(1024,2)));
            for (unsigned int j=0;j<stepSize;j++) {
                check=check & (sendBuffer[j]=receiveBuffer[j]);
            }
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
            MPI_Recv(receiveBuffer,stepSize,MPI_CHAR,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Send(receiveBuffer,stepSize,MPI_CHAR,0,0,MPI_COMM_WORLD);
	    }
        free(sendBuffer);
        free(receiveBuffer);

    }
 		
	MPI_Finalize();
    return 0;	
}
