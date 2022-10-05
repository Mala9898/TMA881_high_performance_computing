#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

typedef unsigned short ushort;

int main() {
	// omp_set_num_threads(1);
	// omp_set_num_threads(5);
	// omp_set_num_threads(10);
	omp_set_num_threads(20);

	// --- timing
	double start_time, end_time, elapsed;

	// --- 1. read cell coordintes ---
	FILE *file = fopen("cells", "r");
	// FILE *file = fopen("cell_e4", "r");
	// FILE *file = fopen("cell_e5", "r");
	if(file==NULL) {
	 	printf("ERROR: could not find file cells...\n");
		return -1;
	}
	// find number of cells
	fseek(file, 0L, SEEK_END); // end of file
	long file_total_bytes = ftell(file);
	printf("length of file in bytes: %li", file_total_bytes);
	fseek(file, 0L, SEEK_SET); // beginning of file
	
	long num_cells = file_total_bytes/(3L*8L);
	long arr_size = file_total_bytes/(8L);
	// int arr_size = 10*3;
	// int arr_size = 1e4*3;
	// int arr_size = 1e5*3;
	float* cell_floats = (float*)malloc(sizeof(float)*arr_size);
	float temp = 0.0f;
	int counter = 0;
	// int num_cells = 0;

		start_time = omp_get_wtime();
	while(fscanf(file, "%f", &cell_floats[counter]) != EOF) {
		counter++;
		// if(counter%3==0)
		// 	num_cells++;
	}
		printf("reading data  %20f time\n", omp_get_wtime()-start_time);

	// printf("%f \n", cell_floats[arr_size-1]);
	// printf("computing distances...\n");
	int dist_max_len = 3465; // 3 * (20^2)
	int* distance_map = (int*) malloc(dist_max_len*sizeof(int));


	// --- 2. compute distances ---
	// float* distances = (float*)malloc(sizeof(float)*counter);
		start_time = omp_get_wtime();
	
	#pragma omp parallel for reduction(+:distance_map[:dist_max_len])
	for(int i = 0; i< num_cells; i++){
		// if (i%10000 == 0)
		// 	printf("i=%d\n", i);
		// for(int j = i+1; j< num_cells;j++) {
		for(int j = i+1; j < num_cells;j+=1) {
			float startx = cell_floats[i*3];
			float starty = cell_floats[i*3 + 1];
			float startz = cell_floats[i*3 + 2];

			float x = (startx - cell_floats[j*3]);
			float y = (starty - cell_floats[j*3 + 1]);
			float z = (startz - cell_floats[j*3 + 2]);
			float dist = sqrtf(x*x + y*y + z*z);
			int idx = (int)(dist*100 + 0.5f);
			// #pragma omp atomic
			distance_map[idx]++;
		}
	}
		printf("computing distances  %20f time\n", omp_get_wtime()-start_time);

		start_time = omp_get_wtime();
	// printf("--- printing distsances: ---\n");
	for(int i = 0; i < dist_max_len; i++) {
		if(distance_map[i])
			printf("%d%d.%d%d %d\n", i/1000, (i%1000)/100,(i%100)/10,(i%10), distance_map[i]);
			// printf("%02d.%02d %hu\n", i, i*100, distance_map[i]);
	}

		printf("printing stdout  %20f time\n", omp_get_wtime()-start_time);
	
	// char* t = "15.023";
	// float f = atof(t);

	// printf("read %d floats\n", counter);
		start_time = omp_get_wtime();
	free(cell_floats);
	free(distance_map);
	fclose(file);
		printf("deallocated everything  %05f time\n", omp_get_wtime()-start_time);


	return 0;
}