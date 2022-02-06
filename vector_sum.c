#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <assert.h>

// Number of elements to compute.
#define N 1048576

/*
 * Generate random array.
 *
 * @input: number of elements
 * @output: pointer to first element of the array.
 */
float *generate_rand_arr(long n) {
        float *rand_arr = (float *)malloc(sizeof(float) * n);
        int i;

        for (i = 0; i < n; i++) {
                rand_arr[i] = (rand() / (float)RAND_MAX);
        }
        return rand_arr;
}

/*
 * Compute sum of elements of 2 arrays.
 *
 * @input: pointer to first element of first array, pointer to first element of second array, number of elements
 * @output: array of sums.
 */
float *compute_el_sum(float *first_arr, float *second_arr, long n) {
	float *result_arr = (float *)malloc(sizeof(float) * n);
	int i;

	for (i = 0; i < n; i++) {	
		result_arr[i] = first_arr[i] + second_arr[i];
	}
	return result_arr;
}

/*
 * Start the programm.
 */
int main (int argc, char **argv) {
	// Set random seed.
	srand(time(0));

        double start, stop, timing;

	long chunk = 0;

        int rank, size;
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	start = MPI_Wtime();

	// chunk of arrays to compute.
	chunk = N / size;

	float *rand_arr_B = NULL;
	float *rand_arr_C = NULL;

	// Process 0 initializes two random arrays of the same size.
	if (rank == 0) {
		rand_arr_B = generate_rand_arr(N);
		rand_arr_C = generate_rand_arr(N);
		
		// Print first and last element to check the calculation.
		printf("B[0]: %f, B[N - 1]: %f\n ", rand_arr_B[0], rand_arr_B[N - 1]);
		printf("C[0]: %f, C[N - 1]: %f\n ", rand_arr_C[0], rand_arr_C[N - 1]);
	}

	// Send even chunks of arrays to other processes.
	float *sub_arr_B = (float *)malloc(sizeof(float) * chunk);
	float *sub_arr_C = (float *)malloc(sizeof(float) * chunk);
	MPI_Scatter(rand_arr_B, chunk, MPI_FLOAT, sub_arr_B, chunk, MPI_FLOAT, 0, MPI_COMM_WORLD);
	MPI_Scatter(rand_arr_C, chunk, MPI_FLOAT, sub_arr_C, chunk, MPI_FLOAT, 0, MPI_COMM_WORLD);
	
	// Calculate sums for each chunk.
	float *sub_sum_arr = NULL;
	sub_sum_arr = compute_el_sum(sub_arr_B, sub_arr_C, chunk);
	
	// Process 0 initializes recieving buffer.
	float *sum_arr = NULL;
	if (rank == 0) {
		sum_arr = (float *)malloc(sizeof(float) * N);
		assert(sum_arr != NULL);
	}

	// Gather all partial sums.
	MPI_Gather(sub_sum_arr, chunk, MPI_FLOAT, sum_arr, chunk, MPI_FLOAT, 0, MPI_COMM_WORLD);
	
	stop = MPI_Wtime();
	timing = stop - start;

	// Print results.	
	if (rank == 0) {
		printf("A[0]: %f, A[n - 1]: %f\n", sum_arr[0], sum_arr[N - 1]);
		printf("Time: %f\n", timing);
	}
	
	if (rank == 0) {
		free(rand_arr_B);
		free(rand_arr_C);
		free(sum_arr);
	}

	free(sub_arr_B);
	free(sub_arr_C);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}

