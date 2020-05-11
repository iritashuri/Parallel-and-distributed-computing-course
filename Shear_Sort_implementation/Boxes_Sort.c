/*
 ============================================================================
 Name        : Boxes_Sort.c
 Author      : Irit Ashuri
 Version     : 1
 Description : Sorting Array of boxes , using Shear Sort implementation
 ============================================================================
 */

#include<mpi.h>
#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Boxes_Sort.h"

#define MASTER 0
// Order of sorting
#define LEFT_TO_RIGHT 0
#define RIGHT_TO_LEFT 1
#define UP_TO_DOWN 2

#define DEBUG_MODE 1 //for debugging with printnig to console - DEBUG_MODE = 1

int main(int argc, char *argv[]) {
	int rank, size;
	MPI_Status status;
	int coord[2];
	int numOfBoxes, n;
	Box *boxes;
	Box myBox;
	MPI_Datatype BoxMPIType;
	//MPI_Comm old_comm;
	MPI_Comm new_comm;
	// Change to boxes.dat / boxes2.dat to different values
	const char *fileName = "./cubids.dat";

	const char *newFileName = "./result.dat";
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Creating Box type to MPI */
	creatBoxMpiType(&BoxMPIType);

	if (rank == MASTER) {
		/* Read boxes from file into boxes array and update number of boxes in to numOfBoxes */
		boxes = readBoxesFromFile(fileName, &numOfBoxes);
		/* Check that number of processes is equal to number of boxes */
		if (numOfBoxes != size)
			printf("Wrong processes number, you need %d processes\n",
					numOfBoxes);
		/* The number of processes need to be n*n */
		n = (int) (sqrt(numOfBoxes));
		if (n * n != numOfBoxes) {
			printf(
					"number of boxes should be n*n type ------------ n=%d numOfBoxes=%d\n",
					n, numOfBoxes);
			MPI_Abort(MPI_COMM_WORLD, __LINE__);
		}
	}

	/* Send n to all of the processes */
	MPI_Bcast(&n, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	/* Create cartezian organization */
	createCartezian(n, &new_comm, rank, coord);
	/* Scatter boxes to all process */
	MPI_Scatter(boxes, 1, BoxMPIType, &myBox, 1, BoxMPIType, MASTER,
	MPI_COMM_WORLD);
	/* Each process calculate his box volume */
	calcVolume(&myBox);

	/* If debug mode is not 0 - print the marix of ids and volumes before sorting */
	if (DEBUG_MODE) {
		MPI_Gather(&myBox, 1, BoxMPIType, boxes, 1, BoxMPIType, MASTER,
				new_comm);

		if (rank == MASTER) {
			printf("Before sorting : id matrixs \n");
			for (int i = 0; i < n * n; i++) {
				printf("%2d\t", boxes[i].id);
				if (i % n == (n - 1)) {
					printf("\n");
				}
			}
			printf("\n");
			printf("Before sorting : volume matrixs \n");
			for (int i = 0; i < n * n; i++) {
				printf("%.2lf\t", boxes[i].volume);
				if (i % n == (n - 1)) {
					printf("\n");
				}
			}
			printf("\n");
		}
	}

	/* Sort boxes according volume in a matrix */
	shearSort(&myBox, coord, n, new_comm, BoxMPIType, &status);
	/* Gather all results to boxes array according to the sorting order */
	MPI_Gather(&myBox, 1, BoxMPIType, boxes, 1, BoxMPIType, MASTER, new_comm);

	if (DEBUG_MODE) {

	}

	if (rank == MASTER) {
		/* If debug mode is not 0 - print the marix of ids and volumes after sorting */
		if (DEBUG_MODE) {
			printf("After sorting : id matrix\n");
			for (int i = 0; i < n * n; i++) {
				printf("%2d\t", boxes[i].id);
				if (i % n == (n - 1)) {
					printf("\n");
				}
			}
			printf("\n");

			printf("After sorting : volume matrix\n");
			for (int i = 0; i < n * n; i++) {
				printf("%.2lf\t", boxes[i].volume);
				if (i % n == (n - 1)) {
					printf("\n");
				}
			}
			printf("\n");
		}
		writeToFile(boxes, n, newFileName);
	}
	MPI_Finalize();
	return 0;
}

/* Calculate volume of box */
void calcVolume(Box *box) {
	box->volume = box->x * box->y * box->z;
}

/* Create Cartesian order */
void createCartezian(int n, MPI_Comm *comm, int rank, int *coord) {
	int dim[2];
	int period[2];
	// No option to reorder
	int reorder = 0;

	// A two-dimensional cylinder of n*n processes in a n*n grid //
	dim[0] = n;
	dim[1] = n;
	// No cycling
	period[0] = 0;
	period[1] = 0;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, comm);

	// Give each process Cartesian coordinates according the rank
	MPI_Cart_coords(*comm, rank, 2, coord);
}

