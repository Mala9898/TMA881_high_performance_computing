#include <stdio.h>
#include <stdlib.h>

int main() {

	int matrixSize = 10;
	int *m = (int*) malloc(sizeof(int*)*matrixSize*matrixSize);

	// write index to entries (instead of i*j)
	for(int i = 0; i < matrixSize; i++){
		for(int j = 0; j < matrixSize; j++){
			int idx = j*matrixSize+i;
			m[idx] = idx;
		}
	}

	FILE *filePointer;
	filePointer = fopen("matrix.txt", "w+");
	for (int i = 0; i < matrixSize*matrixSize; i++) {
		fprintf(filePointer, "%d\n", m[i]);
	}


	return 0;
}