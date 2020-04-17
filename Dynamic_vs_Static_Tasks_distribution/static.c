/*
 ============================================================================
 Name        : static.c
 Author      : Irit Ashuri
 Version     : 1.0
 Description : Static tasks partition
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
			printf("ERROR during HEAVY calculation\n");
		}

}
void slave(int n) {
	double answer = 0;
	int xyArray[3];
	MPI_Status status;
	int i;

	/*Receive the number from master*/
	MPI_Recv(xyArray, 3, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);
	int x = xyArray[0];
	int y = xyArray[1];
	/*Run heavy according to number of tasks salve need to do and change x,y accordingly*/
	for (i = 0; i < xyArray[2]; i++) {
		answer += heavy(x, y);
		addIterationToArray(n, xyArray);
		x = xyArray[0];
		y = xyArray[1];
	}

	/*Return an answer*/
	MPI_Send(&answer, 1, MPI_DOUBLE, MASTER, 0, MPI_COMM_WORLD);
}

void master(int numOfTasksPerSlave, int numOfSlaves, int n, int mod) {
	double value;
	double res = 0;
	int i, j;
	MPI_Status status;
	int xyArray[3]; /*xy[0]=x, xy[1]=y, xy[2]=numOfTasks*/
	xyArray[0] = 0;
	xyArray[1] = 0;

	for (i = 1; i <= numOfSlaves; i++) {
		xyArray[2] = numOfTasksPerSlave;

		/*Adding to numOfTasksPerSlave 1 or 0 tasks, according to mod results*/
		if (mod) {
			xyArray[2]++;
			mod--;
		}
		/*Send tasks to slave i*/
		MPI_Send(xyArray, 3, MPI_INT, i, 0, MPI_COMM_WORLD);
		/*Move iteration according the last number of tasks*/
		for (j = 0; j < xyArray[2]; j++) {
			addIterationToArray(n, xyArray);
		}
	}

	/*Get results from slaves*/
	for (i = 1; i <= numOfSlaves; i++) {
		MPI_Recv(&value, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
		res += value;
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
	int numOfSlaves = size - 1;
	int numOfTasks = n * n;
	/*Part tasks equal to all Slaves*/
	int numOfTasksPerSlave = numOfTasks / numOfSlaves;
	/*Tasks that left after the partition */
	int mod = numOfTasks % (numOfSlaves);

	if (rank == MASTER) {
		double t1, t2;
		t1 = MPI_Wtime();
		master(numOfTasksPerSlave, numOfSlaves, n, mod);
		t2 = MPI_Wtime();
		printf("Execution time = %lf\n", t2 - t1);
	} else {
		/*Not master = slave*/
		slave(n);
	}

	MPI_Finalize();
	return 0;
}
