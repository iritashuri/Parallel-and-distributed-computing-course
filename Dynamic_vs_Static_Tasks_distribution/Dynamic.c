/*
 ============================================================================
 Name        : static.c
 Author      : Irit Ashuri
 Version     : 1.0
 Description : Dynamic tasks partition
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define MASTER 0
#define HEAVY 100000
#define SHORT 1
#define LONG 10
#define DEBUG 0// 1 for enabled, 0 for disabled
// MPI tags - operations
#define SYN 1
#define DO 2
#define DID 3
#define FIN 4

double heavy(int x, int y) {
	int i, loop = SHORT;
	double sum = 0;

	// Super heavy tasks
	if (x < 3 || y < 3)
		loop = LONG;
	// Heavy calculations
	for (i = 0; i < loop * HEAVY; i++)
		sum += cos(exp(sin((double) i / HEAVY)));

	return sum;
}

/*This function simulate one iteration of double for loop*/
void addIterationToArray(int n, int *arr) {
	if (arr[0] < n && arr[1] < n) {
		if (arr[1] >= 0 && arr[1] < n - 1) {
			arr[1]++;
		} else if (arr[1] == n - 1) {
			arr[1] = 0;
			arr[0]++;
		}
	} else {
		if(DEBUG)
			printf("ERROR during HEAVY calculation\n");
	}
}

void slave(int n) {
	double answer = 0;
	int xyArray[2];
	xyArray[0] = 0;
	xyArray[1] = 0;
	MPI_Status status;
	int rank, FIN_not_received = 1;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/*Synchronization request*/
	MPI_Send(&answer, 1, MPI_DOUBLE, MASTER, SYN, MPI_COMM_WORLD);

	while (FIN_not_received) {
		/*While slaves didn't get the message to stop, continue receive tasks*/
		MPI_Recv(xyArray, 2, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD,
				&status);

		switch (status.MPI_TAG) {
		// DO case - need to run heavy
		case DO:
			answer = heavy(xyArray[0], xyArray[1]);
			MPI_Send(&answer, 1, MPI_DOUBLE, MASTER, DID, MPI_COMM_WORLD);
			break;
			// FIN case - disconnect
		case FIN:
			FIN_not_received--;
			break;
		default:
			break;
		}
	}
}

void master(int n, int size) {
	double value;
	double res = 0;
	MPI_Status status;
	int xyArray[2];							//xy[0]=x, xy[1]=y
	int numOfTasks = (n * n);
	int slavesCounter = 0;

	xyArray[0] = 0;
	xyArray[1] = 0;

	while (numOfTasks) {

		MPI_Recv(&value, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG,
		MPI_COMM_WORLD, &status);
		switch (status.MPI_TAG) {
		/*Synchronization case*/
		case SYN:
			slavesCounter++;
			/*If no more task - finish the connection*/
			if (!numOfTasks) {
				MPI_Send(xyArray, 2, MPI_INT, status.MPI_SOURCE, FIN,
				MPI_COMM_WORLD);
				slavesCounter--;
				/*There are more tasks to do*/
			} else {
				/*Send DO message - give slave more task to do*/
				MPI_Send(xyArray, 2, MPI_INT, status.MPI_SOURCE, DO,
				MPI_COMM_WORLD);

				addIterationToArray(n, xyArray);
			}
			break;
		/*Heavy operation completed*/
		case DID:
			numOfTasks--;
			res += value;

			// If no more task - finish the connection
			if (!numOfTasks) {
				MPI_Send(xyArray, 2, MPI_INT, status.MPI_SOURCE, FIN,
				MPI_COMM_WORLD);
				slavesCounter--;
				/*There are more tasks to do*/
			} else {
				/*Send DO message - give slave more task to do*/
				MPI_Send(xyArray, 2, MPI_INT, status.MPI_SOURCE, DO,
				MPI_COMM_WORLD);
				addIterationToArray(n, xyArray);
			}
			break;

		default:
			break;
		}
	}

	/*Send all slaves message to stop working - all done*/
	for (int i = 1; i < size; i++) {
		MPI_Send(xyArray, 2, MPI_INT, i, FIN, MPI_COMM_WORLD);
	}

	printf("\nAnswer = %e\n", res);
}

int main(int argc, char *argv[]) {
	int n = 20;
	int rank; /* rank of process */
	int size; /* number of processes */

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == MASTER) {
		double t1, t2;
		t1 = MPI_Wtime();
		master(n, size);
		t2 = MPI_Wtime();
		printf("Execution time = %lf\n", t2 - t1);
	} else {
		slave(n);
	}

	MPI_Finalize();
	return 0;
}
