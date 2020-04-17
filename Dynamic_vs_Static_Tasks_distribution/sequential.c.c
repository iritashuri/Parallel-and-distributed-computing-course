/*
 ============================================================================
 Name        : static.c
 Author      : Irit Ashuri
 Version     : 1.0
 Description : sequential tasks 
 ============================================================================
 */

#include <stdio.h>
#include <mpi.h>
#include <math.h>

#define HEAVY 100000
#define SHORT 1
#define LONG 10
//This function performs heavy computations,
// its run time depends on x and y values
double heavy(int x, int y)
{
	int i, loop=SHORT;
	double sum =0;
	//super heavy tasks
	if (x<3 || y<3)
		loop = LONG;
	for (i=0;i<loop*HEAVY;i++)
		sum += cos(exp(sin((double)i/HEAVY)));
	return sum;
}


int main(int argc, char *argv[])
{
	int			my_rank;		/* rank of process */
	int			num_procs;		/* number of processes */
	double t1, t2;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	t1 = MPI_Wtime();
	int x,y;
	int N=20;
	double answer = 0;
	for (x=0;x<N;x++)
		for (y=0;y<N;y++)
			answer += heavy(x,y);
	printf("Answer =%e\n",answer);
	t2 = MPI_Wtime();
	printf("Execution time = %lf\n", t2 - t1);
	/* shut down MPI */
	MPI_Finalize();

	return 0;
}