// Read file, update number of boxes with n, return Array of boxes from file.
Box* readBoxesFromFile(const char *fileName, int *numOfBoxes) {
	Box *boxes;
	FILE *fp;
	int i;
	char c;
	*numOfBoxes = 0;

// Open the file
	fp = fopen(fileName, "r");

	// Check if file exists
	if (fp == NULL) {
		printf("Could not open file %s\n", fileName);
		return 0;
	}

	// Extract characters from file and store in character c
	for (c = getc(fp); c != EOF; c = getc(fp))
		if (c == '\n') // Increment count if this character is newline
			*numOfBoxes += 1;
	// Check that there is data in the file
	if (*numOfBoxes == 0) {
		printf("no data found\n");
		fclose(fp);
		return NULL;
	}
	// Allocate boxes array
	boxes = (Box*) malloc((*numOfBoxes) * sizeof(Box));

	if (!boxes) {
		printf("Error with allocate boxes array\n");
		fclose(fp);
		return NULL;
	}
	// Close the file
	fclose(fp);

	// Open file
	fp = fopen(fileName, "r");
	if (fp == NULL) {
		printf("Could not open file %s\n", fileName);
		return 0;
	}
	// Scan data from file in to boxes array
	// x = length,y =  width,z =  height.
	for (i = 0; i < *numOfBoxes; i++) {
		fscanf(fp, "%d", &(boxes[i].id));
		fscanf(fp, "%lf", &(boxes[i].x));
		fscanf(fp, "%lf", &(boxes[i].y));
		fscanf(fp, "%lf", &(boxes[i].z));
		fflush(stdout);
	}

	// Close file
	fclose(fp);
	return boxes;
}

// Create MPI user data type for box
void creatBoxMpiType(MPI_Datatype *BoxMPIType) {
	Box box;
	MPI_Datatype type[5] = { MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
	MPI_DOUBLE };
	int blocklen[5] = { 1, 1, 1, 1, 1 };
	MPI_Aint disp[5];
	// Define size of each value and its location in
	disp[0] = (char*) &box.id - (char*) &box;
	disp[1] = (char*) &box.x - (char*) &box;
	disp[2] = (char*) &box.y - (char*) &box;
	disp[3] = (char*) &box.z - (char*) &box;
	disp[4] = (char*) &box.volume - (char*) &box;
	MPI_Type_create_struct(5, blocklen, disp, type, BoxMPIType);
	MPI_Type_commit(BoxMPIType);

}

// Parallel Shear Sort implementation
void shearSort(Box *myBox, int *location, int n, MPI_Comm cart_comm,
		MPI_Datatype BoxMPIType, MPI_Status *status) {
	int i;
	int myId;
	MPI_Comm_rank(cart_comm, &myId);

	for (i = 0; i < 2 * log2(n) + 1; i++) {
		// Step is even
		if (i % 2 == 0) {
			// Row is even
			if (location[0] % 2 == 0) {
				sort(LEFT_TO_RIGHT, myBox, location, n, cart_comm, BoxMPIType,
						status);

			}
			// Row is odd
			else {
				sort(RIGHT_TO_LEFT, myBox, location, n, cart_comm, BoxMPIType,
						status);
			}
		}
		// Step is odd
		else {
			sort(UP_TO_DOWN, myBox, location, n, cart_comm, BoxMPIType, status);
		}

	}

}

// Call oddEvenSort according the code location
void sort(int order, Box *myBox, int *location, int n, MPI_Comm cart_comm,
		MPI_Datatype BoxMPIType, MPI_Status *status) {
	int Pleft, Pright;
	int rowColsCase;
	int myRow = location[0];
	int myCol = location[1];
	int myId;
	MPI_Comm_rank(cart_comm, &myId);
	switch (order) {
	case LEFT_TO_RIGHT:
		// Row case
		rowColsCase = 1;
		MPI_Cart_shift(cart_comm, rowColsCase, 1, &Pleft, &Pright);
		oddEvenSort(myBox, myCol, n, cart_comm, Pleft, Pright, BoxMPIType,
				status, order);
		break;
	case RIGHT_TO_LEFT:
		// Row case
		rowColsCase = 1;
		MPI_Cart_shift(cart_comm, rowColsCase, 1, &Pleft, &Pright);
		oddEvenSort(myBox, myCol, n, cart_comm, Pleft, Pright, BoxMPIType,
				status, order);
		break;
	case UP_TO_DOWN:
		// Column case
		rowColsCase = 0;
		MPI_Cart_shift(cart_comm, rowColsCase, 1, &Pleft, &Pright);
		oddEvenSort(myBox, myRow, n, cart_comm, Pleft, Pright, BoxMPIType,
				status, order);
		break;
	default:
		break;
	}

}

