#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>



int main(int arcg,char** argv) {
	int np;
	int pid;
	char* sendWord;
	char*  receiveWord;
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	
	if (pid==0) {
		sendWord="PING";
		MPI_Send(sendWord,5,MPI_CHAR,0,0,MPI_COMM_WORLD);
		MPI_Recv(receiveWord,5,MPI_CHAR,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		printf("%s sent to process 1\n",sendWord);
		printf("%s received from process 1\n",receiveWord);
	}
 	else {
		sendWord="PONG";
		MPI_Recv(receiveWord,5,MPI_CHAR,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Send(sendWord,5,MPI_CHAR,1,0,MPI_COMM_WORLD);
	}	
	
}
