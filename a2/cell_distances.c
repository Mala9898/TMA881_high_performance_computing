#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

typedef unsigned short ushort;

// i : block index
int load_block(FILE* file, long i, long width, float *block ) {
	fseek(file, i*width*3*8, SEEK_SET); // beginning of file
	int counter = 0;

	while(fscanf(file, "%f", &block[counter]) != EOF) {
		// printf("read %f \n", block[counter]);
		counter++;
		if (counter >= width*3)
			break;
	}
	return counter/3;
}
// static inline = force compiler to inline
static inline void add_count(long i, long j, ushort* distance_map, float* cell_floats) {
	float startx = cell_floats[i*3];
	float starty = cell_floats[i*3 + 1];
	float startz = cell_floats[i*3 + 2];

	float x = (startx - cell_floats[j*3]);
	float y = (starty - cell_floats[j*3 + 1]);
	float z = (startz - cell_floats[j*3 + 2]);
	float dist = sqrtf(x*x + y*y + z*z);
	int idx = (int)(dist*100.0f + 0.5f);
	// #pragma omp atomic
	distance_map[idx]++;
}
static inline void add_count_from_2_blocks(long i, long j, ushort* distance_map, float* current_block, float*prev_block) {
	float startx = current_block[i*3];
	float starty = current_block[i*3 + 1];
	float startz = current_block[i*3 + 2];

	float x = (startx - prev_block[j*3]);
	float y = (starty - prev_block[j*3 + 1]);
	float z = (startz - prev_block[j*3 + 2]);
	float dist = sqrtf(x*x + y*y + z*z);
	int idx = (int)(dist*100.0f + 0.5f);
	// int idx = (int)roundf(dist*100.0f);
	// #pragma omp atomic
	distance_map[idx]++;
}

int main() {
	// omp_set_num_threads(1);
	// omp_set_num_threads(5);
	// omp_set_num_threads(10);
	omp_set_num_threads(20);

	// --- timing
	double start_time, end_time, elapsed;

	// --- 1. read cell coordintes ---
	// FILE *file = fopen("cells", "r");
	// FILE *file = fopen("cell_e4", "r");
	FILE *file = fopen("cell_e5", "r");
	if(file==NULL) {
	 	printf("ERROR: could not find file cells...\n");
		return -1;
	}
	// find number of cells
	fseek(file, 0L, SEEK_END); // end of file
	long file_total_bytes = ftell(file);
	fseek(file, 0L, SEEK_SET); // beginning of file
	
	long num_cells = file_total_bytes/(3L*8L);
	long arr_size = file_total_bytes/(8L);

	// block setup
	long width = 100000L;
	// long total_blocks = num_cells/width + (num_cells%width) ? 1 : 0; // account for remaining items to fit in a block
	long total_blocks = num_cells/width + ((num_cells%width) ? 1 : 0); // account for remaining items to fit in a block
	
	
	float* cell_floats = (float*)malloc(sizeof(float)*(width*3));
	float temp = 0.0f;
	int counter = 0;

	

	// 	start_time = omp_get_wtime();
	// while(fscanf(file, "%f", &cell_floats[counter]) != EOF) {
	// 	counter++;
	// }

		// printf("reading data  %20f time\n", omp_get_wtime()-start_time);


	// printf("computing distances...\n");
	int dist_max_len = 3465; // 3 * (20^2)
	ushort* distance_map = (ushort*) calloc(dist_max_len, sizeof(ushort));


	// --- 2. compute distances ---
		start_time = omp_get_wtime();

	float * CURRENT_BLOCK = (float*) malloc(width*3 * sizeof(float));
	float * PREVIOUS_BLOCK = (float*) malloc(width*3 * sizeof(float));
	
	// load_block(file, 0, width, CURRENT_BLOCK);


	for (long block_i = 0; block_i < total_blocks; block_i++) {
		int cells_read = load_block(file, block_i, width, CURRENT_BLOCK);
		// compute distances within block first
		// iterate on individual cell indices
		for (long i = 0; i < cells_read; i++) {
			for(long j = i+1; j < cells_read; j++) {
				add_count(i,j,distance_map, CURRENT_BLOCK);
			}
		}
		if (block_i == 0)
			continue;
		
		// compute distances of current block against all previous blocks
		for(int block_prev = 0; block_prev < block_i; block_prev++){
			int prev_cells_read = load_block(file, block_prev, width, PREVIOUS_BLOCK);
			for (int current_block_i = 0; current_block_i < cells_read; current_block_i++) {
				for(int prev_block_i = 0; prev_block_i < prev_cells_read; prev_block_i++) {
					add_count_from_2_blocks(current_block_i,prev_block_i, distance_map, CURRENT_BLOCK, PREVIOUS_BLOCK);
				}
			}
		}
		
	}
	
	// #pragma omp parallel for reduction(+:distance_map[:dist_max_len])
	// for(int i = 0; i< num_cells; i++){
	// 	// if (i%10000 == 0)
	// 	// 	printf("i=%d\n", i);
	// 	// for(int j = i+1; j< num_cells;j++) {
	// 	for(int j = i+1; j < num_cells;j+=1) {
	// 		add_count(i,j,distance_map, cell_floats);
	// 	}
	// }


		// printf("computing distances  %20f time\n", omp_get_wtime()-start_time);

		start_time = omp_get_wtime();
	// printf("--- printing distsances: ---\n");
	for(int i = 0; i < dist_max_len; i++) {
		if(distance_map[i])
			printf("%d%d.%d%d %hu\n", i/1000, (i%1000)/100,(i%100)/10,(i%10), distance_map[i]);
			// printf("%02d.%02d %hu\n", i, i*100, distance_map[i]);
	}

		// printf("printing stdout  %20f time\n", omp_get_wtime()-start_time);
	
	// char* t = "15.023";
	// float f = atof(t);

	// printf("read %d floats\n", counter);
		start_time = omp_get_wtime();
	free(CURRENT_BLOCK);
	free(PREVIOUS_BLOCK);
	free(distance_map);
	fclose(file);
		// printf("deallocated everything  %05f time\n", omp_get_wtime()-start_time);


	return 0;
}