typedef struct {
	int id;
	double x;
	double y;
	double z;
	double volume;
} Box;


Box* readBoxesFromFile(const char *fileName, int *numOfBoxes);
void creatBoxMpiType(MPI_Datatype *BoxMPIType);
void createCartezian(int n, MPI_Comm *comm, int rank, int *coord);
void calcVolume(Box *box);
void max(Box *box1, Box *box2);
void min(Box *box1, Box *box2);
void oddEvenSort(Box *mtBox, int location, int n, MPI_Comm cart_comm, int Pleft,
		int Pright, MPI_Datatype BoxMPIType, MPI_Status *status, int order);
void shearSort(Box *myBox, int *location, int n, MPI_Comm cart_comm,
		MPI_Datatype BoxMPIType, MPI_Status *status);
void sort(int order, Box *myBox, int *location, int n,
		MPI_Comm cart_comm, MPI_Datatype BoxMPIType, MPI_Status *status);
void copyBox(Box *box1, Box *box2);
void writeToFile(Box* boxes, int n, const char* newFileName);
