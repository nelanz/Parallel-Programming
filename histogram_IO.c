#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// number of elements to generate.
#define N 1024
#define FILENAME "in.txt"

int main (int argv, char** argc) {
	srand(time(0));

	FILE *fptr;
	int i;

	fptr = fopen(FILENAME, "w");

	if (fptr == NULL) {
		printf("IO Error");
		exit(1);
	}
	
	for (i = 0; i < N; i++) {
		fprintf(fptr, "%d ", rand() % 101);
	}

	fclose(fptr);

	return(0);
}
