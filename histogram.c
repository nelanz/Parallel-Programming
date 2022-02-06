#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

// number of elements to process
#define N 1024

// file with input
#define FILE_NAME "in.txt"

// histogram bins
#define BINS 100

/*
 * Read numbers from file.
 *
 * @input: pointer to file, number of elements.
 * @output: array of integers
 */
int *read_nums(FILE *fptr, int n) {
	int *nums_arr = (int *)malloc(sizeof(int) * N);
	int num, i;

	for (i = 0; i < n; i++) {
		fscanf(fptr, "%d", &num);
		nums_arr[i] = num;
	}

	return nums_arr;
}

/*
 * Create histogram array.
 *
 * @input: pointer to array of numbers, number of elements, number of bins.
 */
int *create_hist(int *arr, int n, int bins) {
	int *hist = (int *)malloc(sizeof(int) * (bins + 1)); // bins + 1 because of 0
	memset(hist, 0, sizeof(int) * (bins + 1)); 
	int i;

	for (i = 0; i < n; i++) {
		hist[arr[i]]++;	
	}

	return hist;	
}

/*
 * Print histogram array.
 *
 * @input: pointer to array, number of elements.
 */
void print_hist(int *arr, int n) {
	int i;

	for (i = 0; i < n; i++) {
		printf(" %d : %d |", i, arr[i]);
	}
	printf("\n");
}

/*
 * Start the programm.
 */ 
int main (int argc, char** argv) {

	double start, stop, timing;

        int rank, size, chunk = 0;
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        // Compute the size of chunk of array for each process.
        chunk = N/size;

        start = MPI_Wtime();

	// Process 0 reads the file.
	FILE *fptr = NULL;
	int *nums_arr = NULL;
	if (rank == 0) {
		fptr = fopen(FILE_NAME, "r");

        	if (fptr == NULL) {
                	printf("IO Error");
                	exit(1);
       		 }

        	nums_arr = read_nums(fptr, N);
        	fclose(fptr);
	}
	
	// Send chunks of array to other processes. 
	int *sub_arr = (int *)malloc(sizeof(int) * chunk);
        MPI_Scatter(nums_arr, chunk, MPI_INT, sub_arr, chunk, MPI_INT, 0, MPI_COMM_WORLD);
	
	// Calculate histogram for each chunk.
	int *sub_hist = NULL;
	sub_hist = create_hist(sub_arr, chunk, BINS);
	
	// Reduce the results.
	int *global_hist = NULL;
	if (rank == 0) {
		global_hist = (int*)malloc(sizeof(int) * (BINS + 1));
	}       
	MPI_Reduce(sub_hist, global_hist, BINS + 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	stop = MPI_Wtime();
	timing = stop - start;

	// Print the reduced histogram and time.
	if (rank == 0) {
	
		print_hist(global_hist, BINS + 1);	
		printf("Time: %f\n", timing);
	}

	free(nums_arr);
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();	
}
