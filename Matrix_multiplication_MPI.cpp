#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
static int const ROWS = 100;
static int const COLS = 1200;
static int const ROWS_TOTAL = 1200;
static int const COLS_TOTAL = 1200;

void compute(int rcvd_arr_1[10][10], int my_rank);

void performNonBlkSendRcv();

int main(argc, argv)
int argc;
char *argv[];
{

	performNonBlkSendRcv();

	return 0;
}


int **alloc_2d_int(int rows, int cols) {
	int *data = (int *)calloc(rows*cols, sizeof(int));
	int **array = (int **)calloc(rows, sizeof(int*));
	for (int i = 0; i<rows; i++)
		array[i] = &(data[cols*i]);

	int i, j, counter = 0;
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			array[i][j] = rand() % 10;
	return array;
}



//compute the sum
void compute(int rcvd_arr_1[100][10] , int my_rank) {
	int i, j, sum = 0;
	for (i = 0; i < ROWS; i++) {
		printf("Sum of % d row is %d \n", i, sum);
		sum = 0;
		for (j = 0; j < COLS; j++) {
			sum = sum + rcvd_arr_1[i][j];
			
		}

	}
	//printf("================Process %d calculating==========================", my_rank);
	//printf("\n");
	printf("Sum of % d row is %d \n", i, sum);
}

int arr[1200][10];

void populateData(int rowNum) {

	int i, j = 0;
	for (i = rowNum; i < (rowNum + 100) ; i++) {
		for (j = 0; j < COLS; j++) {
			arr[i][j] = rand() % 10;
		}
	}
}

void performNonBlkSendRcv() {
	int pool_size, my_rank;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &pool_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	if (my_rank == 0) {


		/*int i, j, counter = 0;
		for (i = 0; i < ROWS_TOTAL; i++)
			for (j = 0; j < COLS; j++)
				arr[i][j] = rand() % 10;
				*/

		MPI_Request request1;
		MPI_Request request2;
		MPI_Status status;

		int chunk_data = 0;
		int process_iteration = 1;
		int loopCounter = 1;
		int rowNum = 0;

		int numberOfChunks = 2;


		do {
			printf("%d ", rowNum);
			chunk_data = chunk_data + ROWS * COLS;

			//call populate data
			populateData(rowNum);

			// sending first 100 rows 
			MPI_Isend(&arr[rowNum][0], ROWS* COLS, MPI_INT, loopCounter, "MY_TAG_CH1_" + loopCounter, MPI_COMM_WORLD, &request1);
			rowNum = rowNum + ROWS;

			//call populate data
			populateData(rowNum);
			//sending second 100 rows . 
			MPI_Isend(&arr[rowNum][0], ROWS* COLS, MPI_INT, loopCounter, "MY_TAG_CH2_" + loopCounter, MPI_COMM_WORLD, &request2);
			loopCounter = loopCounter + 1;

		} while (loopCounter <= pool_size - 1);

		//printf("The message has been sent\n");

		/*if (request == MPI_REQUEST_NULL) {
			printf("\tthe send request is MPI_REQUEST_NULL now\n");
		}
		else {
			printf("\tthe send request still lingers\n");
		}*/

	}
	else { // for other processes from 1 to 6
		printf("==============Process %d Executing ================= ", my_rank);
		printf("\n");
		int my_name_length, count;
		MPI_Request request1;
		MPI_Request request2;
		MPI_Status status;
		int rcv;

		int rcvArr_1[100][10];
		int rcvArr_2[100][10];

		MPI_Irecv(&rcvArr_1, ROWS* COLS, MPI_INT, 0, "MY_TAG_CH1_" + my_rank, MPI_COMM_WORLD,
			&request1);

		// blocking for 1st data chunk
		MPI_Wait(&request1, &status);

		// recieving 2nd chunk and the first chunk of data is passed for computation which is overlapping
		MPI_Irecv(&rcvArr_2, ROWS* COLS, MPI_INT, 0, "MY_TAG_CH2_" + my_rank, MPI_COMM_WORLD,
			&request2);

		//MPI_Get_count(&status, MPI_CHAR, &count);

		// first chunk is being compute along with overlapped receiveing of 2nd data chunk
		compute(rcvArr_1, my_rank);

		// Blocking for 2nd data chunk
		MPI_Wait(&request2, &status);

		// computing 2nd data chunk once received
		compute(rcvArr_2, my_rank);


		/*if (request == MPI_REQUEST_NULL) {
			printf("\tthe receive request is MPI_REQUEST_NULL now\n");
		}
		else {
			printf("\tthe receive request still lingers\n");
		}*/

	}

	MPI_Finalize();
}
