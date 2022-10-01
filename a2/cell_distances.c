#include <stdio.h>
#include <stdlib.h>

int main() {
	

	FILE *file = fopen("cells", "r");
	if(file==NULL) {
	 	printf("ERROR: could not find file cells...\n");
		return -1;
	}
	int arr_size = 1e5*3;
	float* cell_floats = (float*)malloc(sizeof(float)*arr_size);
	float temp = 0.0f;
	int counter = 0;
	while(fscanf(file, "%f", &cell_floats[counter]) != EOF) {
		counter++;
	}
	
	// char* t = "15.023";
	// float f = atof(t);

	printf("read %d floats\n", counter);
	// printf("%f\n", f);

	return 0;
}