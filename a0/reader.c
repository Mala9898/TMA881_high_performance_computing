#include <stdio.h>
#include <stdlib.h>

int main () {

	int matrixSize = 10;
	int *m = (int*) malloc(sizeof(int*)*matrixSize*matrixSize);

	FILE *filePointer;
	filePointer = fopen("matrix.txt", "r");
	int passed = 1;
	for (int i = 0; i < matrixSize*matrixSize; i++) {
		int temp;
		fscanf(filePointer, "%d\n", &temp);
		if (temp != i)
			passed = 0;
	}
	if (passed)
		printf("YES\n");
	else
		printf("NO\n");
	return 0;
}