// OddEvenSort implementation
void oddEvenSort(Box *myBox, int location, int n, MPI_Comm cart_comm, int Pleft,
		int Pright, MPI_Datatype BoxMPIType, MPI_Status *status, int order) {
	int i;
	Box otherBox;
	int myId;
	MPI_Comm_rank(cart_comm, &myId);
	for (i = 0; i < n; i++) {
		// Even step
		if (i % 2 == 0) {
			// Even location
			if (location % 2 == 0 && Pright > 0) {
				MPI_Send(myBox, 1, BoxMPIType, Pright, 0, cart_comm);
				MPI_Recv(&otherBox, 1, BoxMPIType, Pright, 0, cart_comm,
						status);
				if (order == LEFT_TO_RIGHT || order == UP_TO_DOWN)
					min(myBox, &otherBox);
				else
					max(myBox, &otherBox);
			}
			// Odd location
			else if (location % 2 != 0) {
				if (Pleft >= 0) {
					MPI_Recv(&otherBox, 1, BoxMPIType, Pleft, 0, cart_comm,
							status);
					MPI_Send(myBox, 1, BoxMPIType, Pleft, 0, cart_comm);
					if (order == LEFT_TO_RIGHT || order == UP_TO_DOWN)
						max(myBox, &otherBox);
					else
						min(myBox, &otherBox);
				}
			}
		}
		// Odd step
		else {
			// Even location
			if (location % 2 == 0 && location != 0 && Pleft >= 0) {
				MPI_Send(myBox, 1, BoxMPIType, Pleft, 0, cart_comm);
				MPI_Recv(&otherBox, 1, BoxMPIType, Pleft, 0, cart_comm, status);
				if (order == LEFT_TO_RIGHT || order == UP_TO_DOWN)
					max(myBox, &otherBox);
				else
					min(myBox, &otherBox);
				// Odd location
			} else if (location % 2 != 0) {
				if (location != (n - 1) && Pright > 0) {
					MPI_Recv(&otherBox, 1, BoxMPIType, Pright, 0, cart_comm,
							status);
					MPI_Send(myBox, 1, BoxMPIType, Pright, 0, cart_comm);
					if (order == LEFT_TO_RIGHT || order == UP_TO_DOWN)
						min(myBox, &otherBox);
					else
						max(myBox, &otherBox);
				}

			}
		}
	}
}

// box1 volume should be smaller the box2
void min(Box *box1, Box *box2) {
	if (box1->volume > box2->volume)
		copyBox(box1, box2);
	// If volume is even compare height
	else if (box1->volume == box2->volume) {
		if (box1->z > box2->z)
			copyBox(box1, box2);
	}
}

// box1 volume should be larger the box2
void max(Box *box1, Box *box2) {
	if (box1->volume < box2->volume)
		copyBox(box1, box2);
	// If volume is even compare height
	else if (box1->volume == box2->volume) {
		if (box1->z < box2->z)
			copyBox(box1, box2);
	}
}

// Copy box value
void copyBox(Box *box1, Box *box2) {
	box1->id = box2->id;
	box1->x = box2->x;
	box1->y = box2->y;
	box1->z = box2->z;
	box1->volume = box2->volume;
}

// Write Array of boxes in to dat file
void writeToFile(Box *boxes, int n, const char *newFileName) {
	int i, j;
	int counter = 0;

	FILE *res = fopen(newFileName, "w+t");
	if (res == NULL) {
		printf("Cannot open the file\n");
		exit(1);
	}
// Print to the file in the sorted order
	for (i = 0; i < n; i++) {
		if (i % 2 == 0) {
			for (j = 0; j < n; j++) {
				fprintf(res, "%2d   ", boxes[counter].id);
				counter++;
			}
		} else {
			counter += 3;
			for (j = 0; j < n; j++) {
				fprintf(res, "%2d   ", boxes[counter].id);
				counter--;
			}
			counter += 5;
		}
	}

	if (fclose(res) != 0) {
		printf("Cannot close the file\n");
		exit(1);
	}
}
