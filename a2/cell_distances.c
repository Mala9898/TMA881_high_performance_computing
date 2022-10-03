#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
typedef unsigned short ushort;

int main() {
	

	// --- 1. read cell coordintes ---
	// FILE *file = fopen("cells", "r");
	FILE *file = fopen("cell_e4", "r");
	// FILE *file = fopen("cell_e5", "r");
	if(file==NULL) {
	 	printf("ERROR: could not find file cells...\n");
		return -1;
	}
	// int arr_size = 10;
	int arr_size = 1e4*3;
	// int arr_size = 1e5*3;
	float* cell_floats = (float*)malloc(sizeof(float)*arr_size);
	float temp = 0.0f;
	int counter = 0;
	int num_cells = 0;

		clock_t start1 = clock();
	while(fscanf(file, "%f", &cell_floats[counter]) != EOF) {
		counter++;
		if(counter%3==0)
			num_cells++;
	}
		clock_t end1 = clock();
		double elapsed1 = (double)(end1 - start1) / CLOCKS_PER_SEC;
		printf("reading data  %f time\n", elapsed1);

	// printf("%f \n", cell_floats[arr_size-1]);
	// printf("computing distances...\n");

	// int dist_max_len = 2829;
	int dist_max_len = 3465; // 3 * (20^2)
	ushort* distance_map = (ushort*) calloc(dist_max_len, sizeof(ushort));


	// --- 2. compute distances ---
	// float* distances = (float*)malloc(sizeof(float)*counter);
		start1 = clock();
	
	float startx = 0;
	float starty = 0;
	float startz = 0;
	float startx2,starty2,startz2;
	float x,y,z, x2,y2,z2,x3,y3,z3, x4,y4,z4;
	float x_2,y_2,z_2, x2_2,y2_2,z2_2,x3_2,y3_2,z3_2;
	int idx, idx2, idx3, idx4;
	int idx_2, idx2_2, idx3_2;

	float dist, dist2, dist3, dist4;
	float dist_2, dist2_2, dist3_2;
	
	for(int i = 0; i< num_cells; i++){
		// for(int j = i+1; j< num_cells;j++) {
		for(int j = i+1; j < num_cells;j+=1) {
			startx = cell_floats[i*3];
			starty = cell_floats[i*3 + 1];
			startz = cell_floats[i*3 + 2];

				x = (startx - cell_floats[j*3]);
				y = (starty - cell_floats[j*3 + 1]);
				z = (startz - cell_floats[j*3 + 2]);
				dist = sqrtf(x*x + y*y + z*z);
				idx = (int)(dist*100 + 0.5f);
				// idx = (int)( roundf(dist*100));
				distance_map[idx]++;

				// if (j+1 < num_cells) {
					// x2 = abs(startx - cell_floats[(j+1)*3]);
					// y2 = abs(starty - cell_floats[(j+1)*3 + 1]);
					// z2 = abs(startz - cell_floats[(j+1)*3 + 2]);
					// dist2 = sqrtf(x2*x2 + y2*y2 + z2*z2);
					// idx2 = (int)(dist*100 + 0.5f);
					// distance_map[idx2]++;
				// }
				// if (j+2 < num_cells) {
				// 	x3 = abs(startx - cell_floats[(j+2)*3]);
				// 	y3 = abs(starty - cell_floats[(j+2)*3 + 1]);
				// 	z3 = abs(startz - cell_floats[(j+2)*3 + 2]);
				// 	dist3 = sqrtf(x3*x3 + y3*y3 + z3*z3);
				// 	idx3 = (int)(dist3*100.0f +0.5f);
				// 	distance_map[idx3]++;
				// }
				// if (j+3 < num_cells) {
				// 	x4 = abs(startx - cell_floats[(j+3)*3]);
				// 	y4 = abs(starty - cell_floats[(j+3)*3 + 1]);
				// 	z4 = abs(startz - cell_floats[(j+3)*3 + 2]);
				// 	dist4 = sqrtf(x4*x4 + y4*y4 + z4*z4);
				// 	idx4 = (int)( dist4*100.0f+0.5f);
				// 	distance_map[idx4]++;
				// }
			// ------- i+1 loop
			// startx2 = cell_floats[(i+1)*3];
			// starty2 = cell_floats[(i+1)*3 + 1];
			// startz2 = cell_floats[(i+1)*3 + 1];

			// 	x_2 = startx2 - cell_floats[j*3];
			// 	y_2 = starty2 - cell_floats[j*3 + 1];
			// 	z_2 = startz2 - cell_floats[j*3 + 2];
			// 	dist = sqrtf(x_2*x_2 + y_2*y_2 + z_2*z_2);
			// 	idx_2 = (int)( roundf(dist_2*100.0f));
			// 	distance_map[idx_2]++;

			// 	// if (j+1 < num_cells){
			// 	x2_2 = startx2 - cell_floats[(j+1)*3];
			// 	y2_2 = starty2 - cell_floats[(j+1)*3 + 1];
			// 	z2_2 = startz2 - cell_floats[(j+1)*3 + 2];
			// 	dist2_2 = sqrtf(x2_2*x2_2 + y2_2*y2_2 + z2_2*z2_2);
			// 	idx2_2 = (int)( roundf(dist2_2*100.0f));
			// 	distance_map[idx2_2]++;
				// }

				// x3_2 = startx2 - cell_floats[(j+2)*3];
				// y3_2 = starty2 - cell_floats[(j+2)*3 + 1];
				// z3_2 = startz2 - cell_floats[(j+2)*3 + 2];
				// dist3_2 = sqrtf(x3_2*x3_2 + y3_2*y3_2 + z3_2*z3_2);
				// idx3_2 = (int)( roundf(dist3_2*100.0f));
				// distance_map[idx3_2]++;
		}
	}
		end1 = clock();
		elapsed1 = (double)(end1 - start1) / CLOCKS_PER_SEC;
		printf("computing distances %f time\n", elapsed1);

		start1 = clock();
	// printf("--- printing distsances: ---\n");
	// for(int i = 0; i < dist_max_len; i++) {
	// 	if(distance_map[i])
	// 		printf("%d%d.%d%d %hu\n", i/1000, (i%1000)/100,(i%100)/10,(i%10), distance_map[i]);
	// 		// printf("%02d.%02d %hu\n", i, i*100, distance_map[i]);
	// }

		end1 = clock();
		elapsed1 = (double)(end1 - start1) / CLOCKS_PER_SEC;
		printf("printing stdout %f time\n", elapsed1);
	
	// char* t = "15.023";
	// float f = atof(t);

	// printf("read %d floats\n", counter);


	return 0;
}