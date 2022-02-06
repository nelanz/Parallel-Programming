#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <stdlib.h>

// number of elements to process.
#define N 1024

/*
 * Function generating random array.
 *
 * @input: number of elements.
 * @output: pointer to the first element of the array.
 */
float *generate_rand_arr(int n) {
        float *rand_arr = (float *)malloc(sizeof(float) * n);
	int i;

        for (i = 0; i < n; i++) {
                rand_arr[i] = (rand() / (float)RAND_MAX);
        }
	return rand_arr;
}

/*
 * Function computing norm in quadratic form.
 *
 * @input: pointer to array, number of elements.
 * @output: sum of squares of array elements.
 */
float compute_norm_quadratic(float *array, int n) {
	float sum = 0.f;
	int i;
	
	for (i = 0; i < n; i++) {
		sum += array[i] * array[i];
	}

	return sum;
} 

/*
 * Start of the programm.
 */
int main(int argc , char** argv) {

	// Set random seed.
	srand(time(0));

	double start, stop, timing;

	int rank, size, chunk = 0;
	MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Compute the size of chunk of array for each process.
	chunk = N/size;

	start = MPI_Wtime();

	// Process 0 generates the random array.
	float *rand_arr = NULL;
	if (rank == 0) {
		rand_arr = generate_rand_arr(N);
	}
	
	// Send chunks of array to other processes.
	float *sub_rand_arr = (float *)malloc(sizeof(float) * chunk);
	MPI_Scatter(rand_arr, chunk, MPI_FLOAT, sub_rand_arr, chunk, MPI_FLOAT, 0, MPI_COMM_WORLD);

	// Compute quadratic norms on chunks of array.
	float sub_norm = compute_norm_quadratic(sub_rand_arr, chunk);
	
	// Reduce all results on process 0.
	float global_norm_quadratic;
	MPI_Reduce(&sub_norm, &global_norm_quadratic, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	stop = MPI_Wtime();
	timing = stop - start;

	if (rank == 0) {
		// Calculate square root of result.
		float result =  sqrt(global_norm_quadratic);

		// Print the result and time taken.
		printf("Result: %f, time: %f\n", result, timing);
	}

	free(rand_arr);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}
